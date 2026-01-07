//
//  function.h
//  stdext
//
//  Created by James Touton on 12/27/25.
//  Copyright © 2025 James Touton. All rights reserved.
//

#ifndef STDEXT_FUNCTION_INCLUDED
#define STDEXT_FUNCTION_INCLUDED
#include <type_traits>
#pragma once

#include <stdext/traits.h>
#include <stdext/utility.h>


namespace stdext
{
    template <typename F>
    class function_proxy;

    template <typename R, typename... Args>
    class function_proxy<R (Args...)>
    {
    public:
        template <typename F,
            STDEXT_REQUIRES(!std::is_function_v<F> && !std::is_same_v<remove_cvref_t<F>, function_proxy> && std::is_lvalue_reference_v<F> && std::is_invocable_r_v<R, F, Args...>)>
        function_proxy(F&& f);

        template <typename F,
            STDEXT_REQUIRES(std::is_function_v<F> && std::is_invocable_r_v<R, F, Args...>)>
        function_proxy(F* f);

        friend bool operator == (const function_proxy& a, const function_proxy& b) noexcept
        {
            return a._thunk == b._thunk && a._obj == b._obj;
        }

        friend bool operator != (const function_proxy& a, const function_proxy& b) noexcept
        {
            return a._thunk != b._thunk || a._obj != b._obj;
        }

        friend void swap(function_proxy& a, function_proxy& b) noexcept
        {
            stdext::swap(a._thunk, b._thunk);
            stdext::swap(a._obj, b._obj);
        }

    public:
        R operator () (Args... args) const;

    private:
        R (* _thunk)(Args&&... args, const function_proxy& proxy);
        uintptr_t _obj;
    };

    template <typename R, typename... Args>
    class function_proxy<R (Args...) noexcept>
    {
    public:
        template <typename F,
            STDEXT_REQUIRES(!std::is_function_v<F> && !std::is_same_v<remove_cvref_t<F>, function_proxy> && std::is_lvalue_reference_v<F> && std::is_nothrow_invocable_r_v<R, F, Args...>)>
        function_proxy(F&& f);

        template <typename F,
            STDEXT_REQUIRES(std::is_function_v<F> && std::is_nothrow_invocable_r_v<R, F, Args...>)>
        function_proxy(F* f);

        friend bool operator == (const function_proxy& a, const function_proxy& b) noexcept
        {
            return a._thunk == b._thunk && a._obj == b._obj;
        }

        friend bool operator != (const function_proxy& a, const function_proxy& b) noexcept
        {
            return a._thunk != b._thunk || a._obj != b._obj;
        }

        friend void swap(function_proxy& a, function_proxy& b) noexcept
        {
            stdext::swap(a._thunk, b._thunk);
            stdext::swap(a._obj, b._obj);
        }

    public:
        R operator () (Args... args) const noexcept;

    private:
        R (* _thunk)(Args&&... args, const function_proxy& proxy) noexcept;
        uintptr_t _obj;
    };
}

#include <stdext/_impl/function.inl>

#endif
