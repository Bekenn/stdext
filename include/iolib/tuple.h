//
//  tuple.h
//  iolib
//
//  Created by James Touton on 12/25/15.
//  Copyright � 2015 James Touton. All rights reserved.
//

#ifndef IOLIB_TUPLE_INCLUDED
#define IOLIB_TUPLE_INCLUDED
#pragma once

#include "meta.h"

#include <tuple>

#include <cstddef>


namespace iolib
{
    template <class Tuple> struct tuple_element_list;
    template <class Tuple> using tuple_element_list_t = typename tuple_element_list<Tuple>::type;
    template <class... Ts>
    struct tuple_element_list<::std::tuple<Ts...>>
    {
        using type = type_list<Ts...>;
    };

    namespace detail
    {
        template <class Function, class... Ts, ::std::size_t... indices>
        decltype(auto) apply(Function&& func, const ::std::tuple<Ts...>& args, type_list<constant<size_t, indices>...>)
        {
            return func(::std::forward<Ts>(::std::get<indices>(args))...);
        }
    }

    template <class Function, class... Ts>
    decltype(auto) apply(Function&& func, const ::std::tuple<Ts...>& args)
    {
        return detail::apply(::std::forward<Function>(func), args, iota_list<sizeof...(Ts), ::std::size_t>());
    }

    namespace detail
    {
        template <class... Tuples> struct tuples_size;
        template <class Tuple, class... Tuples>
        struct tuples_size<Tuple, Tuples...>
        {
            static constexpr ::std::enable_if_t<::std::tuple_size<Tuple>::value == tuples_size<Tuples...>::value, ::std::size_t>
                value = ::std::tuple_size<Tuple>::value;
        };

        template <class T>
        ::std::conditional_t<::std::is_reference<T>::value, ::std::reference_wrapper<::std::remove_reference_t<T>>, T&>
            wrap(::std::remove_reference_t<T>& arg)
        {
            return arg;
        }

        template <::std::size_t n, class... Tuples, ::std::size_t... tuple_indices>
        auto zip_element(const ::std::tuple<Tuples&...>& tuples, type_list<constant<::std::size_t, tuple_indices>...>)
        {
            using element_type_list = type_list<tuple_element_t<Tuples, n>...>;
            return ::std::make_tuple(wrap<list_element_t<element_type_list, tuple_indices>>(::std::get<n>(::std::get<tuple_indices>(tuples)))...);
        }

        template <class Tuples, ::std::size_t... element_indices>
        auto zip(const Tuples& tuples, type_list<constant<::std::size_t, element_indices>...>)
        {
            return ::std::make_tuple(zip_element<element_indices>(tuples, iota_list_t<::std::tuple_size<Tuples>::value, ::std::size_t>())...);
        }
    }

    template <class... Tuples>
    auto zip(const Tuples&... tuples)
    {
        return detail::zip(tuples, iota_list<tuples_size<Tuples>::value, ::std::size_t>());
    }
}

#endif
