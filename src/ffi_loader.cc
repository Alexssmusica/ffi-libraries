#include <napi.h>
#include "library_wrapper.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return ffi_libraries::Library::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)