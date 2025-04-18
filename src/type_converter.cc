#include <napi.h>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include "common.h"

std::map<ValueType, size_t> typeSize{
    std::make_pair(TYPE_INT8, sizeof(int8_t)),
    std::make_pair(TYPE_UINT8, sizeof(uint8_t)),
    std::make_pair(TYPE_INT16, sizeof(int16_t)),
    std::make_pair(TYPE_UINT16, sizeof(uint16_t)),
    std::make_pair(TYPE_INT32, sizeof(int32_t)),
    std::make_pair(TYPE_UINT32, sizeof(uint32_t)),
    std::make_pair(TYPE_INT64, sizeof(int64_t)),
    std::make_pair(TYPE_UINT64, sizeof(uint64_t)),
    std::make_pair(TYPE_FLOAT, sizeof(float)),
    std::make_pair(TYPE_DOUBLE, sizeof(double)),
    std::make_pair(TYPE_POINTER, sizeof(void *)),
    std::make_pair(TYPE_BOOL, sizeof(bool))};

ValueType GetTypeFromString(const std::string &typeStr)
{
    if (typeStr == "void")
        return TYPE_VOID;
    if (typeStr == "int8")
        return TYPE_INT8;
    if (typeStr == "uint8")
        return TYPE_UINT8;
    if (typeStr == "int16")
        return TYPE_INT16;
    if (typeStr == "uint16")
        return TYPE_UINT16;
    if (typeStr == "int32" || typeStr == "int")
        return TYPE_INT32;
    if (typeStr == "uint32")
        return TYPE_UINT32;
    if (typeStr == "int64")
        return TYPE_INT64;
    if (typeStr == "uint64")
        return TYPE_UINT64;
    if (typeStr == "float")
        return TYPE_FLOAT;
    if (typeStr == "double")
        return TYPE_DOUBLE;
    if (typeStr == "string")
        return TYPE_STRING;
    if (typeStr == "pointer")
        return TYPE_POINTER;
    if (typeStr == "bool")
        return TYPE_BOOL;
    throw std::runtime_error("Unknown type: " + typeStr);
}

void *ConvertJsValueToNative(Napi::Value value, ValueType type, std::vector<void *> &allocations)
{
    if (value.IsNull() || value.IsUndefined())
    {
        return nullptr;
    }

    void *result = nullptr;

    try
    {
        switch (type)
        {
        case TYPE_INT8:
        {
            int8_t *val = new int8_t(value.As<Napi::Number>().Int32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_UINT8:
        {
            uint8_t *val = new uint8_t(value.As<Napi::Number>().Uint32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_INT16:
        {
            int16_t *val = new int16_t(value.As<Napi::Number>().Int32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_UINT16:
        {
            uint16_t *val = new uint16_t(value.As<Napi::Number>().Uint32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_INT32:
        {
            int32_t *val = new int32_t(value.As<Napi::Number>().Int32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_UINT32:
        {
            uint32_t *val = new uint32_t(value.As<Napi::Number>().Uint32Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_INT64:
        {
            int64_t *val = new int64_t(value.As<Napi::BigInt>().Int64Value(nullptr));
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_UINT64:
        {
            bool lossless = true;
            uint64_t *val = new uint64_t(value.As<Napi::BigInt>().Uint64Value(&lossless));
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_FLOAT:
        {
            float *val = new float(value.As<Napi::Number>().FloatValue());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_DOUBLE:
        {
            double *val = new double(value.As<Napi::Number>().DoubleValue());
            allocations.push_back(val);
            result = val;
            break;
        }
        case TYPE_STRING:
        {
            if (value.IsString())
            {
                std::string str = value.As<Napi::String>().Utf8Value();
                char *val = new char[str.length() + 1];
                std::strcpy(val, str.c_str());
                allocations.push_back(val);
                result = val;
            }
            else
            {
                result = nullptr;
            }
            break;
        }
        case TYPE_POINTER:
        {
            if (value.IsExternal())
            {
                void **val = new void *(value.As<Napi::External<void>>().Data());
                allocations.push_back(val);
                result = val;
            }
            else
            {
                result = nullptr;
            }
            break;
        }
        case TYPE_BOOL:
        {
            bool *val = new bool(value.As<Napi::Boolean>().Value());
            allocations.push_back(val);
            result = val;
            break;
        }
        default:
            throw std::runtime_error("Unsupported type in conversion");
        }
    }
    catch (const Napi::Error &)
    {
        throw;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Error converting value: ") + e.what());
    }
    catch (...)
    {
        throw std::runtime_error("Unknown error converting value");
    }

    return result;
}

Napi::Value ConvertNativeToJsValue(Napi::Env env, void *data, ValueType type)
{
    if (data == nullptr)
    {
        return env.Null();
    }

    try
    {
        switch (type)
        {
        case TYPE_VOID:
            return env.Undefined();
        case TYPE_INT8:
            return Napi::Number::New(env, *static_cast<int8_t *>(data));
        case TYPE_UINT8:
            return Napi::Number::New(env, *static_cast<uint8_t *>(data));
        case TYPE_INT16:
            return Napi::Number::New(env, *static_cast<int16_t *>(data));
        case TYPE_UINT16:
            return Napi::Number::New(env, *static_cast<uint16_t *>(data));
        case TYPE_INT32:
            return Napi::Number::New(env, *static_cast<int32_t *>(data));
        case TYPE_UINT32:
            return Napi::Number::New(env, *static_cast<uint32_t *>(data));
        case TYPE_INT64:
            return Napi::BigInt::New(env, *static_cast<int64_t *>(data));
        case TYPE_UINT64:
            return Napi::BigInt::New(env, *static_cast<uint64_t *>(data));
        case TYPE_FLOAT:
            return Napi::Number::New(env, *static_cast<float *>(data));
        case TYPE_DOUBLE:
            return Napi::Number::New(env, *static_cast<double *>(data));
        case TYPE_STRING:
            return Napi::String::New(env, static_cast<char *>(data));
        case TYPE_POINTER:
            return Napi::External<void>::New(env, *static_cast<void **>(data));
        case TYPE_BOOL:
            return Napi::Boolean::New(env, *static_cast<bool *>(data));
        default:
            throw std::runtime_error("Unsupported type in conversion");
        }
    }
    catch (const Napi::Error &)
    {
        throw;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Error converting value: ") + e.what());
    }
    catch (...)
    {
        throw std::runtime_error("Unknown error converting value");
    }
}