#pragma once

#include "type_system.h"
#include <vector>

namespace ffi_libraries {

// Call a native function with the given return type and arguments
NativeValue CallNativeFunction(void* funcPtr, ValueType returnType, const std::vector<NativeValue>& args);

} // namespace ffi_libraries 