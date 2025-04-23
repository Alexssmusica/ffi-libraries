#include "library_platform.h"
#include <windows.h>
#include <system_error>
#include <iostream>
#include <sstream>

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
                if (!handle_) {
                    DWORD error = GetLastError();
                    std::stringstream ss;
                    ss << "LoadLibrary failed (error " << error << "): " << getSystemErrorMessage(error);
                    lastError_ = ss.str();
                    return false;
                }
                return true;
            }

            void *getSymbol(const std::string &name) override
            {
                if (!handle_) {
                    lastError_ = "Cannot get symbol - library not loaded";
                    return nullptr;
                }

                void* symbol = reinterpret_cast<void *>(GetProcAddress(handle_, name.c_str()));
                if (!symbol) {
                    DWORD error = GetLastError();
                    std::stringstream ss;
                    ss << "GetProcAddress failed (error " << error << "): " << getSystemErrorMessage(error);
                    lastError_ = ss.str();
                }
                return symbol;
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
                return lastError_;
            }

        private:
            HMODULE handle_;
            std::string lastError_;

            std::string getSystemErrorMessage(DWORD error) {
                if (error == 0) return "No error";

                LPSTR messageBuffer = nullptr;
                size_t size = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&messageBuffer),
                    0,
                    nullptr);

                if (size == 0) {
                    return "Unknown error";
                }

                std::string message(messageBuffer, size);
                LocalFree(messageBuffer);

                // Remove trailing newlines and periods that Windows tends to add
                while (!message.empty() && (message.back() == '\n' || message.back() == '\r' || message.back() == '.')) {
                    message.pop_back();
                }

                return message;
            }
        };

        std::unique_ptr<DynamicLibrary> DynamicLibrary::create()
        {
            return std::make_unique<Win32Library>();
        }

    }
}