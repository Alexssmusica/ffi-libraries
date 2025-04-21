#include "common.h"
#include <stdexcept>
#include <iostream>
#include <napi.h>

void *CallNativeFunction(void *funcPtr, ValueType returnType, const std::vector<void *> &args)
{
    if (!funcPtr)
    {
        throw std::runtime_error("Invalid function pointer");
    }

    if (args.size() > 8)
    {
        throw std::runtime_error("Function calls with more than 8 arguments are not supported");
    }

    void *result = nullptr;

    try
    {
        switch (returnType)
        {
        case TYPE_VOID:
        {
            if (args.empty())
            {
                auto func = reinterpret_cast<void (*)()>(funcPtr);
                func();
            }
            else
            {
                auto func = reinterpret_cast<void (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
                func(
                    args.size() > 0 ? args[0] : nullptr,
                    args.size() > 1 ? args[1] : nullptr,
                    args.size() > 2 ? args[2] : nullptr,
                    args.size() > 3 ? args[3] : nullptr,
                    args.size() > 4 ? args[4] : nullptr,
                    args.size() > 5 ? args[5] : nullptr,
                    args.size() > 6 ? args[6] : nullptr,
                    args.size() > 7 ? args[7] : nullptr);
            }
            break;
        }
        case TYPE_STRING:
        {
            if (args.empty())
            {
                using StringFunc = const char *(*)();
                auto func = reinterpret_cast<StringFunc>(funcPtr);
                const char *strResult = func();
                if (strResult)
                {
                    size_t len = strlen(strResult) + 1;
                    char *copy = new char[len];
                    strncpy(copy, strResult, len - 1);
                    copy[len - 1] = '\0';
                    result = copy;
                }
            }
            else
            {
                using StringFunc = const char *(*)(void *, void *, void *, void *, void *, void *, void *, void *);
                auto func = reinterpret_cast<StringFunc>(funcPtr);
                const char *strResult = func(
                    args.size() > 0 ? args[0] : nullptr,
                    args.size() > 1 ? args[1] : nullptr,
                    args.size() > 2 ? args[2] : nullptr,
                    args.size() > 3 ? args[3] : nullptr,
                    args.size() > 4 ? args[4] : nullptr,
                    args.size() > 5 ? args[5] : nullptr,
                    args.size() > 6 ? args[6] : nullptr,
                    args.size() > 7 ? args[7] : nullptr);
                if (strResult)
                {
                    size_t len = strlen(strResult) + 1;
                    char *copy = new char[len];
                    strncpy(copy, strResult, len - 1);
                    copy[len - 1] = '\0';
                    result = copy;
                }
            }
            break;
        }
        case TYPE_INT32:
        {
            if (args.empty())
            {
                auto func = reinterpret_cast<int32_t (*)()>(funcPtr);
                int32_t *val = new int32_t(func());
                result = val;
            }
            else
            {
                auto func = reinterpret_cast<int32_t (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
                int32_t *val = new int32_t(func(
                    args.size() > 0 ? args[0] : nullptr,
                    args.size() > 1 ? args[1] : nullptr,
                    args.size() > 2 ? args[2] : nullptr,
                    args.size() > 3 ? args[3] : nullptr,
                    args.size() > 4 ? args[4] : nullptr,
                    args.size() > 5 ? args[5] : nullptr,
                    args.size() > 6 ? args[6] : nullptr,
                    args.size() > 7 ? args[7] : nullptr));
                result = val;
            }
            break;
        }
        case TYPE_UINT32:
        {
            auto func = reinterpret_cast<uint32_t (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            uint32_t *val = new uint32_t(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        case TYPE_INT64:
        {
            auto func = reinterpret_cast<int64_t (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            int64_t *val = new int64_t(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        case TYPE_UINT64:
        {
            auto func = reinterpret_cast<uint64_t (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            uint64_t *val = new uint64_t(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        case TYPE_FLOAT:
        {
            auto func = reinterpret_cast<float (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            float *val = new float(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        case TYPE_DOUBLE:
        {
            auto func = reinterpret_cast<double (*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            double *val = new double(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        case TYPE_POINTER:
        {
            auto func = reinterpret_cast<void *(*)(void *, void *, void *, void *, void *, void *, void *, void *)>(funcPtr);
            void **val = new void *(func(
                args.size() > 0 ? args[0] : nullptr,
                args.size() > 1 ? args[1] : nullptr,
                args.size() > 2 ? args[2] : nullptr,
                args.size() > 3 ? args[3] : nullptr,
                args.size() > 4 ? args[4] : nullptr,
                args.size() > 5 ? args[5] : nullptr,
                args.size() > 6 ? args[6] : nullptr,
                args.size() > 7 ? args[7] : nullptr));
            result = val;
            break;
        }
        default:
            throw std::runtime_error("Unsupported return type");
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Error calling native function: ") + e.what());
    }
    catch (...)
    {
        throw std::runtime_error("Unknown error calling native function");
    }

    return result;
} 