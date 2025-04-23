#include <napi.h>
#include "library_wrapper.h"
#include "type_registry.h"
#include "type_system.h"

namespace {
    void RegisterTypeConverters() {
        auto& registry = ffi_libraries::TypeRegistry::instance();
        
        // Register numeric types
        registry.registerConverter(ffi_libraries::ValueType::Int8, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Int8));
        registry.registerConverter(ffi_libraries::ValueType::UInt8, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::UInt8));
        registry.registerConverter(ffi_libraries::ValueType::Int16, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Int16));
        registry.registerConverter(ffi_libraries::ValueType::UInt16, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::UInt16));
        registry.registerConverter(ffi_libraries::ValueType::Int32, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Int32));
        registry.registerConverter(ffi_libraries::ValueType::UInt32, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::UInt32));
        registry.registerConverter(ffi_libraries::ValueType::Int64, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Int64));
        registry.registerConverter(ffi_libraries::ValueType::UInt64, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::UInt64));
        registry.registerConverter(ffi_libraries::ValueType::Float, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Float));
        registry.registerConverter(ffi_libraries::ValueType::Double, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Double));
        
        // Register string type
        registry.registerConverter(ffi_libraries::ValueType::String, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::String));
        
        // Register pointer type
        registry.registerConverter(ffi_libraries::ValueType::Pointer, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Pointer));
        
        // Register boolean type
        registry.registerConverter(ffi_libraries::ValueType::Bool, ffi_libraries::TypeConverter::forType(ffi_libraries::ValueType::Bool));
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    RegisterTypeConverters();
    return ffi_libraries::Library::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)