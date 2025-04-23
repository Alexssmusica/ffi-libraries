#pragma once

#include "type_system.h"
#include <vector>

namespace ffi_libraries
{

    NativeValue CallNativeFunction(void *funcPtr, ValueType returnType, const std::vector<NativeValue> &args);

}