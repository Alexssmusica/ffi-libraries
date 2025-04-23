#pragma once

#include "type_system.h"
#include <napi.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <map>
#include <windows.h>

namespace ffi_libraries {

class Library : public Napi::ObjectWrap<Library> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Library(const Napi::CallbackInfo& info);
    ~Library();

private:
    static Napi::FunctionReference constructor;

    using LibraryHandle = HMODULE;

    struct FunctionInfo {
        void* funcPtr;
        ValueType returnType;
        std::vector<ValueType> paramTypes;
    };

    LibraryHandle handle_;
    bool isOpen_;
    std::map<std::string, FunctionInfo> functions_;

    void LoadLibrary(const std::string& path);
    void* GetFunctionPointer(const std::string& name);
    std::vector<NativeValue> PrepareArguments(const Napi::CallbackInfo& info, const FunctionInfo& funcInfo);

    Napi::Value CallFunction(const Napi::CallbackInfo& info);
    void Close(const Napi::CallbackInfo& info);
};

} // namespace ffi_libraries