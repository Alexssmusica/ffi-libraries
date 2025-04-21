#include "library_wrapper.h"
#include "common.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

struct LibraryWrapper::Impl
{
    void *libraryHandle;
    std::map<std::string, FunctionInfo> functions;
};

Napi::Object LibraryWrapper::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Library", {InstanceMethod("close", &LibraryWrapper::Close)});

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("Library", func);
    return exports;
}

LibraryWrapper::LibraryWrapper(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<LibraryWrapper>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsObject())
    {
        Napi::TypeError::New(env, "Expected library path and function definitions").ThrowAsJavaScriptException();
        return;
    }

    std::string libraryPath = info[0].As<Napi::String>().Utf8Value();

#ifdef _WIN32
    void *handle = LoadLibraryA(libraryPath.c_str());
#else
    void *handle = dlopen(libraryPath.c_str(), RTLD_NOW);
#endif

    if (!handle)
    {
        Napi::Error::New(env, "Failed to load library: " + libraryPath).ThrowAsJavaScriptException();
        return;
    }

    Napi::Object funcDefs = info[1].As<Napi::Object>();
    Napi::Array funcNames = funcDefs.GetPropertyNames();
    Napi::Object thisObj = info.This().As<Napi::Object>();

    for (uint32_t i = 0; i < funcNames.Length(); i++)
    {
        Napi::Value funcName = funcNames[i];
        Napi::Value funcDef = funcDefs.Get(funcName);

        if (!funcDef.IsArray())
            continue;

        Napi::Array def = funcDef.As<Napi::Array>();
        if (def.Length() != 2)
            continue;

        std::string returnType = def.Get(uint32_t(0)).As<Napi::String>().Utf8Value();

        Napi::Array paramTypes = def.Get(uint32_t(1)).As<Napi::Array>();
        std::vector<std::string> paramTypeList;

        for (uint32_t j = 0; j < paramTypes.Length(); j++)
        {
            paramTypeList.push_back(paramTypes.Get(j).As<Napi::String>().Utf8Value());
        }

#ifdef _WIN32
        void *funcPtr = GetProcAddress(static_cast<HMODULE>(handle), funcName.As<Napi::String>().Utf8Value().c_str());
#else
        void *funcPtr = dlsym(handle, funcName.As<Napi::String>().Utf8Value().c_str());
#endif

        if (!funcPtr)
        {
            Napi::Error::New(env, "Failed to get function pointer: " + funcName.As<Napi::String>().Utf8Value()).ThrowAsJavaScriptException();
            continue;
        }

        FunctionInfo funcInfo;
        funcInfo.ptr = funcPtr;
        funcInfo.returnType = returnType;
        funcInfo.paramTypes = paramTypeList;

        Napi::Object funcObj = Napi::Object::New(env);

        Napi::Function syncFunc = CreateSyncWrapper(env, funcInfo);
        funcObj = syncFunc;

        funcObj.Set("async", CreateAsyncWrapper(env, funcInfo));

        thisObj.Set(funcName, funcObj);
    }
}

LibraryWrapper::~LibraryWrapper()
{
    if (impl)
    {
        if (impl->libraryHandle)
        {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(impl->libraryHandle));
#else
            dlclose(impl->libraryHandle);
#endif
        }
        delete impl;
    }
}

Napi::Value LibraryWrapper::Close(const Napi::CallbackInfo &info)
{
    if (impl && impl->libraryHandle)
    {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(impl->libraryHandle));
#else
        dlclose(impl->libraryHandle);
#endif
        impl->libraryHandle = nullptr;
    }
    return info.Env().Undefined();
}

