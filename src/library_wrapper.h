#pragma once

#include <napi.h>
#include <string>
#include <vector>
#include <map>

struct FunctionInfo
{
    void *ptr;
    std::string returnType;
    std::vector<std::string> paramTypes;
};

class LibraryWrapper : public Napi::ObjectWrap<LibraryWrapper>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    LibraryWrapper(const Napi::CallbackInfo &info);
    ~LibraryWrapper();

private:
    struct Impl;
    Impl *impl;

    Napi::Value Close(const Napi::CallbackInfo &info);
    Napi::Function CreateSyncWrapper(Napi::Env env, const FunctionInfo &funcInfo);
    Napi::Function CreateAsyncWrapper(Napi::Env env, const FunctionInfo &funcInfo);
};