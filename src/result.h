#pragma once

#include <variant>
#include <optional>
#include <string>
#include <type_traits>
#include <functional>

namespace ffi_libraries {

// Forward declare a special type for void results
struct VoidOk {};

template<typename T, typename E = std::string>
class Result {
public:
    // Constructors for success and error cases
    template<typename U, typename = std::enable_if_t<!std::is_same_v<T, void>>>
    static Result<T, E> ok(U&& value) {
        return Result(std::in_place_index<0>, std::forward<U>(value));
    }
    
    // Special case for void
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, void>>>
    static Result<void, E> ok() {
        return Result(std::in_place_index<0>, VoidOk{});
    }
    
    template<typename U>
    static Result<T, E> err(U&& error) {
        return Result(std::in_place_index<1>, std::forward<U>(error));
    }
    
    // Status checks
    bool isOk() const { return value_.index() == 0; }
    bool isErr() const { return value_.index() == 1; }
    
    // Value access - only enabled for non-void types
    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, void>>>
    const T& value() const& {
        if (!isOk()) {
            throw std::runtime_error("Attempted to get value of error result");
        }
        return std::get<0>(value_);
    }
    
    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, void>>>
    T& value() & {
        if (!isOk()) {
            throw std::runtime_error("Attempted to get value of error result");
        }
        return std::get<0>(value_);
    }
    
    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, void>>>
    T&& value() && {
        if (!isOk()) {
            throw std::runtime_error("Attempted to get value of error result");
        }
        return std::move(std::get<0>(value_));
    }
    
    const E& error() const& {
        if (!isErr()) {
            throw std::runtime_error("Attempted to get error of successful result");
        }
        return std::get<1>(value_);
    }
    
    E& error() & {
        if (!isErr()) {
            throw std::runtime_error("Attempted to get error of successful result");
        }
        return std::get<1>(value_);
    }
    
    E&& error() && {
        if (!isErr()) {
            throw std::runtime_error("Attempted to get error of successful result");
        }
        return std::move(std::get<1>(value_));
    }
    
    // Map operations
    template<typename F>
    auto map(F&& f) -> Result<std::invoke_result_t<F, T>, E> {
        using ResultType = std::invoke_result_t<F, T>;
        if (isOk()) {
            if constexpr (std::is_same_v<T, void>) {
                return Result<ResultType, E>::ok(f());
            } else {
                return Result<ResultType, E>::ok(f(value()));
            }
        }
        return Result<ResultType, E>::err(error());
    }
    
    template<typename F>
    auto mapError(F&& f) -> Result<T, std::invoke_result_t<F, E>> {
        using NewError = std::invoke_result_t<F, E>;
        if (isErr()) {
            return Result<T, NewError>::err(f(error()));
        }
        if constexpr (std::is_same_v<T, void>) {
            return Result<T, NewError>::ok();
        } else {
            return Result<T, NewError>::ok(std::move(value()));
        }
    }
    
    // Monadic operations
    template<typename F>
    auto andThen(F&& f) -> std::invoke_result_t<F, T> {
        if (isOk()) {
            if constexpr (std::is_same_v<T, void>) {
                return f();
            } else {
                return f(value());
            }
        }
        return std::invoke_result_t<F, T>::err(error());
    }
    
    // Unwrap operations
    template<typename U, typename = std::enable_if_t<!std::is_same_v<T, void>>>
    T unwrapOr(U&& defaultValue) const {
        if (isOk()) {
            return value();
        }
        return std::forward<U>(defaultValue);
    }
    
    template<typename F, typename = std::enable_if_t<!std::is_same_v<T, void>>>
    T unwrapOrElse(F&& f) const {
        if (isOk()) {
            return value();
        }
        return f(error());
    }

    // Conversion operator to T for convenience (only for non-void types)
    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, void>>>
    operator T() const& {
        return value();
    }
    
    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, void>>>
    operator T() && {
        return std::move(*this).value();
    }

private:
    using ValueType = std::conditional_t<std::is_same_v<T, void>,
                                       VoidOk,
                                       T>;
    std::variant<ValueType, E> value_;
    
    template<size_t I, typename U>
    Result(std::in_place_index_t<I>, U&& value)
        : value_(std::in_place_index<I>, std::forward<U>(value)) {}
};

// Helper function to wrap a function that might throw into a Result
template<typename F, typename... Args>
auto tryInvoke(F&& f, Args&&... args) 
    -> Result<std::invoke_result_t<F, Args...>> {
    using ReturnType = std::invoke_result_t<F, Args...>;
    try {
        if constexpr (std::is_same_v<ReturnType, void>) {
            f(std::forward<Args>(args)...);
            return Result<void>::ok();
        } else {
            return Result<ReturnType>::ok(
                f(std::forward<Args>(args)...)
            );
        }
    } catch (const std::exception& e) {
        return Result<ReturnType>::err(e.what());
    }
}

} // namespace ffi_libraries 