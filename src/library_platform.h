#pragma once

#include <string>
#include <memory>

namespace ffi_libraries {
namespace platform {

// Interface for platform-specific dynamic library loading
class DynamicLibrary {
public:
    virtual ~DynamicLibrary() = default;
    virtual bool load(const std::string& path) = 0;
    virtual void* getSymbol(const std::string& name) = 0;
    virtual void close() = 0;
    virtual std::string getLastError() = 0;
    
    // Factory method
    static std::unique_ptr<DynamicLibrary> create();

protected:
    DynamicLibrary() = default;
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;
};

} // namespace platform
} // namespace ffi_libraries 