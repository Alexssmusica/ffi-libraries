#include <napi.h>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include "type_system.h"
#include <sstream>
#include <unordered_map>

namespace ffi_libraries {

namespace {
    // String to ValueType mapping
    const std::unordered_map<std::string, ValueType> typeMap = {
        {"void", ValueType::Void},
        {"int8", ValueType::Int8},
        {"uint8", ValueType::UInt8},
        {"int16", ValueType::Int16},
        {"uint16", ValueType::UInt16},
        {"int32", ValueType::Int32},
        {"uint32", ValueType::UInt32},
        {"int64", ValueType::Int64},
        {"uint64", ValueType::UInt64},
        {"float", ValueType::Float},
        {"double", ValueType::Double},
        {"string", ValueType::String},
        {"pointer", ValueType::Pointer},
        {"bool", ValueType::Bool},
        // Add aliases for common C types
        {"int", ValueType::Int32},
        {"unsigned int", ValueType::UInt32},
        {"long", ValueType::Int64},
        {"unsigned long", ValueType::UInt64},
        {"short", ValueType::Int16},
        {"unsigned short", ValueType::UInt16},
        {"char", ValueType::Int8},
        {"unsigned char", ValueType::UInt8}
    };

    // Concrete type converter implementations
    class NumericConverter : public TypeConverter {
    public:
        NativeValue toNative(const Napi::Value& value) override {
            if (!value.IsNumber()) {
                throw TypeConversionError("Expected numeric value");
            }
            
            return NativeValue(value.As<Napi::Number>().Int32Value());
        }

        Napi::Value toJS(Napi::Env env, const NativeValue& value) override {
            return Napi::Number::New(env, std::get<int32_t>(value.getValue()));
        }
    };

    class StringConverter : public TypeConverter {
    public:
        NativeValue toNative(const Napi::Value& value) override {
            if (!value.IsString()) {
                throw TypeConversionError("Expected string value");
            }
            
            std::string str = value.As<Napi::String>().Utf8Value();
            return NativeValue(CString(str.c_str()));
        }

        Napi::Value toJS(Napi::Env env, const NativeValue& value) override {
            const auto& str = std::get<CString>(value.getValue());
            const char* cstr = str.get();
            if (!cstr) {
                return env.Null();
            }
            return Napi::String::New(env, cstr);
        }
    };

    class BooleanConverter : public TypeConverter {
    public:
        NativeValue toNative(const Napi::Value& value) override {
            if (!value.IsBoolean()) {
                throw TypeConversionError("Expected boolean value");
            }
            
            return NativeValue(value.As<Napi::Boolean>().Value());
        }

        Napi::Value toJS(Napi::Env env, const NativeValue& value) override {
            return Napi::Boolean::New(env, std::get<bool>(value.getValue()));
        }
    };

    class PointerConverter : public TypeConverter {
    public:
        NativeValue toNative(const Napi::Value& value) override {
            if (!value.IsBuffer() && !value.IsNull()) {
                throw TypeConversionError("Expected buffer or null for pointer");
            }
            
            if (value.IsNull()) {
                return NativeValue(static_cast<void*>(nullptr));
            }
            
            auto buffer = value.As<Napi::Buffer<uint8_t>>();
            return NativeValue(static_cast<void*>(buffer.Data()));
        }

