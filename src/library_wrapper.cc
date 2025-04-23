#include "library_wrapper.h"
#include "type_system.h"
#include "native_function_caller.h"
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <future>
#include <thread>
#include <windows.h>

static WCHAR* UTF8toWCHAR(const char* inputString)
{
    int outputSize = MultiByteToWideChar(CP_UTF8, 0, inputString, -1, NULL, 0);
    if (outputSize == 0)
        return NULL;

    WCHAR* outputString = (WCHAR*)malloc(outputSize * sizeof(WCHAR));
    if (outputString == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, inputString, -1, outputString, outputSize) != outputSize) {
        free(outputString);
        return NULL;
    }

    return outputString;
}

namespace ffi_libraries {

Napi::FunctionReference Library::constructor;

Napi::Object Library::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Library", {
        InstanceMethod("callFunction", &Library::CallFunction),
        InstanceMethod("close", &Library::Close)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Library", func);
    return exports;
}

Library::Library(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<Library>(info), handle_(nullptr), isOpen_(false) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsObject()) {
        Napi::TypeError::New(env, "Expected (string, object) arguments").ThrowAsJavaScriptException();
        return;
    }

    std::string path = info[0].As<Napi::String>();
    LoadLibrary(path);

    // Process function definitions
    Napi::Object funcDefs = info[1].As<Napi::Object>();
    Napi::Array funcNames = funcDefs.GetPropertyNames();

    for (uint32_t i = 0; i < funcNames.Length(); i++) {
        Napi::Value funcName = funcNames[i];
        std::string name = funcName.As<Napi::String>();
        
        Napi::Array funcDef = funcDefs.Get(funcName).As<Napi::Array>();
        if (funcDef.Length() != 2 || !funcDef.Get(uint32_t(0)).IsString() || !funcDef.Get(uint32_t(1)).IsArray()) {
            throw Napi::TypeError::New(env, "Invalid function definition for " + name);
        }

        // Get return type and parameter types
        std::string returnTypeStr = funcDef.Get(uint32_t(0)).As<Napi::String>();
        Napi::Array paramTypesArr = funcDef.Get(uint32_t(1)).As<Napi::Array>();

        // Create function info
        FunctionInfo funcInfo;
        funcInfo.returnType = TypeConverter::getTypeFromString(returnTypeStr, env);
        funcInfo.funcPtr = GetFunctionPointer(name);

        // Store parameter types
        uint32_t paramLength = paramTypesArr.Length();
        funcInfo.paramTypes.reserve(paramLength);
        for (uint32_t j = 0; j < paramLength; j++) {
            Napi::Value val = paramTypesArr[j];
            if (!val.IsString()) {
                throw Napi::TypeError::New(env, "Parameter types must be strings");
            }
            std::string typeStr = val.As<Napi::String>();
            funcInfo.paramTypes.push_back(TypeConverter::getTypeFromString(typeStr, env));
        }

        // Store function info
        functions_[name] = std::move(funcInfo);

        // Create function wrapper with captured name
        auto wrapper = [this, name](const Napi::CallbackInfo& info) -> Napi::Value {
            auto it = functions_.find(name);
            if (it == functions_.end()) {
                throw Napi::Error::New(info.Env(), "Function not defined: " + name);
            }

            const FunctionInfo& funcInfo = it->second;
            std::vector<NativeValue> nativeArgs;
            nativeArgs.reserve(funcInfo.paramTypes.size());

            // Convert arguments
            size_t expectedArgs = funcInfo.paramTypes.size();
            bool isAsync = false;
            Napi::Function callback;

            // Check if last argument is a callback for async call
            if (info.Length() > 0 && info[info.Length() - 1].IsFunction()) {
                callback = info[info.Length() - 1].As<Napi::Function>();
                isAsync = true;
            }

            // Convert arguments (excluding callback if async)
            for (size_t i = 0; i < expectedArgs; i++) {
                if (i >= info.Length() || (isAsync && i >= info.Length() - 1)) {
                    throw Napi::TypeError::New(info.Env(), "Not enough arguments");
                }

                try {
                    auto converter = TypeConverter::forType(funcInfo.paramTypes[i]);
                    nativeArgs.push_back(converter->toNative(info[i]));
                } catch (const TypeConversionError& e) {
                    throw Napi::TypeError::New(info.Env(), e.what());
                }
            }

            if (isAsync) {
                // Create a ThreadSafeFunction for the callback
                auto tsfn = Napi::ThreadSafeFunction::New(
                    info.Env(),
                    callback,
                    "Async Callback",
                    0,
                    1
                );

                // Launch async operation
                std::thread([this, tsfn, funcPtr = funcInfo.funcPtr, returnType = funcInfo.returnType, args = std::move(nativeArgs)]() mutable {
                    try {
                        // Call the native function
                        auto result = CallNativeFunction(funcPtr, returnType, args);
                        
                        // Call back with the result
                        tsfn.BlockingCall([result = std::move(result)](Napi::Env env, Napi::Function jsCallback) {
                            try {
                                auto converter = TypeConverter::forType(result.getType());
                                jsCallback.Call({env.Null(), converter->toJS(env, result)});
                            } catch (const std::exception& e) {
                                jsCallback.Call({Napi::Error::New(env, e.what()).Value(), env.Null()});
                            }
                        });
                    } catch (const std::exception& e) {
                        std::string error = e.what();
                        tsfn.BlockingCall([error = std::move(error)](Napi::Env env, Napi::Function jsCallback) {
                            jsCallback.Call({Napi::Error::New(env, error).Value(), env.Null()});
                        });
                    }

                    tsfn.Release();
                }).detach();

                return info.Env().Undefined();
            } else {
                // Synchronous call
                try {
                    auto result = CallNativeFunction(funcInfo.funcPtr, funcInfo.returnType, nativeArgs);
                    auto converter = TypeConverter::forType(result.getType());
                    return converter->toJS(info.Env(), result);
                } catch (const std::exception& e) {
                    throw Napi::Error::New(info.Env(), e.what());
                }
            }
        };

        // Create the main function
        Napi::Function func = Napi::Function::New(env, wrapper, name);

        // Create the async property
        Napi::Function asyncFunc = Napi::Function::New(env, wrapper, name);
        func.Set("async", asyncFunc);

        // Set the function on the instance
        info.This().As<Napi::Object>().Set(name, func);
    }
}

