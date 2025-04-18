#pragma once

#include <napi.h>
#include <string>
#include <vector>
#include <map>

enum ValueType
{
    TYPE_VOID,
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_POINTER,
    TYPE_BOOL
};

ValueType GetTypeFromString(const std::string &typeStr);
void *ConvertJsValueToNative(Napi::Value value, ValueType type, std::vector<void *> &allocations);
Napi::Value ConvertNativeToJsValue(Napi::Env env, void *data, ValueType type);
void *CallFunction(void *funcPtr, ValueType returnType, const std::vector<void *> &args);