#pragma once

#include <variant>
#include <optional>
#include <string>
#include <type_traits>
#include <functional>

namespace ffi_libraries
{

    struct VoidOk
    {
    };

    // Forward declaration of the primary template
    template <typename T, typename E = std::string>
    class Result;

    // Specialization for void
    template <typename E>
    class Result<void, E>
    {
    public:
        static Result<void, E> ok()
        {
            return Result(std::in_place_index<0>, VoidOk{});
        }

        template <typename U>
        static Result<void, E> err(U &&error)
        {
            return Result(std::in_place_index<1>, std::forward<U>(error));
        }

        bool isOk() const { return value_.index() == 0; }
        bool isErr() const { return value_.index() == 1; }

        void value() const
        {
            if (!isOk())
            {
                throw std::runtime_error("Attempted to get value of error result");
            }
        }

        const E &error() const &
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::get<1>(value_);
        }

        E &error() &
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::get<1>(value_);
        }

        E &&error() &&
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::move(std::get<1>(value_));
        }

        template <typename F>
        auto map(F &&f) -> Result<std::invoke_result_t<F>, E>
        {
            using ResultType = std::invoke_result_t<F>;
            if (isOk())
            {
                return Result<ResultType, E>::ok(f());
            }
            return Result<ResultType, E>::err(error());
        }

        template <typename F>
        auto mapError(F &&f) -> Result<void, std::invoke_result_t<F, E>>
        {
            using NewError = std::invoke_result_t<F, E>;
            if (isErr())
            {
                return Result<void, NewError>::err(f(error()));
            }
            return Result<void, NewError>::ok();
        }

        template <typename F>
        auto andThen(F &&f) -> std::invoke_result_t<F>
        {
            if (isOk())
            {
                return f();
            }
            return std::invoke_result_t<F>::err(error());
        }

    private:
        std::variant<VoidOk, E> value_;

        template <size_t I, typename U>
        Result(std::in_place_index_t<I>, U &&value)
            : value_(std::in_place_index<I>, std::forward<U>(value)) {}
    };

    // Primary template for non-void types
    template <typename T, typename E>
    class Result
    {
    public:
        template <typename U>
        static Result<T, E> ok(U &&value)
        {
            return Result(std::in_place_index<0>, std::forward<U>(value));
        }

        template <typename U>
        static Result<T, E> err(U &&error)
        {
            return Result(std::in_place_index<1>, std::forward<U>(error));
        }

        bool isOk() const { return value_.index() == 0; }
        bool isErr() const { return value_.index() == 1; }

        const T &value() const &
        {
            if (!isOk())
            {
                throw std::runtime_error("Attempted to get value of error result");
            }
            return std::get<0>(value_);
        }

        T &value() &
        {
            if (!isOk())
            {
                throw std::runtime_error("Attempted to get value of error result");
            }
            return std::get<0>(value_);
        }

        T &&value() &&
        {
            if (!isOk())
            {
                throw std::runtime_error("Attempted to get value of error result");
            }
            return std::move(std::get<0>(value_));
        }

        const E &error() const &
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::get<1>(value_);
        }

        E &error() &
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::get<1>(value_);
        }

        E &&error() &&
        {
            if (!isErr())
            {
                throw std::runtime_error("Attempted to get error of successful result");
            }
            return std::move(std::get<1>(value_));
        }

        template <typename F>
        auto map(F &&f) -> Result<std::invoke_result_t<F, T>, E>
        {
            using ResultType = std::invoke_result_t<F, T>;
            if (isOk())
            {
                return Result<ResultType, E>::ok(f(value()));
            }
            return Result<ResultType, E>::err(error());
        }

        template <typename F>
        auto mapError(F &&f) -> Result<T, std::invoke_result_t<F, E>>
        {
            using NewError = std::invoke_result_t<F, E>;
            if (isErr())
            {
                return Result<T, NewError>::err(f(error()));
            }
            return Result<T, NewError>::ok(std::move(value()));
        }

        template <typename F>
        auto andThen(F &&f) -> std::invoke_result_t<F, T>
        {
            if (isOk())
            {
                return f(value());
            }
            return std::invoke_result_t<F, T>::err(error());
        }

        template <typename U>
        T unwrapOr(U &&defaultValue) const
        {
            if (isOk())
            {
                return value();
            }
            return std::forward<U>(defaultValue);
        }

        template <typename F>
        T unwrapOrElse(F &&f) const
        {
            if (isOk())
            {
                return value();
            }
            return f(error());
        }

        operator T() const &
        {
            return value();
        }

        operator T() &&
        {
            return std::move(*this).value();
        }

    private:
        std::variant<T, E> value_;

        template <size_t I, typename U>
        Result(std::in_place_index_t<I>, U &&value)
            : value_(std::in_place_index<I>, std::forward<U>(value)) {}
    };

    template <typename F, typename... Args>
    auto tryInvoke(F &&f, Args &&...args)
        -> Result<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;
        try
        {
            if constexpr (std::is_same_v<ReturnType, void>)
            {
                f(std::forward<Args>(args)...);
                return Result<void>::ok();
            }
            else
            {
                return Result<ReturnType>::ok(f(std::forward<Args>(args)...));
            }
        }
        catch (const std::exception &e)
        {
            return Result<ReturnType>::err(e.what());
        }
    }

}