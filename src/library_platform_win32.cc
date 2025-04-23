#include "library_platform.h"
#include <windows.h>
#include <system_error>

namespace ffi_libraries
{
    namespace platform
    {

        class Win32Library : public DynamicLibrary
        {
        public:
            Win32Library() : handle_(nullptr) {}
            ~Win32Library() override { close(); }

            bool load(const std::string &path) override
            {
                close();
                handle_ = LoadLibraryA(path.c_str());
                return handle_ != nullptr;
            }

            void *getSymbol(const std::string &name) override
            {
                if (!handle_)
                    return nullptr;
                return reinterpret_cast<void *>(GetProcAddress(handle_, name.c_str()));
            }

            void close() override
            {
                if (handle_)
                {
                    FreeLibrary(handle_);
                    handle_ = nullptr;
                }
            }

            std::string getLastError() override
            {
                DWORD error = ::GetLastError();
                if (error == 0)
                    return "";

                LPSTR messageBuffer = nullptr;
                size_t size = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&messageBuffer),
                    0,
                    nullptr);

                std::string message(messageBuffer, size);
                LocalFree(messageBuffer);

                return message;
            }

        private:
            HMODULE handle_;
        };

        std::unique_ptr<DynamicLibrary> DynamicLibrary::create()
        {
            return std::make_unique<Win32Library>();
        }

    }
}