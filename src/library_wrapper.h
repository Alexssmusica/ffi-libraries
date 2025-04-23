#pragma once

#include "type_system.h"
#include "library_platform.h"
#include "type_registry.h"
#include <napi.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <map>

namespace ffi_libraries
{

    class Library : public Napi::ObjectWrap<Library>
    {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        Library(const Napi::CallbackInfo &info);
        ~Library();

    private:
        static Napi::FunctionReference constructor;

        struct FunctionInfo
        {
            void *funcPtr;
            ValueType returnType;
            std::vector<ValueType> paramTypes;
        };

        std::unique_ptr<platform::DynamicLibrary> library_;
        bool isOpen_;
        std::map<std::string, FunctionInfo> functions_;

        void LoadLibrary(const std::string &path);
        void *GetFunctionPointer(const std::string &name);
        std::vector<NativeValue> PrepareArguments(const Napi::CallbackInfo &info, const FunctionInfo &funcInfo);

        Napi::Value CallFunction(const Napi::CallbackInfo &info);
        void Close(const Napi::CallbackInfo &info);

        NativeValue ConvertToNative(const Napi::Value &value, ValueType type)
        {
            return TypeRegistry::instance().getConverter(type)->toNative(value);
        }

        Napi::Value ConvertToJS(Napi::Env env, const NativeValue &value)
        {
            return TypeRegistry::instance().getConverter(value.getType())->toJS(env, value);
        }
    };

}