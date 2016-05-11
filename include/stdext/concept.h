//
//  concept.h
//  stdext
//
//  Created by James Touton on 10/18/15.
//  Copyright © 2015 James Touton. All rights reserved.
//

#ifndef STDEXT_CONCEPT_INCLUDED
#define STDEXT_CONCEPT_INCLUDED
#pragma once

#include <type_traits>

#define REQUIRED(...) ::std::enable_if_t<(__VA_ARGS__), ::std::nullptr_t>
#define REQUIRES(...) REQUIRED(__VA_ARGS__) = nullptr

#define DECLARE_HAS_INNER_TYPE(Inner)                                       \
template <class T> struct has_inner_type_##Inner                            \
{                                                                           \
    template <class U> static ::std::true_type test(typename U::Inner*);    \
    template <class U> static ::std::false_type test(...);                  \
    static constexpr bool value = decltype(test<T>(nullptr))::value;        \
}
#define HAS_INNER_TYPE_T(T, Inner) has_inner_type_##Inner<T>
#define HAS_INNER_TYPE(T, Inner) HAS_INNER_TYPE_T(T, Inner)::value

#define DECLARE_HAS_METHOD(MethodName)                                                                                          \
template <class T, class... ArgTypes> struct has_method_##MethodName                                                            \
{                                                                                                                               \
    template <class U> static ::std::true_type test(decltype(::std::declval<U>().MethodName(::std::declval<ArgTypes>()...))*);  \
    template <class U> static ::std::false_type test(...);                                                                      \
    static constexpr bool value = decltype(test<T>(nullptr))::value;                                                            \
}
#define HAS_METHOD_T(T, MethodName, ...) has_method_##MethodName<T, ## __VA_ARGS__>
#define HAS_METHOD(T, MethodName, ...) HAS_METHOD_T(T, MethodName, ## __VA_ARGS__)::value

namespace stdext
{
    template <class, class R = void> struct is_callable;
    template <class Function, class... ArgTypes>
    struct is_callable<Function(ArgTypes...), void>
    {
        template <class T> static ::std::true_type test(decltype(::std::declval<T>()(::std::declval<ArgTypes>()...))*);
        template <class T> static ::std::false_type test(...);
        static constexpr bool value = decltype(test<Function>(nullptr))::value;
    };
    template <class Function, class... ArgTypes, class R>
    struct is_callable<Function(ArgTypes...), R>
    {
        template <class T> static R test_ret() { return ::std::declval<T>()(::std::declval<ArgTypes>()...); }
        template <class T> static ::std::true_type test(decltype(test_ret<T>())*);
        template <class T> static ::std::false_type test(...);
        static constexpr bool value = decltype(test<Function>(nullptr))::value;
    };
    template <class F, class R = void>
    constexpr bool is_callable_v = is_callable<F, R>::value;

    template <class T1, class T2>
    struct is_equality_comparable
    {
        template <class U1, class U2>
        static ::std::true_type test(decltype(::std::declval<U1>() == ::std::declval<U2>()
            && ::std::declval<U1>() != ::std::declval<U2>())*);
        template <class U1, class U2>
        static ::std::false_type test(...);
        static constexpr bool value = decltype(test<T1, T2>(nullptr))::value;
    };

    namespace detail
    {
        template <class T, template <class> class IsT> struct value_type_of;
        template <class T, template <class> class IsT> struct position_type_of;
        template <class T, template <class> class IsT> struct difference_type_of;
        template <class T, template <class> class IsT> struct size_type_of;
        template <class T, template <class> class IsT> struct pointer_type_of;
        template <class T, template <class> class IsT> struct reference_type_of;
    }

    template <class T, template <class> class IsT> using value_type = typename detail::value_type_of<T, IsT>::type;
    template <class T, template <class> class IsT> using position_type = typename detail::position_type_of<T, IsT>::type;
    template <class T, template <class> class IsT> using difference_type = typename detail::difference_type_of<T, IsT>::type;
    template <class T, template <class> class IsT> using size_type = typename detail::size_type_of<T, IsT>::type;
    template <class T, template <class> class IsT> using pointer_type = typename detail::pointer_type_of<T, IsT>::type;
    template <class T, template <class> class IsT> using reference_type = typename detail::reference_type_of<T, IsT>::type;
}

#endif
