//
//  function.inl
//  stdext
//
//  Created by James Touton on 12/27/25.
//  Copyright © 2025 James Touton. All rights reserved.
//

#include <type_traits>
namespace stdext
{
    template <typename R, typename... Args>
    template <typename F,
        STDEXT_REQUIRED(!std::is_function_v<F> && !std::is_same_v<remove_cvref_t<F>, function_proxy<R (Args...)>> && std::is_lvalue_reference_v<F> && std::is_invocable_r_v<R, F, Args...>)>
    function_proxy<R (Args...)>::function_proxy(F&& f)
        : _thunk([](Args&&... args, const function_proxy& proxy) -> R
            {
                if constexpr (std::is_void_v<R>)
                    (*reinterpret_cast<std::remove_reference_t<F>*>(proxy._obj))(stdext::forward<Args>(args)...);
                else
                    return (*reinterpret_cast<std::remove_reference_t<F>*>(proxy._obj))(stdext::forward<Args>(args)...);
            })
        , _obj(uintptr_t(&f))
    { }

    template <typename R, typename... Args>
    template <typename F,
        STDEXT_REQUIRED(std::is_function_v<F> && std::is_invocable_r_v<R, F, Args...>)>
    function_proxy<R (Args...)>::function_proxy(F* f)
        : _thunk([](Args&&... args, const function_proxy& proxy) -> R
            {
                if constexpr (std::is_void_v<R>)
                    (*reinterpret_cast<F*>(proxy._obj))(stdext::forward<Args>(args)...);
                else
                    return (*reinterpret_cast<F*>(proxy._obj))(stdext::forward<Args>(args)...);
            })
        , _obj(uintptr_t(f))
    {
        static_assert(sizeof(f) <= sizeof(_obj));
    }

    template <typename R, typename... Args>
    R function_proxy<R (Args... args)>::operator () (Args... args) const
    {
        return _thunk(stdext::forward<Args>(args)..., *this);
    }

    template <typename R, typename... Args>
    template <typename F,
        STDEXT_REQUIRED(!std::is_function_v<F> && !std::is_same_v<remove_cvref_t<F>, function_proxy<R (Args...) noexcept>> && std::is_lvalue_reference_v<F> && std::is_nothrow_invocable_r_v<R, F, Args...>)>
    function_proxy<R (Args...) noexcept>::function_proxy(F&& f)
        : _thunk([](Args&&... args, const function_proxy& proxy) noexcept -> R
            {
                if constexpr (std::is_void_v<R>)
                    (*reinterpret_cast<std::remove_reference_t<F>*>(proxy._obj))(stdext::forward<Args>(args)...);
                else
                    return (*reinterpret_cast<std::remove_reference_t<F>*>(proxy._obj))(stdext::forward<Args>(args)...);
            })
        , _obj(uintptr_t(&f))
    { }

    template <typename R, typename... Args>
    template <typename F,
        STDEXT_REQUIRED(std::is_function_v<F> && std::is_nothrow_invocable_r_v<R, F, Args...>)>
    function_proxy<R (Args...) noexcept>::function_proxy(F* f)
        : _thunk([](Args&&... args, const function_proxy& proxy) noexcept -> R
            {
                if constexpr (std::is_void_v<R>)
                    (*reinterpret_cast<F*>(proxy._obj))(stdext::forward<Args>(args)...);
                else
                    return (*reinterpret_cast<F*>(proxy._obj))(stdext::forward<Args>(args)...);
            })
        , _obj(uintptr_t(f))
    {
        static_assert(sizeof(f) <= sizeof(_obj));
    }

    template <typename R, typename... Args>
    R function_proxy<R (Args... args) noexcept>::operator () (Args... args) const noexcept
    {
        return _thunk(stdext::forward<Args>(args)..., *this);
    }
}
