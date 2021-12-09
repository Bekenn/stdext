//
//  utility.h
//  stdext
//
//  Created by James Touton on 12/14/15.
//  Copyright © 2015 James Touton. All rights reserved.
//

#ifndef STDEXT_UTILITY_INCLUDED
#define STDEXT_UTILITY_INCLUDED
#pragma once

#include <stdext/traits.h>


#define STDEXT_CONCAT_(a, b) a ## b
#define STDEXT_CONCAT(a, b) STDEXT_CONCAT_(a, b)

namespace stdext
{
    // Discards a value; can be used to silence warnings about unused entities.
    template <class... Ts>
    constexpr void discard(Ts&&...) noexcept
    {
    }

    // Run-time invocation of unreachable results in undefined behavior.
    [[noreturn]] inline void unreachable() noexcept
    {
#ifndef NDEBUG
        [[noreturn]] extern void abort();
        abort();
#endif
#if STDEXT_COMPILER_GCC
        __builtin_unreachable();
#endif
    }

    namespace detail
    {
#if __cpp_lib_is_nothrow_convertible >= 201806
        using std::is_nothrow_convertible;
        using std::is_nothrow_convertible_v;
#else
        template <class From, class To>
        struct is_nothrow_convertible
        {
            template <class T>
            static true_type test(T) noexcept;
            template <class T>
            static false_type test(...);
            static constexpr bool value = noexcept(test<To>(declval<From>()));
        };
        template <class From, class To>
        constexpr auto is_nothrow_convertible_v = is_nothrow_convertible<From, To>::value;
#endif
    }

    // forward
    template <class T>
    constexpr T&& forward(std::remove_reference_t<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <class T>
    constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    // copy
    template <class T, STDEXT_REQUIRES(std::is_convertible_v<T, std::decay_t<T>>)>
    constexpr std::decay_t<T> copy(T&& v) noexcept(detail::is_nothrow_convertible_v<T, std::decay_t<T>>)
    {
        return stdext::forward<T>(v);
    }

    // move
    template <class T>
    constexpr std::remove_reference_t<T>&& move(T&& t) noexcept
    {
        return static_cast<std::remove_reference_t<T>&&>(t);
    }

    // move_if_noexcept
    template <class T>
    constexpr std::conditional_t<!std::is_nothrow_move_constructible_v<T> && std::is_copy_constructible_v<T>, const T&, T&&>
        move_if_noexcept(T& x) noexcept
    {
        return stdext::move(x);
    }

    // exchange
    template <class T, class U = T, STDEXT_REQUIRES(std::is_move_constructible_v<T> && std::is_assignable_v<T&, U>)>
    constexpr T exchange(T& obj, U&& new_val) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_assignable_v<T&, U>)
    {
        T old_val = stdext::move(obj);
        obj = stdext::forward<U>(new_val);
        return old_val;
    }

    // swap
    namespace _swap
    {
        template <class T, STDEXT_REQUIRES(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)>
        constexpr void swap(T& a, T& b) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
        {
            a = stdext::exchange(b, a);
        }

        struct impl
        {
            template <class T, STDEXT_REQUIRES(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)>
            constexpr void operator()(T& a, T& b) const noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
            {
                return swap(a, b);
            }

            template <class T, size_t N, STDEXT_REQUIRES(std::is_swappable_v<T>)>
            constexpr void operator()(T (&a)[N], T (&b)[N]) const noexcept(std::is_nothrow_swappable_v<T>)
            {
                for (size_t n = 0; n != N; ++n)
                    swap(a[n], b[n]);
            }
        };
    }

    inline namespace _customizable
    {
        inline constexpr _swap::impl swap;
    }

    // min
    template <class T> constexpr T min(T a) noexcept { return a; }
    template <class T> constexpr T min(T a, T b) noexcept { return b < a ? b : a; }
    template <class T, class... Us>
    constexpr std::enable_if_t<(... && std::is_same_v<T, Us>), T>
        min(T a, T b, Us... rest) noexcept { return min(min(a, b), rest...); }

    // max
    template <class T> constexpr T max(T a) noexcept { return a; }
    template <class T> constexpr T max(T a, T b) noexcept { return b < a ? a : b; }
    template <class T, class... Us>
    constexpr std::enable_if_t<(... && std::is_same_v<T, Us>), T>
        max(T a, T b, Us... rest) noexcept { return max(a, max(b, rest...)); }
}

#endif
