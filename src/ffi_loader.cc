#include <napi.h>
#include "library_wrapper.h"

Napi::Object InitModule(Napi::Env env, Napi::Object exports)
{
    return LibraryWrapper::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitModule)