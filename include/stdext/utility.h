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

#include <stdext/types.h>

#include <utility>


#define STDEXT_CONCAT_(a, b) a ## b
#define STDEXT_CONCAT(a, b) STDEXT_CONCAT_(a, b)

namespace stdext
{
    namespace detail
    {
        // lightweight traits
        template <class T1, class T2> constexpr bool is_same = false;
        template <class T> constexpr bool is_same<T, T> = true;

        template <bool Enable, class T> struct enable_if { };
        template <class T> struct enable_if<true, T> { using type = T; };
        template <bool Enable, class T> using enable_if_t = typename enable_if<Enable, T>::type;
    }

    // Useful declarations
    using std::swap;
    using std::move;
    using std::move_if_noexcept;
    using std::forward;
    using std::exchange;
    using std::declval;

    // Discards a value; can be used to silence warnings about unused entities.
    template <class... Ts>
    constexpr void discard(Ts&&...) noexcept
    {
    }

    // Returns the length of an array.
    template <class T, size_t size>
    constexpr size_t lengthof(T (&)[size]) noexcept
    {
        return size;
    }

    // min
    template <class T> constexpr T min(T a) noexcept { return a; }
    template <class T> constexpr T min(T a, T b) noexcept { return b < a ? b : a; }
    template <class T, class... Us>
    constexpr detail::enable_if_t<(... && detail::is_same<T, Us>), T>
        min(T a, T b, Us... rest) noexcept { return min(min(a, b), rest...); }

    // max
    template <class T> constexpr T max(T a) noexcept { return a; }
    template <class T> constexpr T max(T a, T b) noexcept { return b < a ? a : b; }
    template <class T, class... Us>
    constexpr detail::enable_if_t<(... && detail::is_same<T, Us>), T>
        max(T a, T b, Us... rest) noexcept { return max(a, max(b, rest...)); }

    // Run-time invocation of unreachable results in undefined behavior.
    [[noreturn]] inline void unreachable() noexcept
    {
#ifndef NDEBUG
        std::abort();
#endif
#if STDEXT_COMPILER_GCC
        __builtin_unreachable();
#endif
    }
}

#endif