Library::~Library() {
    if (isOpen_) {
        FreeLibrary(handle_);
    }
}

void Library::LoadLibrary(const std::string& path) {
    HMODULE hModule = NULL;
    hModule = ::LoadLibraryA(path.c_str());
    handle_ = hModule;
    if (!handle_) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to load library: " + std::to_string(error));
    }

    isOpen_ = true;
}

void* Library::GetFunctionPointer(const std::string& name) {
    if (!isOpen_) {
        throw std::runtime_error("Library is not open");
    }

    void* funcPtr = reinterpret_cast<void*>(GetProcAddress(handle_, name.c_str()));

    if (!funcPtr) {
        throw std::runtime_error("Function not found: " + name);
    }

    return funcPtr;
}

std::vector<NativeValue> Library::PrepareArguments(const Napi::CallbackInfo& info, const FunctionInfo& funcInfo) {
    std::vector<NativeValue> nativeArgs;
    nativeArgs.reserve(funcInfo.paramTypes.size());

    for (size_t i = 0; i < funcInfo.paramTypes.size(); i++) {
        if (i >= info.Length()) {
            throw Napi::TypeError::New(info.Env(), "Not enough arguments");
        }

        auto converter = TypeConverter::forType(funcInfo.paramTypes[i]);
        nativeArgs.push_back(converter->toNative(info[i]));
    }

    return nativeArgs;
}

Napi::Value Library::CallFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Get function name from the function object
    Napi::Function func = info.This().As<Napi::Object>().Get("name").As<Napi::Function>();
    std::string funcName = func.Get("name").As<Napi::String>();
    auto it = functions_.find(funcName);
    if (it == functions_.end()) {
        throw Napi::Error::New(env, "Function not defined: " + funcName);
    }

    const FunctionInfo& funcInfo = it->second;
    std::vector<NativeValue> nativeArgs;
    nativeArgs.reserve(funcInfo.paramTypes.size());

    // Convert arguments
    for (size_t i = 0; i < funcInfo.paramTypes.size(); i++) {
        if (i >= info.Length()) {
            throw Napi::TypeError::New(env, "Not enough arguments");
        }

        try {
            auto converter = TypeConverter::forType(funcInfo.paramTypes[i]);
            nativeArgs.push_back(converter->toNative(info[i]));
        } catch (const TypeConversionError& e) {
            throw Napi::TypeError::New(env, e.what());
        }
    }

    // Call the native function
    NativeValue result;
    try {
        result = CallNativeFunction(funcInfo.funcPtr, funcInfo.returnType, nativeArgs);
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, std::string("Error calling native function: ") + e.what());
    }

    // Convert result back to JS
    auto converter = TypeConverter::forType(funcInfo.returnType);
    return converter->toJS(env, result);
}

void Library::Close(const Napi::CallbackInfo& info) {
    if (isOpen_) {
        FreeLibrary(handle_);
        isOpen_ = false;
    }
}

} // namespace ffi_libraries