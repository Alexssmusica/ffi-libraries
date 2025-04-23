#include <stdexcept>
#include <iostream>
#include <napi.h>
#include "type_system.h"
#include <memory>

namespace ffi_libraries
{

    namespace
    {

        using FuncPtr0 = void (*)();
        using FuncPtr1 = void (*)(void *);
        using FuncPtr2 = void (*)(void *, void *);
        using FuncPtr3 = void (*)(void *, void *, void *);
        using FuncPtr4 = void (*)(void *, void *, void *, void *);
        using FuncPtr5 = void (*)(void *, void *, void *, void *, void *);
        using FuncPtr6 = void (*)(void *, void *, void *, void *, void *, void *);
        using FuncPtr7 = void (*)(void *, void *, void *, void *, void *, void *, void *);
        using FuncPtr8 = void (*)(void *, void *, void *, void *, void *, void *, void *, void *);

        template <typename T>
        T *get_value_ptr(const NativeValue &value)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                switch (value.getType())
                {
                case ValueType::String:
                {
                    const auto &str = std::get<CString>(value.getValue());
                    return static_cast<T *>(const_cast<void *>(static_cast<const void *>(str.get())));
                }
                case ValueType::Pointer:
                    return static_cast<T *>(std::get<void *>(value.getValue()));
                case ValueType::Int8:
                    return static_cast<T *>(static_cast<void *>(const_cast<int8_t *>(&std::get<int8_t>(value.getValue()))));
                case ValueType::UInt8:
                    return static_cast<T *>(static_cast<void *>(const_cast<uint8_t *>(&std::get<uint8_t>(value.getValue()))));
                case ValueType::Int16:
                    return static_cast<T *>(static_cast<void *>(const_cast<int16_t *>(&std::get<int16_t>(value.getValue()))));
                case ValueType::UInt16:
                    return static_cast<T *>(static_cast<void *>(const_cast<uint16_t *>(&std::get<uint16_t>(value.getValue()))));
                case ValueType::Int32:
                    return static_cast<T *>(static_cast<void *>(const_cast<int32_t *>(&std::get<int32_t>(value.getValue()))));
                case ValueType::UInt32:
                    return static_cast<T *>(static_cast<void *>(const_cast<uint32_t *>(&std::get<uint32_t>(value.getValue()))));
                case ValueType::Int64:
                    return static_cast<T *>(static_cast<void *>(const_cast<int64_t *>(&std::get<int64_t>(value.getValue()))));
                case ValueType::UInt64:
                    return static_cast<T *>(static_cast<void *>(const_cast<uint64_t *>(&std::get<uint64_t>(value.getValue()))));
                case ValueType::Float:
                    return static_cast<T *>(static_cast<void *>(const_cast<float *>(&std::get<float>(value.getValue()))));
                case ValueType::Double:
                    return static_cast<T *>(static_cast<void *>(const_cast<double *>(&std::get<double>(value.getValue()))));
                case ValueType::Bool:
                    return static_cast<T *>(static_cast<void *>(const_cast<bool *>(&std::get<bool>(value.getValue()))));
                default:
                    return nullptr;
                }
            }
            else
            {
                return const_cast<T *>(&std::get<T>(value.getValue()));
            }
        }

        template <typename RetType, typename... Args>
        RetType call_function(void *func_ptr, Args... args)
        {
            using FuncType = RetType (*)(Args...);
            return reinterpret_cast<FuncType>(func_ptr)(args...);
        }

        template <typename T>
        T callNativeFunction(void *funcPtr, const std::vector<NativeValue> &args)
        {
            switch (args.size())
            {
            case 0:
                return call_function<T>(funcPtr);
            case 1:
                return call_function<T, void *>(funcPtr,
                                                get_value_ptr<void>(args[0]));
            case 2:
                return call_function<T, void *, void *>(funcPtr,
                                                        get_value_ptr<void>(args[0]),
                                                        get_value_ptr<void>(args[1]));
            case 3:
                return call_function<T, void *, void *, void *>(funcPtr,
                                                                get_value_ptr<void>(args[0]),
                                                                get_value_ptr<void>(args[1]),
                                                                get_value_ptr<void>(args[2]));
            case 4:
                return call_function<T, void *, void *, void *, void *>(funcPtr,
                                                                        get_value_ptr<void>(args[0]),
                                                                        get_value_ptr<void>(args[1]),
                                                                        get_value_ptr<void>(args[2]),
                                                                        get_value_ptr<void>(args[3]));
            case 5:
                return call_function<T, void *, void *, void *, void *, void *>(funcPtr,
                                                                                get_value_ptr<void>(args[0]),
                                                                                get_value_ptr<void>(args[1]),
                                                                                get_value_ptr<void>(args[2]),
                                                                                get_value_ptr<void>(args[3]),
                                                                                get_value_ptr<void>(args[4]));
            case 6:
                return call_function<T, void *, void *, void *, void *, void *, void *>(funcPtr,
                                                                                        get_value_ptr<void>(args[0]),
                                                                                        get_value_ptr<void>(args[1]),
                                                                                        get_value_ptr<void>(args[2]),
                                                                                        get_value_ptr<void>(args[3]),
                                                                                        get_value_ptr<void>(args[4]),
                                                                                        get_value_ptr<void>(args[5]));
            case 7:
                return call_function<T, void *, void *, void *, void *, void *, void *, void *>(funcPtr,
                                                                                                get_value_ptr<void>(args[0]),
                                                                                                get_value_ptr<void>(args[1]),
                                                                                                get_value_ptr<void>(args[2]),
                                                                                                get_value_ptr<void>(args[3]),
                                                                                                get_value_ptr<void>(args[4]),
                                                                                                get_value_ptr<void>(args[5]),
                                                                                                get_value_ptr<void>(args[6]));
            case 8:
                return call_function<T, void *, void *, void *, void *, void *, void *, void *, void *>(funcPtr,
                                                                                                        get_value_ptr<void>(args[0]),
                                                                                                        get_value_ptr<void>(args[1]),
                                                                                                        get_value_ptr<void>(args[2]),
                                                                                                        get_value_ptr<void>(args[3]),
                                                                                                        get_value_ptr<void>(args[4]),
                                                                                                        get_value_ptr<void>(args[5]),
                                                                                                        get_value_ptr<void>(args[6]),
                                                                                                        get_value_ptr<void>(args[7]));
            default:
                throw std::runtime_error("Too many arguments (maximum 8 supported)");
            }
        }

        template <>
        void callNativeFunction<void>(void *funcPtr, const std::vector<NativeValue> &args)
        {
            switch (args.size())
            {
            case 0:
                call_function<void>(funcPtr);
                break;
            case 1:
                call_function<void, void *>(funcPtr, get_value_ptr<void>(args[0]));
                break;
            case 2:
                call_function<void, void *, void *>(funcPtr,
                                                    get_value_ptr<void>(args[0]),
                                                    get_value_ptr<void>(args[1]));
                break;
            default:
                throw std::runtime_error("Too many arguments (maximum 8 supported)");
            }
        }

    }

    NativeValue CallNativeFunction(void *funcPtr, ValueType returnType, const std::vector<NativeValue> &args)
    {
        if (!funcPtr)
        {
            throw std::invalid_argument("Function pointer is null");
        }

        if (args.size() > 8)
        {
            throw std::runtime_error("Too many arguments (maximum 8 supported)");
        }

        switch (returnType)
        {
        case ValueType::Void:
            callNativeFunction<void>(funcPtr, args);
            return NativeValue();

        case ValueType::Int8:
            return NativeValue(callNativeFunction<int8_t>(funcPtr, args));

        case ValueType::UInt8:
            return NativeValue(callNativeFunction<uint8_t>(funcPtr, args));

        case ValueType::Int16:
            return NativeValue(callNativeFunction<int16_t>(funcPtr, args));

        case ValueType::UInt16:
            return NativeValue(callNativeFunction<uint16_t>(funcPtr, args));

        case ValueType::Int32:
            return NativeValue(callNativeFunction<int32_t>(funcPtr, args));

        case ValueType::UInt32:
            return NativeValue(callNativeFunction<uint32_t>(funcPtr, args));

        case ValueType::Int64:
            return NativeValue(callNativeFunction<int64_t>(funcPtr, args));

        case ValueType::UInt64:
            return NativeValue(callNativeFunction<uint64_t>(funcPtr, args));

        case ValueType::Float:
            return NativeValue(callNativeFunction<float>(funcPtr, args));

        case ValueType::Double:
            return NativeValue(callNativeFunction<double>(funcPtr, args));

        case ValueType::String:
        {
            const char *result = callNativeFunction<const char *>(funcPtr, args);
            if (!result)
            {
                return NativeValue(CString());
            }
            return NativeValue(CString(result));
        }

        case ValueType::Pointer:
            return NativeValue(callNativeFunction<void *>(funcPtr, args));

        case ValueType::Bool:
            return NativeValue(callNativeFunction<bool>(funcPtr, args));

        default:
            throw std::runtime_error("Unsupported return type");
        }
    }

}