Napi::Function LibraryWrapper::CreateSyncWrapper(Napi::Env env, const FunctionInfo &funcInfo)
{
    return Napi::Function::New(env, [funcInfo](const Napi::CallbackInfo &cbInfo) -> Napi::Value
                               {
        Napi::Env cbEnv = cbInfo.Env();
        std::vector<void*> args;
        std::vector<void*> allocations;

        try {
            for (size_t i = 0; i < cbInfo.Length() && i < funcInfo.paramTypes.size(); i++) {
                ValueType type = GetTypeFromString(funcInfo.paramTypes[i], cbEnv);
                void* arg = ConvertJsValueToNative(cbInfo[i], type, allocations);
                args.push_back(arg);
            }

            ValueType returnType = GetTypeFromString(funcInfo.returnType, cbEnv);
            void* result = CallFunction(funcInfo.ptr, returnType, args);

            Napi::Value jsResult = ConvertNativeToJsValue(cbEnv, result, returnType);

            for (void* ptr : allocations) {
                delete[] static_cast<uint8_t*>(ptr);
            }
            if (result && returnType != TYPE_VOID) {
                delete[] static_cast<uint8_t*>(result);
            }

            return jsResult;
        } catch (const std::exception& e) {
            for (void* ptr : allocations) {
                delete[] static_cast<uint8_t*>(ptr);
            }
            throw Napi::Error::New(cbEnv, e.what());
        } });
}

Napi::Function LibraryWrapper::CreateAsyncWrapper(Napi::Env env, const FunctionInfo &funcInfo)
{
    return Napi::Function::New(env, [funcInfo](const Napi::CallbackInfo &cbInfo) -> Napi::Value
                               {
        Napi::Env cbEnv = cbInfo.Env();
        std::vector<void*> args;
        std::vector<void*> allocations;
        
        try {
            if (cbInfo.Length() < 1 || !cbInfo[cbInfo.Length()-1].IsFunction()) {
                throw Napi::TypeError::New(cbEnv, "Last argument must be a callback function");
            }

            for (size_t i = 0; i < cbInfo.Length() - 1 && i < funcInfo.paramTypes.size(); i++) {
                ValueType type = GetTypeFromString(funcInfo.paramTypes[i], cbEnv);
                void* arg = ConvertJsValueToNative(cbInfo[i], type, allocations);
                args.push_back(arg);
            }

            Napi::Function callback = cbInfo[cbInfo.Length()-1].As<Napi::Function>();

            class AsyncWorker : public Napi::AsyncWorker {
            public:
                AsyncWorker(Napi::Function& callback, void* funcPtr, 
                        ValueType returnType, std::vector<void*> args)
                    : Napi::AsyncWorker(callback), 
                    funcPtr(funcPtr), returnType(returnType), args(args) {}

                void Execute() override {
                    try {
                        result = CallFunction(funcPtr, returnType, args);
                    } catch (const std::exception& e) {
                        SetError(e.what());
                    }
                }

                void OnOK() override {
                    Napi::HandleScope scope(Env());
                    Callback().Call({Env().Null(), ConvertNativeToJsValue(Env(), result, returnType)});
                    
                    if (result && returnType != TYPE_VOID) {
                        delete[] static_cast<uint8_t*>(result);
                    }
                    
                    for (void* ptr : args) {
                        delete[] static_cast<uint8_t*>(ptr);
                    }
                }

                void OnError(const Napi::Error& e) override {
                    Napi::HandleScope scope(Env());
                    Callback().Call({e.Value(), Env().Undefined()});
                    
                    for (void* ptr : args) {
                        delete[] static_cast<uint8_t*>(ptr);
                    }
                }

            private:
                void* funcPtr;
                ValueType returnType;
                std::vector<void*> args;
                void* result = nullptr;
            };

            AsyncWorker* worker = new AsyncWorker(callback, funcInfo.ptr, GetTypeFromString(funcInfo.returnType, cbEnv), std::move(args));
            worker->Queue();

            return cbEnv.Undefined();
        } catch (const std::exception& e) {
            for (void* ptr : allocations) {
                delete[] static_cast<uint8_t*>(ptr);
            }
            throw Napi::Error::New(cbEnv, e.what());
        } });
}