        Napi::Value toJS(Napi::Env env, const NativeValue& value) override {
            void* ptr = std::get<void*>(value.getValue());
            if (!ptr) {
                return env.Null();
            }
            
            // Create an external reference to the pointer without taking ownership
            return Napi::External<void>::New(env, ptr);
        }
    };
}

std::unique_ptr<TypeConverter> TypeConverter::forType(ValueType type) {
    switch (type) {
        case ValueType::Int8:
        case ValueType::UInt8:
        case ValueType::Int16:
        case ValueType::UInt16:
        case ValueType::Int32:
        case ValueType::UInt32:
        case ValueType::Int64:
        case ValueType::UInt64:
        case ValueType::Float:
        case ValueType::Double:
            return std::make_unique<NumericConverter>();
        case ValueType::String:
            return std::make_unique<StringConverter>();
        case ValueType::Bool:
            return std::make_unique<BooleanConverter>();
        case ValueType::Pointer:
            return std::make_unique<PointerConverter>();
        default:
            throw TypeConversionError("Unsupported type");
    }
}

ValueType TypeConverter::getTypeFromString(const std::string& typeStr, Napi::Env env) {
    auto it = typeMap.find(typeStr);
    if (it == typeMap.end()) {
        throw Napi::Error::New(env, "Invalid type: " + typeStr);
    }
    return it->second;
}

// Type size mapping
const std::map<ValueType, size_t> typeSize{
    {ValueType::Int8, sizeof(int8_t)},
    {ValueType::UInt8, sizeof(uint8_t)},
    {ValueType::Int16, sizeof(int16_t)},
    {ValueType::UInt16, sizeof(uint16_t)},
    {ValueType::Int32, sizeof(int32_t)},
    {ValueType::UInt32, sizeof(uint32_t)},
    {ValueType::Int64, sizeof(int64_t)},
    {ValueType::UInt64, sizeof(uint64_t)},
    {ValueType::Float, sizeof(float)},
    {ValueType::Double, sizeof(double)},
    {ValueType::Pointer, sizeof(void*)},
    {ValueType::Bool, sizeof(bool)}
};

void* ConvertJsValueToNative(Napi::Value value, ValueType type, std::vector<void*>& allocations) {
    if (value.IsNull() || value.IsUndefined()) {
        return nullptr;
    }

    void* result = nullptr;

    try {
        switch (type) {
            case ValueType::Int8: {
                int8_t* val = new int8_t(value.As<Napi::Number>().Int32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::UInt8: {
                uint8_t* val = new uint8_t(value.As<Napi::Number>().Uint32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Int16: {
                int16_t* val = new int16_t(value.As<Napi::Number>().Int32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::UInt16: {
                uint16_t* val = new uint16_t(value.As<Napi::Number>().Uint32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Int32: {
                int32_t* val = new int32_t(value.As<Napi::Number>().Int32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::UInt32: {
                uint32_t* val = new uint32_t(value.As<Napi::Number>().Uint32Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Int64: {
                int64_t* val = new int64_t(value.As<Napi::BigInt>().Int64Value(nullptr));
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::UInt64: {
                bool lossless = true;
                uint64_t* val = new uint64_t(value.As<Napi::BigInt>().Uint64Value(&lossless));
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Float: {
                float* val = new float(value.As<Napi::Number>().FloatValue());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Double: {
                double* val = new double(value.As<Napi::Number>().DoubleValue());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::String: {
                if (!value.IsString()) {
                    throw Napi::Error::New(value.Env(), "Expected string value");
                }
                std::string str = value.As<Napi::String>().Utf8Value();
                size_t len = str.length() + 1;  // Include null terminator
                char* val = new char[len];
                std::strcpy(val, str.c_str());
                allocations.push_back(val);
                result = val;
                break;
            }
            case ValueType::Pointer: {
                if (value.IsExternal()) {
                    void** val = new void*(value.As<Napi::External<void>>().Data());
                    allocations.push_back(val);
                    result = val;
                } else {
                    result = nullptr;
                }
                break;
            }
            case ValueType::Bool: {
                bool* val = new bool(value.As<Napi::Boolean>().Value());
                allocations.push_back(val);
                result = val;
                break;
            }
            default:
                throw Napi::Error::New(value.Env(), "Unsupported type in conversion");
        }
    } catch (const Napi::Error&) {
        throw;
    } catch (const std::exception& e) {
        throw Napi::Error::New(value.Env(), std::string("Error converting value: ") + e.what());
    } catch (...) {
        throw Napi::Error::New(value.Env(), "Unknown error converting value");
    }

    return result;
}

Napi::Value ConvertNativeToJsValue(Napi::Env env, void* data, ValueType type) {
    if (data == nullptr) {
        return env.Null();
    }

    try {
        switch (type) {
            case ValueType::Void:
                return env.Undefined();
            case ValueType::Int8:
                return Napi::Number::New(env, *static_cast<int8_t*>(data));
            case ValueType::UInt8:
                return Napi::Number::New(env, *static_cast<uint8_t*>(data));
            case ValueType::Int16:
                return Napi::Number::New(env, *static_cast<int16_t*>(data));
            case ValueType::UInt16:
                return Napi::Number::New(env, *static_cast<uint16_t*>(data));
            case ValueType::Int32:
                return Napi::Number::New(env, *static_cast<int32_t*>(data));
            case ValueType::UInt32:
                return Napi::Number::New(env, *static_cast<uint32_t*>(data));
            case ValueType::Int64:
                return Napi::BigInt::New(env, *static_cast<int64_t*>(data));
            case ValueType::UInt64:
                return Napi::BigInt::New(env, *static_cast<uint64_t*>(data));
            case ValueType::Float:
                return Napi::Number::New(env, *static_cast<float*>(data));
            case ValueType::Double:
                return Napi::Number::New(env, *static_cast<double*>(data));
            case ValueType::String:
                return Napi::String::New(env, static_cast<char*>(data));
            case ValueType::Pointer:
                return Napi::External<void>::New(env, *static_cast<void**>(data));
            case ValueType::Bool:
                return Napi::Boolean::New(env, *static_cast<bool*>(data));
            default:
                throw Napi::Error::New(env, "Unsupported type in conversion");
        }
    } catch (const Napi::Error&) {
        throw;
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, std::string("Error converting value: ") + e.what());
    } catch (...) {
        throw Napi::Error::New(env, "Unknown error converting value");
    }
}

} // namespace ffi_libraries