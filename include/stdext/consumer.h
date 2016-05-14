#ifndef STDEXT_CONSUMER_INCLUDED
#define STDEXT_CONSUMER_INCLUDED
#pragma once

#include "iterator.h"


namespace stdext
{
    template <class Consumer>
    struct is_consumer
    {
        static constexpr bool value = false;
    };
    template <class Consumer, class T>
    struct is_consumer<Consumer(T)>
    {
        static constexpr bool value = is_callable_v<Consumer(T), bool>;
    };

    template <class T>
    struct is_consumer_adaptable
    {
        template <class U> static ::std::true_type test(decltype(make_consumer(::std::declval<T>()))*);
        template <class U> static ::std::false_type test(...);
        static constexpr bool value = decltype(test<T>(nullptr))::value;
    };

    template <class Iterator>
    class iterator_consumer
    {
    public:
        using iterator = Iterator;

    public:
        iterator_consumer() : i() { }
        explicit iterator_consumer(const iterator& i) : i(i) { }
        explicit iterator_consumer(iterator&& i) : i(::std::move(i)) { }

    public:
        template <class T>
        bool operator () (T&& value)
        {
            *i++ = ::std::forward<T>(value);
            return true;
        }

    private:
        iterator i;
    };

    template <class Iterator, class Sentinel>
    class delimited_iterator_consumer
    {
    public:
        using iterator = Iterator;
        using sentinel = Sentinel;

    public:
        delimited_iterator_consumer() : i(), j() { }
        delimited_iterator_consumer(const iterator& i, const sentinel& j) : i(i), j(j) { }
        delimited_iterator_consumer(const iterator& i, sentinel&& j) : i(i), j(::std::move(j)) { }
        delimited_iterator_consumer(iterator&& i, const sentinel& j) : i(::std::move(i)), j(j) { }
        delimited_iterator_consumer(iterator&& i, sentinel&& j) : i(::std::move(i)), j(::std::move(j)) { }

    public:
        template <class T>
        bool operator () (T&& value)
        {
            if (i == j)
                return false;
            *i++ = ::std::forward<T>(value);
            return true;
        }

    private:
        Iterator i;
        Sentinel j;
    };

    template <class Iterator, REQUIRES(is_iterator<::std::decay_t<Iterator>>::value)>
    auto make_consumer(Iterator&& i)
    {
        return iterator_consumer<::std::decay_t<Iterator>>(::std::forward<Iterator>(i));
    }

    template <class Iterator, class Sentinel, REQUIRES(is_iterator<::std::decay_t<Iterator>>::value)>
    auto make_consumer(Iterator&& i, Sentinel&& j)
    {
        return delimited_iterator_consumer<::std::decay_t<Iterator>, ::std::decay_t<Sentinel>>(::std::forward<Iterator>(i), ::std::forward<Sentinel>(j));
    }
}

#endif
