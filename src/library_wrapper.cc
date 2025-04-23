#include "library_wrapper.h"
#include "type_system.h"
#include "native_function_caller.h"
#include "thread_pool.h"
#include "result.h"
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <future>
#include <thread>
#include <sstream>

namespace ffi_libraries
{

    Napi::FunctionReference Library::constructor;

    Napi::Object Library::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Library", {InstanceMethod("callFunction", &Library::CallFunction), InstanceMethod("close", &Library::Close)});

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Library", func);
        return exports;
    }

    Library::Library(const Napi::CallbackInfo &info)
        : Napi::ObjectWrap<Library>(info), library_(platform::DynamicLibrary::create()), isOpen_(false)
    {
        Napi::Env env = info.Env();

        try {
            if (info.Length() < 2 || !info[0].IsString() || !info[1].IsObject())
            {
                throw Napi::Error::New(env, "Expected (string, object) arguments");
            }

            std::string path = info[0].As<Napi::String>();
            LoadLibrary(path);

            Napi::Object funcDefs = info[1].As<Napi::Object>();
            Napi::Array funcNames = funcDefs.GetPropertyNames();

            for (uint32_t i = 0; i < funcNames.Length(); i++)
            {
                Napi::Value funcName = funcNames[i];
                std::string name = funcName.As<Napi::String>();

                Napi::Array funcDef = funcDefs.Get(funcName).As<Napi::Array>();
                if (funcDef.Length() != 2 || !funcDef.Get(uint32_t(0)).IsString() || !funcDef.Get(uint32_t(1)).IsArray())
                {
                    throw Napi::Error::New(env, "Invalid function definition for " + name);
                }

                std::string returnTypeStr = funcDef.Get(uint32_t(0)).As<Napi::String>();
                Napi::Array paramTypesArr = funcDef.Get(uint32_t(1)).As<Napi::Array>();

                FunctionInfo funcInfo;
                funcInfo.returnType = TypeConverter::getTypeFromString(returnTypeStr, env);
                
                try {
                    funcInfo.funcPtr = GetFunctionPointer(name);
                } catch (const std::exception& e) {
                    throw Napi::Error::New(env, std::string("Failed to get function '") + name + "': " + e.what());
                }

                uint32_t paramLength = paramTypesArr.Length();
                funcInfo.paramTypes.reserve(paramLength);
                for (uint32_t j = 0; j < paramLength; j++)
                {
                    Napi::Value val = paramTypesArr[j];
                    if (!val.IsString())
                    {
                        throw Napi::Error::New(env, "Parameter types must be strings");
                    }
                    std::string typeStr = val.As<Napi::String>();
                    funcInfo.paramTypes.push_back(TypeConverter::getTypeFromString(typeStr, env));
                }

                functions_[name] = std::move(funcInfo);

                auto wrapper = [this, name](const Napi::CallbackInfo &info) -> Napi::Value
                {
                    Napi::Env env = info.Env();
                    try {
                        auto it = functions_.find(name);
                        if (it == functions_.end())
                        {
                            throw Napi::Error::New(env, "Function not defined: " + name);
                        }

                        const FunctionInfo &funcInfo = it->second;
                        std::vector<NativeValue> nativeArgs;
                        nativeArgs.reserve(funcInfo.paramTypes.size());

                        size_t expectedArgs = funcInfo.paramTypes.size();
                        bool isAsync = false;
                        Napi::Function callback;

                        if (info.Length() > 0 && info[info.Length() - 1].IsFunction())
                        {
                            callback = info[info.Length() - 1].As<Napi::Function>();
                            isAsync = true;
                        }

                        for (size_t i = 0; i < expectedArgs; i++)
                        {
                            if (i >= info.Length() || (isAsync && i >= info.Length() - 1))
                            {
                                throw Napi::Error::New(env, "Not enough arguments");
                            }

                            try
                            {
                                nativeArgs.push_back(ConvertToNative(info[i], funcInfo.paramTypes[i]));
                            }
                            catch (const std::exception &e)
                            {
                                throw Napi::Error::New(env, e.what());
                            }
                        }

                        if (isAsync)
                        {
                            auto tsfn = Napi::ThreadSafeFunction::New(
                                env,
                                callback,
                                "Async Callback",
                                0,
                                1);

                            GlobalThreadPool::instance().enqueue([this, tsfn, funcPtr = funcInfo.funcPtr,
                                                                returnType = funcInfo.returnType,
                                                                args = std::move(nativeArgs)]() mutable
                                                               {
                                try {
                                    auto nativeResult = CallNativeFunction(funcPtr, returnType, args);
                                    
                                    tsfn.BlockingCall([this, nativeResult](Napi::Env env, Napi::Function jsCallback) {
                                        try {
                                            auto jsValue = ConvertToJS(env, nativeResult);
                                            jsCallback.Call({env.Null(), jsValue});
                                        } catch (const std::exception& e) {
                                            jsCallback.Call({Napi::Error::New(env, e.what()).Value(), env.Undefined()});
                                        }
                                    });
                                } catch (const std::exception& e) {
                                    tsfn.BlockingCall([e](Napi::Env env, Napi::Function jsCallback) {
                                        jsCallback.Call({Napi::Error::New(env, e.what()).Value(), env.Undefined()});
                                    });
                                }

                                tsfn.Release(); });

                            return env.Undefined();
                        }
                        else
                        {
                            auto result = CallNativeFunction(funcInfo.funcPtr, funcInfo.returnType, nativeArgs);
                            return ConvertToJS(env, result);
                        }
                    } catch (const std::exception& e) {
                        throw Napi::Error::New(env, e.what());
                    }
                };

                Napi::Function func = Napi::Function::New(env, wrapper, name);
                Napi::Function asyncFunc = Napi::Function::New(env, wrapper, name);
                func.Set("async", asyncFunc);

                info.This().As<Napi::Object>().Set(name, func);
            }
        } catch (const std::exception& e) {
            throw Napi::Error::New(env, e.what());
        }
    }

    Library::~Library()
    {
        if (isOpen_)
        {
            library_->close();
        }
    }

    void Library::LoadLibrary(const std::string &path)
    {
        if (!library_->load(path)) {
            throw Napi::Error::New(Env(), "Failed to load library: " + library_->getLastError());
        }
        isOpen_ = true;
    }

    void *Library::GetFunctionPointer(const std::string &name)
    {
        if (!isOpen_) {
            throw Napi::Error::New(Env(), "Library is not open");
        }

        void* funcPtr = library_->getSymbol(name);
        if (!funcPtr) {
            throw Napi::Error::New(Env(), "Failed to get function '" + name + "': " + library_->getLastError());
        }

        return funcPtr;
    }

    std::vector<NativeValue> Library::PrepareArguments(const Napi::CallbackInfo &info, const FunctionInfo &funcInfo)
    {
        auto result = tryInvoke([this, &info, &funcInfo]() -> Result<std::vector<NativeValue>>
                                {
        std::vector<NativeValue> nativeArgs;
        nativeArgs.reserve(funcInfo.paramTypes.size());

        for (size_t i = 0; i < funcInfo.paramTypes.size(); i++) {
            if (i >= info.Length()) {
                return Result<std::vector<NativeValue>>::err("Not enough arguments");
            }

            auto convResult = tryInvoke([this, &info, &funcInfo, i]() -> Result<NativeValue> {
                return Result<NativeValue>::ok(ConvertToNative(info[i], funcInfo.paramTypes[i]));
            });
            
            if (convResult.isErr()) {
                return Result<std::vector<NativeValue>>::err(convResult.error());
            }
            
            nativeArgs.push_back(convResult.value());
        }

        return Result<std::vector<NativeValue>>::ok(std::move(nativeArgs)); });

        if (result.isErr())
        {
            Napi::Error::New(info.Env(), result.error()).ThrowAsJavaScriptException();
            return std::vector<NativeValue>();
        }

        return result.value();
    }

    Napi::Value Library::CallFunction(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        auto result = tryInvoke([this, &info, env]() -> Result<Napi::Value>
                                {
        Napi::Function func = info.This().As<Napi::Object>().Get("name").As<Napi::Function>();
        std::string funcName = func.Get("name").As<Napi::String>();
        
        auto it = functions_.find(funcName);
        if (it == functions_.end()) {
            return Result<Napi::Value>::err("Function not defined: " + funcName);
        }

        const FunctionInfo& funcInfo = it->second;
        auto nativeArgs = PrepareArguments(info, funcInfo);
        
        auto nativeResult = CallNativeFunction(funcInfo.funcPtr, funcInfo.returnType, nativeArgs);
        return Result<Napi::Value>::ok(ConvertToJS(env, nativeResult)); });

        if (result.isErr())
        {
            Napi::Error::New(env, result.error()).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return result.value();
    }

    void Library::Close(const Napi::CallbackInfo &info)
    {
        auto result = tryInvoke([this]() -> Result<void>
                                {
        if (isOpen_) {
            library_->close();
            isOpen_ = false;
        }
        return Result<void>::ok(); });

        if (result.isErr())
        {
            Napi::Error::New(info.Env(), result.error()).ThrowAsJavaScriptException();
            return;
        }
    }

}