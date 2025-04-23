#pragma once

#include <napi.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>

namespace ffi_libraries {

// Forward declarations
class TypeConverter;
class NativeValue;

// Modern enum class for type safety
enum class ValueType {
    Void,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Float,
    Double,
    String,
    Pointer,
    Bool
};

// Type traits for mapping C++ types to ValueType
template<typename T> struct TypeToEnum;
template<> struct TypeToEnum<void> { static constexpr ValueType value = ValueType::Void; };
template<> struct TypeToEnum<int8_t> { static constexpr ValueType value = ValueType::Int8; };
template<> struct TypeToEnum<uint8_t> { static constexpr ValueType value = ValueType::UInt8; };
template<> struct TypeToEnum<int16_t> { static constexpr ValueType value = ValueType::Int16; };
template<> struct TypeToEnum<uint16_t> { static constexpr ValueType value = ValueType::UInt16; };
template<> struct TypeToEnum<int32_t> { static constexpr ValueType value = ValueType::Int32; };
template<> struct TypeToEnum<uint32_t> { static constexpr ValueType value = ValueType::UInt32; };
template<> struct TypeToEnum<int64_t> { static constexpr ValueType value = ValueType::Int64; };
template<> struct TypeToEnum<uint64_t> { static constexpr ValueType value = ValueType::UInt64; };
template<> struct TypeToEnum<float> { static constexpr ValueType value = ValueType::Float; };
template<> struct TypeToEnum<double> { static constexpr ValueType value = ValueType::Double; };
template<> struct TypeToEnum<bool> { static constexpr ValueType value = ValueType::Bool; };
template<> struct TypeToEnum<void*> { static constexpr ValueType value = ValueType::Pointer; };
template<typename T> struct TypeToEnum<T*> { static constexpr ValueType value = ValueType::Pointer; };

// Custom string wrapper that supports proper copying
class CString {
public:
    CString() : data_(nullptr) {}
    
    explicit CString(const char* str) {
        if (str) {
            size_t len = strlen(str) + 1;
            data_ = std::make_shared<std::vector<char>>(len);
            strcpy_s(data_->data(), len, str);
        }
    }
    
    CString(const CString& other) = default;
    CString(CString&& other) noexcept = default;
    CString& operator=(const CString& other) = default;
    CString& operator=(CString&& other) noexcept = default;
    
    const char* get() const {
        return data_ ? data_->data() : nullptr;
    }
    
    operator const char*() const { return get(); }
    
private:
    std::shared_ptr<std::vector<char>> data_;
};

template<> struct TypeToEnum<CString> { static constexpr ValueType value = ValueType::String; };

// Smart wrapper for native values
class NativeValue {
public:
    using Variant = std::variant<
        std::monostate,  // for void
        int8_t, uint8_t,
        int16_t, uint16_t,
        int32_t, uint32_t,
        int64_t, uint64_t,
        float, double,
        CString,
        void*,
        bool
    >;

    NativeValue() : type_(ValueType::Void), value_(std::monostate{}) {}
    
    template<typename T>
    explicit NativeValue(T&& value) 
        : type_(TypeToEnum<std::remove_reference_t<T>>::value)
        , value_(std::forward<T>(value)) {}
    
    // Special constructor for string literals and char*
    explicit NativeValue(const char* str) 
        : type_(ValueType::String)
        , value_(CString(str)) {}

    // Copy and move operations can now be defaulted
    NativeValue(const NativeValue&) = default;
    NativeValue(NativeValue&&) noexcept = default;
    NativeValue& operator=(const NativeValue&) = default;
    NativeValue& operator=(NativeValue&&) noexcept = default;
    
    ValueType getType() const { return type_; }
    const Variant& getValue() const { return value_; }
    
    template<typename T>
    T as() const {
        return std::get<T>(value_);
    }

private:
    ValueType type_;
    Variant value_;
};

// Modern type converter interface
class TypeConverter {
public:
    virtual ~TypeConverter() = default;
    
    // Convert JS to native
    virtual NativeValue toNative(const Napi::Value& value) = 0;
    
    // Convert native to JS
    virtual Napi::Value toJS(Napi::Env env, const NativeValue& value) = 0;
    
    // Factory method to get converter for a type
    static std::unique_ptr<TypeConverter> forType(ValueType type);
    
    // Helper to convert type string to enum
    static ValueType getTypeFromString(const std::string& typeStr, Napi::Env env);
};

// Exception classes
class TypeConversionError : public std::runtime_error {
public:
    explicit TypeConversionError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace ffi_libraries 