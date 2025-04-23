#pragma once

#include "type_system.h"
#include <memory>
#include <unordered_map>
#include <stdexcept>

namespace ffi_libraries
{

    class TypeRegistry
    {
    public:
        static TypeRegistry &instance()
        {
            static TypeRegistry registry;
            return registry;
        }

        void registerConverter(ValueType type, std::unique_ptr<TypeConverter> converter)
        {
            if (converters_.find(type) != converters_.end())
            {
                throw std::runtime_error("Type converter already registered");
            }
            converters_[type] = std::move(converter);
        }

        TypeConverter *getConverter(ValueType type)
        {
            auto it = converters_.find(type);
            if (it == converters_.end())
            {
                throw TypeConversionError("No converter registered for type");
            }
            return it->second.get();
        }

        bool hasConverter(ValueType type) const
        {
            return converters_.find(type) != converters_.end();
        }

        template <typename T, typename ConverterT>
        void registerType(ValueType type)
        {
            static_assert(std::is_base_of<TypeConverter, ConverterT>::value,
                          "Converter must inherit from TypeConverter");
            registerConverter(type, std::make_unique<ConverterT>());
        }

    private:
        TypeRegistry() = default;
        TypeRegistry(const TypeRegistry &) = delete;
        TypeRegistry &operator=(const TypeRegistry &) = delete;

        std::unordered_map<ValueType, std::unique_ptr<TypeConverter>> converters_;
    };

    class ScopedTypeRegistration
    {
    public:
        template <typename T, typename ConverterT>
        ScopedTypeRegistration(ValueType type)
        {
            TypeRegistry::instance().registerType<T, ConverterT>(type);
        }
    };

}