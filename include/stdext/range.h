//
//  range.h
//  stdext
//
//  Created by James Touton on 10/19/15.
//  Copyright © 2015 James Touton. All rights reserved.
//

#ifndef STDEXT_RANGE_INCLUDED
#define STDEXT_RANGE_INCLUDED
#pragma once

#include <stdext/consumer.h>
#include <stdext/generator.h>
#include <stdext/meta.h>

#include <stdexcept>
#include <tuple>


namespace stdext
{
    // range categories
    struct multi_pass_range_tag { };
    struct bidirectional_range_tag : multi_pass_range_tag { };
    struct random_access_range_tag : bidirectional_range_tag { };

    // range traits
    namespace _private
    {
        STDEXT_DECLARE_HAS_INNER_TYPE(range_category);
    }
    template <typename T> struct is_range
        : std::conditional_t<_private::STDEXT_HAS_INNER_TYPE_V(T, range_category),
            true_type,
            false_type>
    { };

    template <typename Range> using range_category = typename Range::range_category;

    template <typename Range> using range_value_type = typename Range::value_type;
    template <typename Range> using range_position_type = typename Range::position;
    template <typename Range> using range_difference_type = typename Range::difference_type;
    template <typename Range> using range_reference_type = typename Range::reference;

    namespace _private
    {
        STDEXT_DECLARE_HAS_INNER_TYPE(size_type);
        template <typename T, bool HasInnerSizeType>
        struct check_range_inner_size_type
        {
            using type = std::make_unsigned_t<range_difference_type<T>>;
        };
        template <typename T>
        struct check_range_inner_size_type<T, true>
        {
            using type = typename T::size_type;
        };
    }
    template <typename Range> using range_size_type = typename _private::check_range_inner_size_type<Range, _private::STDEXT_HAS_INNER_TYPE_V(Range, size_type)>::type;

    namespace _private
    {
        STDEXT_DECLARE_HAS_METHOD(end_pos);
        template <typename T>
        struct check_end_pos_getter
        {
            template <typename U> static STDEXT_HAS_METHOD(U, end_pos) test(void*);
            template <typename U> static false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
        template <typename T>
        struct check_end_pos_setter
        {
            template <typename U> static STDEXT_HAS_METHOD(U, end_pos, range_position_type<U>) test(void*);
            template <typename U> static false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
    }
    template <typename T> struct is_delimited_range
        : std::conditional_t<is_range<T>::value && _private::check_end_pos_getter<T>::value && _private::check_end_pos_setter<T>::value,
            true_type,
            false_type>
    { };

    namespace _private
    {
        STDEXT_DECLARE_HAS_METHOD(size);
        template <typename T>
        struct check_has_size
        {
            template <typename U> static STDEXT_HAS_METHOD(U, size) test(void*);
            template <typename U> static false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
        STDEXT_DECLARE_HAS_METHOD(resize);
        template <typename T>
        struct check_has_resize
        {
            template <typename U> static STDEXT_HAS_METHOD(U, resize, range_size_type<T>) test(void*);
            template <typename U> static false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
    }
    template <typename T> struct is_counted_range
        : std::conditional_t<is_range<T>::value && _private::check_has_size<T>::value && _private::check_has_resize<T>::value,
            true_type,
            false_type>
    { };

    namespace _private
    {
        template <typename T>
        struct check_stl_range
        {
            template <typename U> static is_equality_comparable_with<
                std::decay_t<decltype(begin(declval<U&>()))>,
                std::decay_t<decltype(end(declval<U&>()))>>
                test(nullptr_t);
            template <typename U> static false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
    }
    template <typename T> struct is_stl_range_provider
        : std::conditional_t<_private::check_stl_range<T>::value,
            true_type,
            false_type>
    { };

    // range concepts
    template <typename Range> struct is_multi_pass_range :
        std::is_base_of<multi_pass_range_tag, list_conditional_apply_t<is_range<Range>::value, range_category, type_list<Range>, std::void_t, type_list<>>> { };
    template <typename Range> struct is_bidirectional_range :
        std::is_base_of<bidirectional_range_tag, list_conditional_apply_t<is_range<Range>::value, range_category, type_list<Range>, std::void_t, type_list<>>> { };
    template <typename Range> struct is_random_access_range :
        std::is_base_of<random_access_range_tag, list_conditional_apply_t<is_range<Range>::value, range_category, type_list<Range>, std::void_t, type_list<>>> { };

    // generic range utilities
    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    range_position_type<Range> next_pos(const Range& range, range_position_type<Range> pos)
    {
        return range.inc_pos(pos);
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value)>
    range_position_type<Range> prev_pos(const Range& range, range_position_type<Range> pos)
    {
        return range.dec_pos(pos);
    }

    namespace _private
    {
        template <typename Range>
        range_position_type<Range>& advance_pos(const Range& range, range_position_type<Range>& pos, range_difference_type<Range> n, multi_pass_range_tag)
        {
            assert(n >= 0);

            while (n-- != 0)
                range.inc_pos(pos);
            return pos;
        }

        template <typename Range>
        range_position_type<Range>& advance_pos(const Range& range, range_position_type<Range>& pos, range_difference_type<Range> n, bidirectional_range_tag)
        {
            for (; n > 0; --n)
                range.inc_pos(pos);
            for (; n < 0; ++n)
                range.dec_pos(pos);
            return pos;
        }

        template <typename Range>
        range_position_type<Range>& advance_pos(const Range& range, range_position_type<Range>& pos, range_difference_type<Range> n, random_access_range_tag)
        {
            return range.advance_pos(pos, n);
        }
    }

    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    range_position_type<Range>& advance_pos(const Range& range, range_position_type<Range>& pos, range_difference_type<Range> n)
    {
        return _private::advance_pos(range, pos, n, range_category<Range>());
    }

    namespace _private
    {
        template <typename Range>
        range_difference_type<Range> distance(const Range& range, range_position_type<Range> p1, range_position_type<Range> p2, multi_pass_range_tag)
        {
            range_difference_type<Range> d = 0;
            for (; p1 != p2; range.inc_pos(p1))
                ++d;
            return d;
        }

        template <typename Range>
        range_difference_type<Range> distance(const Range& range, range_position_type<Range> p1, range_position_type<Range> p2, random_access_range_tag)
        {
            return range.distance(p1, p2);
        }
    }

    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    range_difference_type<Range> distance(const Range& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return _private::distance(range, p1, p2, range_category<Range>());
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    bool is_empty(const Range& range) noexcept
    {
        return range.is_end_pos(range.begin_pos());
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    range_value_type<Range> read(const Range& range, range_position_type<Range>& pos)
    {
        auto value = range.at_pos(pos);
        range.inc_pos(pos);
        return value;
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    range_value_type<Range> consume(const Range& range, range_position_type<Range>& pos)
    {
        auto value = stdext::move(range.at_pos(pos));
        range.inc_pos(pos);
        return value;
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    void write(const Range& range, range_position_type<Range>& pos, const range_value_type<Range>& value)
    {
        range.at_pos(pos) = value;
        range.inc_pos(pos);
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    void write(const Range& range, range_position_type<Range>& pos, range_value_type<Range>&& value)
    {
        range.at_pos(pos) = stdext::move(value);
        range.inc_pos(pos);
    }

    namespace _private
    {
        template <typename Range>
        range_size_type<Range> size_before(const Range& range, range_position_type<Range> pos, multi_pass_range_tag)
        {
            range_size_type<Range> size = 0;
            for (auto p = range.begin_pos(); p != pos; range.inc_pos(p))
                ++size;
            return size;
        }

        template <typename Range>
        range_size_type<Range> size_before(const Range& range, range_position_type<Range> pos, random_access_range_tag)
        {
            return range_size_type<Range>(range.distance(range.begin_pos(), pos));
        }

        template <typename Range>
        range_size_type<Range> size_after(const Range& range, range_position_type<Range> pos, multi_pass_range_tag)
        {
            range_size_type<Range> size = 0;
            for (auto p = pos; !range.is_end_pos(p); range.inc_pos(p))
                ++size;
            return size;
        }

        template <typename Range, STDEXT_REQUIRES(is_delimited_range<Range>::value)>
        range_size_type<Range> size_after(const Range& range, range_position_type<Range> pos, random_access_range_tag)
        {
            return range_size_type<Range>(range.distance(pos, range.end_pos()));
        }
    }

    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    range_size_type<Range> size_before(const Range& range, range_position_type<Range> pos)
    {
        return _private::size_before(range, pos, range_category<Range>());
    }

    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    range_size_type<Range> size_after(const Range& range, range_position_type<Range> pos)
    {
        return _private::size_after(range, pos, range_category<Range>());
    }

    template <typename Range, STDEXT_REQUIRES(is_multi_pass_range<Range>::value)>
    Range& drop_first(Range& range, range_size_type<Range> n = 1)
    {
        auto pos = range.begin_pos();
        range.begin_pos(advance_pos(range, pos, n));
        return range;
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value && is_delimited_range<Range>::value)>
    Range& drop_last(Range& range, range_size_type<Range> n = 1)
    {
        auto pos = range.end_pos();
        range.end_pos(advance_pos(range, pos, -range_difference_type<Range>(n)));
        return range;
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    range_reference_type<Range> head(const Range& range)
    {
        return range.at_pos(range.begin_pos());
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value && is_delimited_range<Range>::value)>
    range_reference_type<Range> last(const Range& range)
    {
        auto pos = range.end_pos();
        return range.at_pos(range.dec_pos(pos));
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    Range tail(const Range& range)
    {
        Range result = range;
        return drop_first(result);
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value && is_delimited_range<Range>::value)>
    Range init(const Range& range)
    {
        Range result = range;
        return drop_last(result);
    }

    // adapter ranges
    namespace _private
    {
        template <typename Range, typename TerminationPredicate> class delegated_multi_pass_range;
        template <typename Range, typename TerminationPredicate> class delegated_bidirectional_range;
        template <typename Range, typename TerminationPredicate> class delegated_random_access_range;

        template <typename Range> class delimited_multi_pass_range;
        template <typename Range> class delimited_bidirectional_range;
        template <typename Range> class delimited_random_access_range;

        template <typename Range> class counted_multi_pass_range;
        template <typename Range> class counted_bidirectional_range;
        template <typename Range> class counted_random_access_range;

        template <typename Range, typename TerminationPredicate> class reverse_bidirectional_range;
        template <typename Range, typename TerminationPredicate> class reverse_random_access_range;

        template <typename Range> class delimited_reverse_bidirectional_range;
        template <typename Range> class delimited_reverse_random_access_range;

        template <typename Range> class counted_reverse_bidirectional_range;
        template <typename Range> class counted_reverse_random_access_range;

        template <typename Iterator, typename TerminationPredicate> class forward_iterator_range;
        template <typename Iterator, typename TerminationPredicate> class bidirectional_iterator_range;
        template <typename Iterator, typename TerminationPredicate> class random_access_iterator_range;

        template <typename Iterator> class delimited_forward_iterator_range;
        template <typename Iterator> class delimited_bidirectional_iterator_range;
        template <typename Iterator> class delimited_random_access_iterator_range;

        template <typename Iterator> class counted_forward_iterator_range;
        template <typename Iterator> class counted_bidirectional_iterator_range;
        template <typename Iterator> class counted_random_access_iterator_range;

        template <typename IteratorCategory> struct iterator_range_category_map;
        template <> struct iterator_range_category_map<std::forward_iterator_tag> { using type = multi_pass_range_tag; };
        template <> struct iterator_range_category_map<std::bidirectional_iterator_tag> { using type = bidirectional_range_tag; };
        template <> struct iterator_range_category_map<std::random_access_iterator_tag> { using type = random_access_range_tag; };
        template <typename IteratorCategory> using iterator_range_category_map_t = typename iterator_range_category_map<IteratorCategory>::type;

        template <typename Range> class multi_pass_range_iterator;
        template <typename Range> class bidirectional_range_iterator;
        template <typename Range> class random_access_range_iterator;

        template <typename RangeCategory> struct range_iterator_category_map;
        template <> struct range_iterator_category_map<multi_pass_range_tag> { using type = std::forward_iterator_tag; };
        template <> struct range_iterator_category_map<bidirectional_range_tag> { using type = std::bidirectional_iterator_tag; };
        template <> struct range_iterator_category_map<random_access_range_tag> { using type = std::random_access_iterator_tag; };
        template <typename RangeCategory> using range_iterator_category_map_t = typename range_iterator_category_map<RangeCategory>::type;
    }

    template <typename Range, typename TerminationPredicate> using delegated_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::delegated_random_access_range<Range, TerminationPredicate>,
            std::conditional_t<is_bidirectional_range<Range>::value, _private::delegated_bidirectional_range<Range, TerminationPredicate>,
                std::enable_if_t<is_multi_pass_range<Range>::value, _private::delegated_multi_pass_range<Range, TerminationPredicate>>
            >
        >;

    template <typename Range> using delimited_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::delimited_random_access_range<Range>,
            std::conditional_t<is_bidirectional_range<Range>::value, _private::delimited_bidirectional_range<Range>,
                std::enable_if_t<is_multi_pass_range<Range>::value, _private::delimited_multi_pass_range<Range>>
            >
        >;

    template <typename Range> using counted_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::counted_random_access_range<Range>,
            std::conditional_t<is_bidirectional_range<Range>::value, _private::counted_bidirectional_range<Range>,
                std::enable_if_t<is_multi_pass_range<Range>::value, _private::counted_multi_pass_range<Range>>
            >
        >;

    template <typename Range, typename TerminationPredicate> using reverse_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::reverse_random_access_range<Range, TerminationPredicate>,
            std::enable_if_t<is_bidirectional_range<Range>::value, _private::reverse_bidirectional_range<Range, TerminationPredicate>>
        >;

    template <typename Range> using delimited_reverse_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::delimited_reverse_random_access_range<Range>,
            std::enable_if_t<is_bidirectional_range<Range>::value, _private::delimited_reverse_bidirectional_range<Range>>
        >;

    template <typename Range> using counted_reverse_range =
        std::conditional_t<is_random_access_range<Range>::value, _private::counted_reverse_random_access_range<Range>,
            std::enable_if_t<is_bidirectional_range<Range>::value, _private::counted_reverse_bidirectional_range<Range>>
        >;

    template <typename Iterator, typename TerminationPredicate> using iterator_range =
        std::conditional_t<is_random_access_iterator<Iterator>::value, _private::random_access_iterator_range<Iterator, TerminationPredicate>,
            std::conditional_t<is_bidirectional_iterator<Iterator>::value, _private::bidirectional_iterator_range<Iterator, TerminationPredicate>,
                std::enable_if_t<is_forward_iterator<Iterator>::value, _private::forward_iterator_range<Iterator, TerminationPredicate>>
            >
        >;

    template <typename Iterator> using delimited_iterator_range =
        std::conditional_t<is_random_access_iterator<Iterator>::value, _private::delimited_random_access_iterator_range<Iterator>,
            std::conditional_t<is_bidirectional_iterator<Iterator>::value, _private::delimited_bidirectional_iterator_range<Iterator>,
                std::enable_if_t<is_forward_iterator<Iterator>::value, _private::delimited_forward_iterator_range<Iterator>>
            >
        >;

    template <typename Iterator> using counted_iterator_range =
        std::conditional_t<is_random_access_iterator<Iterator>::value, _private::counted_random_access_iterator_range<Iterator>,
            std::conditional_t<is_bidirectional_iterator<Iterator>::value, _private::counted_bidirectional_iterator_range<Iterator>,
                std::enable_if_t<is_forward_iterator<Iterator>::value, _private::counted_forward_iterator_range<Iterator>>
            >
        >;

    template <typename Range> using range_iterator =
        std::conditional_t<is_random_access_range<Range>::value, _private::random_access_range_iterator<Range>,
            std::conditional_t<is_bidirectional_range<Range>::value, _private::bidirectional_range_iterator<Range>,
                std::enable_if_t<is_multi_pass_range<Range>::value, _private::multi_pass_range_iterator<Range>>
            >
        >;

    template <typename Range> class range_generator;

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto subrange_from(const Range& range, range_position_type<Range> first)
    {
        return make_range(range, first, [](const auto& r, const auto& p) { return r.is_end_pos(p); });
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto subrange_to(const Range& range, range_position_type<Range> last)
    {
        return make_range(range, range.begin_pos(), last);
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto split_range(const Range& range, range_position_type<Range> pos)
    {
        return std::make_pair(subrange_to(range, pos), subrange_from(range, pos));
    }

    namespace _private
    {
        // Iterator ranges
        template <typename Iterator, typename TerminationPredicate>
        class forward_iterator_range
        {
        public:
            using range_category = _private::iterator_range_category_map_t<iterator_category<Iterator>>;
            using value_type = iterator_value_type<Iterator>;
            using position = Iterator;
            using difference_type = iterator_difference_type<Iterator>;
            using reference = iterator_reference_type<Iterator>;
            using iterator = Iterator;

            class sentinel
            {
            public:
                explicit sentinel(const forward_iterator_range& range) noexcept : range(&range) { }
                friend bool operator == (const iterator& i, const sentinel& s) noexcept
                {
                    return s.range->is_end_pos(i);
                }
                friend bool operator == (const sentinel& s, const iterator& i) noexcept
                {
                    return i == s;
                }
                friend bool operator != (const iterator& i, const sentinel& s) noexcept
                {
                    return !(i == s);
                }
                friend bool operator != (const sentinel& s, const iterator& i) noexcept
                {
                    return !(s == i);
                }
            private:
                forward_iterator_range* range;
            };

        public:
            forward_iterator_range() noexcept : first(), term() { }
            forward_iterator_range(iterator first, const TerminationPredicate& term)
                : first(first), term(term) { }
            forward_iterator_range(iterator first, TerminationPredicate&& term)
                : first(first), term(stdext::move(term)) { }

        public:
            friend bool operator == (const forward_iterator_range& a, const forward_iterator_range& b) noexcept
            {
                return a.first == b.first && a.term == b.term;
            }
            friend bool operator != (const forward_iterator_range& a, const forward_iterator_range& b) noexcept
            {
                return !(a == b);
            }

            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) noexcept(noexcept(first = pos)) { first = pos; }

            bool is_end_pos(position pos) const noexcept { return term(pos); }

            position& inc_pos(position& pos) const { return ++pos; }
            reference at_pos(position pos) const { return *pos; }

            iterator begin() const { return first; }
            sentinel end() const { return sentinel(*this); }

        private:
            position first;
            TerminationPredicate term;
        };

        template <typename Iterator, typename TerminationPredicate>
        class bidirectional_iterator_range : public forward_iterator_range<Iterator, TerminationPredicate>
        {
        public:
            using typename forward_iterator_range<Iterator, TerminationPredicate>::position;

        public:
            using forward_iterator_range<Iterator, TerminationPredicate>::forward_iterator_range;

        public:
            position& dec_pos(position& pos) const { return --pos; }
        };

        template <typename Iterator, typename TerminationPredicate>
        class random_access_iterator_range : public bidirectional_iterator_range<Iterator, TerminationPredicate>
        {
        public:
            using typename bidirectional_iterator_range<Iterator, TerminationPredicate>::position;
            using typename bidirectional_iterator_range<Iterator, TerminationPredicate>::difference_type;
            using typename bidirectional_iterator_range<Iterator, TerminationPredicate>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using bidirectional_iterator_range<Iterator, TerminationPredicate>::bidirectional_iterator_range;

        public:
            position& advance_pos(position& pos, difference_type n) const
            {
                std::advance(pos, n);
                return pos;
            }

            difference_type distance(position p1, position p2) const noexcept { return std::distance(p1, p2); }

            reference operator [] (size_type n) const { return this->begin_pos()[n]; }
        };

        template <typename Iterator>
        class delimited_forward_iterator_range
        {
        public:
            using range_category = _private::iterator_range_category_map_t<iterator_category<Iterator>>;
            using value_type = iterator_value_type<Iterator>;
            using position = Iterator;
            using difference_type = iterator_difference_type<Iterator>;
            using reference = iterator_reference_type<Iterator>;
            using iterator = Iterator;

        public:
            constexpr delimited_forward_iterator_range() noexcept : first(), last() { }
            constexpr delimited_forward_iterator_range(iterator first, iterator last) noexcept(noexcept(iterator(first)))
                : first(first), last(last) { }

        public:
            friend constexpr bool operator == (const delimited_forward_iterator_range& a, const delimited_forward_iterator_range& b) noexcept
            {
                return a.first == b.first && a.last == b.last;
            }
            friend constexpr bool operator != (const delimited_forward_iterator_range& a, const delimited_forward_iterator_range& b) noexcept
            {
                return !(a == b);
            }

            constexpr position begin_pos() const { return first; }
            constexpr void begin_pos(position pos) { first = pos; }

            constexpr position end_pos() const { return last; }
            constexpr void end_pos(position pos) { last = pos; }

            constexpr bool is_end_pos(position pos) const noexcept { return pos == last; }

            constexpr position& inc_pos(position& pos) const { return ++pos; }
            constexpr reference at_pos(position pos) const { return *pos; }

            constexpr iterator begin() const { return first; }
            constexpr iterator end() const { return last; }

        private:
            position first;
            position last;
        };

        template <typename Iterator>
        class delimited_bidirectional_iterator_range : public delimited_forward_iterator_range<Iterator>
        {
        public:
            using typename delimited_forward_iterator_range<Iterator>::position;
            using reverse_iterator = std::reverse_iterator<Iterator>;

        public:
            using delimited_forward_iterator_range<Iterator>::delimited_forward_iterator_range;

        public:
            constexpr position& dec_pos(position& pos) const { return --pos; }

            constexpr reverse_iterator rbegin() const { return std::make_reverse_iterator(this->end()); }
            constexpr reverse_iterator rend() const { return std::make_reverse_iterator(this->begin()); }
        };

        template <typename Iterator>
        class delimited_random_access_iterator_range : public delimited_bidirectional_iterator_range<Iterator>
        {
        public:
            using typename delimited_bidirectional_iterator_range<Iterator>::position;
            using typename delimited_bidirectional_iterator_range<Iterator>::difference_type;
            using typename delimited_bidirectional_iterator_range<Iterator>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using delimited_bidirectional_iterator_range<Iterator>::delimited_bidirectional_iterator_range;

        public:
            constexpr position& advance_pos(position& pos, difference_type n) const
            {
                std::advance(pos, n);
                return pos;
            }

            constexpr difference_type distance(position p1, position p2) const noexcept { return std::distance(p1, p2); }
            constexpr size_type size() const noexcept { return size_type(distance(this->begin_pos(), this->end_pos())); }
            constexpr void resize(size_type n) { this->end_pos(this->begin_pos() + n); }

            constexpr reference operator [] (size_type n) const { return this->begin_pos()[n]; }
            constexpr reference at(size_type n) const
            {
                if (n >= size())
                    throw std::out_of_range("element index out of range");
                return this->begin_pos()[n];
            }
        };

        template <typename Iterator>
        class counted_forward_iterator_range
        {
        public:
            using range_category = _private::iterator_range_category_map_t<iterator_category<Iterator>>;
            using value_type = iterator_value_type<Iterator>;
            using difference_type = iterator_difference_type<Iterator>;
            using reference = iterator_reference_type<Iterator>;
            using iterator = Iterator;

            using size_type = std::make_unsigned_t<difference_type>;
            using position = std::tuple<iterator, size_type>;

        public:
            counted_forward_iterator_range() : first(), count(0) { }
            counted_forward_iterator_range(iterator i, size_type count) : first(i, 0), count(count) { }

            friend bool operator == (const counted_forward_iterator_range& a, const counted_forward_iterator_range& b) noexcept
            {
                return a.first == b.first
                    && a.count == b.count;
            }
            friend bool operator != (const counted_forward_iterator_range& a, const counted_forward_iterator_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) noexcept(noexcept(first = pos)) { first = pos; }

            bool is_end_pos(position pos) const noexcept { return pos.second() == count; }

            position& inc_pos(position& pos) const
            {
                ++std::get<0>(pos);
                ++std::get<1>(pos);
                return pos;
            }

            reference at_pos(position pos) const { return *std::get<0>(pos); }

            size_type size() const noexcept { return count - std::get<1>(first); }
            void resize(size_type n) noexcept { count = std::get<1>(first) + n; }

            iterator base_pos(position pos) const { return std::get<0>(pos); }

        private:
            position first;
            size_type count;
        };

        template <typename Iterator>
        class counted_bidirectional_iterator_range : public counted_forward_iterator_range<Iterator>
        {
        public:
            using typename counted_forward_iterator_range<Iterator>::position;
            using typename counted_forward_iterator_range<Iterator>::difference_type;

        public:
            using counted_forward_iterator_range<Iterator>::counted_forward_iterator_range;

        public:
            position& dec_pos(position& pos) const
            {
                --pos.first();
                --pos.second();
                return pos;
            }
        };

        template <typename Iterator>
        class counted_random_access_iterator_range : public counted_bidirectional_iterator_range<Iterator>
        {
        public:
            using typename counted_bidirectional_iterator_range<Iterator>::position;
            using typename counted_bidirectional_iterator_range<Iterator>::difference_type;
            using typename counted_bidirectional_iterator_range<Iterator>::reference;
            using typename counted_bidirectional_iterator_range<Iterator>::size_type;

        public:
            using counted_bidirectional_iterator_range<Iterator>::counted_bidirectional_iterator_range;

        public:
            position end_pos() const
            {
                position pos = this->begin_pos();
                this->advance_pos(pos, this->size());
                return pos;
            }
            void end_pos(position pos) noexcept
            {
                this->resize(pos.second() - this->begin_pos().second());
            }

            position& advance_pos(position& pos, difference_type n) const
            {
                std::advance(pos.first(), n);
                pos.second() += n;
                return pos;
            }

            difference_type distance(position p1, position p2) const noexcept
            {
                return difference_type(p2.second() - p1.second());
            }

            reference operator [] (size_type n) const { return *(this->begin_pos().first() + n); }
            reference at(size_type n) const
            {
                if (n >= this->size())
                    throw std::out_of_range("element index out of range");
                return (*this)[n];
            }
        };

        // Range iterators
        template <typename Range>
        class multi_pass_range_iterator
        {
        public:
            using iterator_category = _private::range_iterator_category_map<range_category<Range>>;
            using value_type = stdext::range_value_type<Range>;
            using difference_type = stdext::range_difference_type<Range>;
            using pointer = value_type*;
            using reference = stdext::range_reference_type<Range>;
            using position = stdext::range_position_type<Range>;
            using range = Range;

        public:
            multi_pass_range_iterator() : r(nullptr), p() { }
            multi_pass_range_iterator(const range& r, position pos) noexcept(noexcept(position(pos)))
                : r(&r), p(pos) { }

            friend bool operator == (const multi_pass_range_iterator& a, const multi_pass_range_iterator& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.p == b.p;
            }
            friend bool operator != (const multi_pass_range_iterator& a, const multi_pass_range_iterator& b) noexcept
            {
                return !(a == b);
            }

        public:
            reference operator * () const { return r->at_pos(p); }
            pointer operator -> () const { return std::addressof(r->at_pos(p)); }
            multi_pass_range_iterator& operator ++ ()
            {
                r->inc_pos(p);
                return *this;
            }
            multi_pass_range_iterator operator ++ (int)
            {
                multi_pass_range_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            const range& base_range() const noexcept { return *r; }
            position base_pos() const noexcept { return p; }

            void swap(multi_pass_range_iterator& other)
                noexcept(noexcept(swap(p, other.p)))
            {
                swap(r, other.r);
                swap(p, other.p);
            }

            friend void swap(multi_pass_range_iterator& a, multi_pass_range_iterator& b)
                noexcept(noexcept(a.swap(b)))
            {
                a.swap(b);
            }

        private:
            friend class multi_pass_range_iterator<Range>;
            friend class bidirectional_range_iterator<Range>;
            friend class random_access_range_iterator<Range>;

            const range* r;
            position p;
        };

        template <typename Range>
        class bidirectional_range_iterator : public multi_pass_range_iterator<Range>
        {
        public:
            using multi_pass_range_iterator<Range>::multi_pass_range_iterator;

        public:
            bidirectional_range_iterator& operator -- ()
            {
                this->r->dec_pos(this->p);
                return *this;
            }
            bidirectional_range_iterator operator -- (int)
            {
                bidirectional_range_iterator tmp = *this;
                --*this;
                return tmp;
            }
        };

        template <typename Range>
        class random_access_range_iterator : public bidirectional_range_iterator<Range>
        {
        public:
            using typename bidirectional_range_iterator<Range>::difference_type;
            using typename bidirectional_range_iterator<Range>::reference;
            using typename bidirectional_range_iterator<Range>::position;

        public:
            using bidirectional_range_iterator<Range>::bidirectional_range_iterator;

            friend bool operator <  (const random_access_range_iterator& a, const random_access_range_iterator& b) noexcept
            {
                if (a.r == nullptr)
                    return b.r != nullptr;
                if (b.r == nullptr)
                    return false;

                assert(*a.r == *b.r);
                return a.r->distance(a.p, b.p) < 0;
            }
            friend bool operator >  (const random_access_range_iterator& a, const random_access_range_iterator& b) noexcept
            {
                return b < a;
            }
            friend bool operator <= (const random_access_range_iterator& a, const random_access_range_iterator& b) noexcept
            {
                return !(b < a);
            }
            friend bool operator >= (const random_access_range_iterator& a, const random_access_range_iterator& b) noexcept
            {
                return !(a < b);
            }

        public:
            random_access_range_iterator& operator += (difference_type n)
            {
                this->r->advance_pos(this->p, n);
                return *this;
            }
            random_access_range_iterator& operator -= (difference_type n)
            {
                this->r->advance_pos(this->p, -n);
                return *this;
            }

            reference operator [] (difference_type n) const
            {
                position pos = this->p;
                this->r->advance_pos(pos, n);
                return this->r->at_pos(pos);
            }

            friend random_access_range_iterator operator + (random_access_range_iterator it, difference_type n)
            {
                return it += n;
            }
            friend random_access_range_iterator operator + (difference_type n, random_access_range_iterator it)
            {
                return it += n;
            }
            friend random_access_range_iterator operator - (random_access_range_iterator it, difference_type n)
            {
                return it -= n;
            }
            friend difference_type operator - (const random_access_range_iterator& a, const random_access_range_iterator& b)
            {
                if (a.r == nullptr && b.r == nullptr)
                    return 0;

                assert(*a.r == *b.r);
                return a.r->distance(a.p, b.p);
            }
        };

        // delegated ranges
        template <typename Range, typename TerminationPredicate>
        class delegated_multi_pass_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using position = range_position_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

        public:
            delegated_multi_pass_range() : r(nullptr), first(), term() { }
            delegated_multi_pass_range(const range& r, position first, const TerminationPredicate& term)
                : r(&r), first(first), term(term) { }
            delegated_multi_pass_range(const range& r, position first, TerminationPredicate&& term)
                : r(&r), first(first), term(stdext::move(term)) { }

            friend bool operator == (const delegated_multi_pass_range& a, const delegated_multi_pass_range& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.first == b.first
                    && a.term == b.term;
            }
            friend bool operator != (const delegated_multi_pass_range& a, const delegated_multi_pass_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) { first = pos; }
            bool is_end_pos(position pos) const noexcept { return r != nullptr && term(*r, pos); }
            position& inc_pos(position& pos) const { return r->inc_pos(pos); }
            reference at_pos(position pos) const { return r->at_pos(pos); }
            const range& base() const noexcept { return *r; }

        private:
            friend class delegated_multi_pass_range<Range, TerminationPredicate>;
            friend class delegated_bidirectional_range<Range, TerminationPredicate>;
            friend class delegated_random_access_range<Range, TerminationPredicate>;

            const range* r;
            position first;
            TerminationPredicate term;
        };

        template <typename Range, typename TerminationPredicate>
        class delegated_bidirectional_range : public delegated_multi_pass_range<Range, TerminationPredicate>
        {
        public:
            using typename delegated_multi_pass_range<Range, TerminationPredicate>::position;
            using typename delegated_multi_pass_range<Range, TerminationPredicate>::difference_type;

        public:
            using delegated_multi_pass_range<Range, TerminationPredicate>::delegated_multi_pass_range;

        public:
            position& dec_pos(position& pos) const { return this->r->dec_pos(pos); }
        };

        template <typename Range, typename TerminationPredicate>
        class delegated_random_access_range : public delegated_bidirectional_range<Range, TerminationPredicate>
        {
        public:
            using typename delegated_bidirectional_range<Range, TerminationPredicate>::position;
            using typename delegated_bidirectional_range<Range, TerminationPredicate>::difference_type;
            using typename delegated_bidirectional_range<Range, TerminationPredicate>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using delegated_bidirectional_range<Range, TerminationPredicate>::delegated_bidirectional_range;

        public:
            position& advance_pos(position& pos, difference_type n) const { return this->r->advance_pos(pos, n); }
            difference_type distance(position p1, position p2) const noexcept { return this->r->distance(p1, p2); }

            reference operator [] (size_type n) const
            {
                position pos = this->first;
                this->r->advance_pos(pos, n);
                return this->r->at_pos(n);
            }
        };

        // delimited ranges
        template <typename Range>
        class delimited_multi_pass_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using position = range_position_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

        public:
            delimited_multi_pass_range() : r(nullptr), first(), last() { }
            delimited_multi_pass_range(const range& r, position first, position last)
                : r(&r), first(first), last(last) { }

            friend bool operator == (const delimited_multi_pass_range& a, const delimited_multi_pass_range& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.first == b.first
                    && a.last == b.last;
            }
            friend bool operator != (const delimited_multi_pass_range& a, const delimited_multi_pass_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) { first = pos; }

            position end_pos() const noexcept { return last; }
            void end_pos(position pos) { last = pos; }

            bool is_end_pos(position pos) const noexcept { return pos == last; }

            position& inc_pos(position& pos) const { return r->inc_pos(pos); }
            reference at_pos(position pos) const { return r->at_pos(pos); }

            const range& base() const noexcept { return *r; }

        private:
            friend class delimited_bidirectional_range<Range>;
            friend class delimited_random_access_range<Range>;

            range* r;
            position first, last;
        };

        template <typename Range>
        class delimited_bidirectional_range : public delimited_multi_pass_range<Range>
        {
        public:
            using typename delimited_multi_pass_range<Range>::position;
            using typename delimited_multi_pass_range<Range>::difference_type;

        public:
            using delimited_multi_pass_range<Range>::delimited_multi_pass_range;

        public:
            position& dec_pos(position& pos) const { return this->r->dec_pos(pos); }
        };

        template <typename Range>
        class delimited_random_access_range : public delimited_bidirectional_range<Range>
        {
        public:
            using typename delimited_bidirectional_range<Range>::position;
            using typename delimited_bidirectional_range<Range>::difference_type;
            using typename delimited_bidirectional_range<Range>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using delimited_bidirectional_range<Range>::delimited_bidirectional_range;

        public:
            position& advance_pos(position& pos, difference_type n) const { return this->r->advance_pos(pos, n); }
            difference_type distance(position p1, position p2) const noexcept { return this->r->distance(p1, p2); }
            size_type size() const noexcept { return size_type(this->r->distance(this->first, this->last)); }
            void resize(size_type n)
            {
                this->last = this->first;
                this->r->advance_pos(last, n);
            }

            reference operator [] (size_type n) const
            {
                position pos = this->first;
                this->r->advance_pos(pos, n);
                return this->r->at_pos(n);
            }
            reference at(size_type n) const
            {
                if (n >= size())
                    throw std::out_of_range("element index out of range");
                return (*this)[n];
            }
        };

        // counted ranges
        template <typename Range>
        class counted_multi_pass_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

            using size_type = std::make_unsigned_t<difference_type>;
            using position = std::tuple<range_position_type<Range>, size_type>;

        public:
            counted_multi_pass_range() : underlying(nullptr), first(), count(0) { }
            counted_multi_pass_range(const range& r, range_position_type<Range> pos, size_type count) : underlying(&r), first(pos, 0), count(count) { }

            friend bool operator == (const counted_multi_pass_range& a, const counted_multi_pass_range& b) noexcept
            {
                return (a.underlying == b.underlying || (a.underlying != nullptr && b.underlying != nullptr && *a.underlying == *b.underlying))
                    && a.count == b.count;
            }
            friend bool operator != (const counted_multi_pass_range& a, const counted_multi_pass_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) noexcept(noexcept(first = pos)) { first = pos; }

            bool is_end_pos(position pos) const noexcept { return std::get<1>(pos) == count; }

            position& inc_pos(position& pos) const
            {
                underlying->inc_pos(std::get<0>(pos));
                ++std::get<1>(pos);
                return pos;
            }

            reference at_pos(position pos) const
            {
                return underlying->at_pos(std::get<0>(pos));
            }

            const range& base() const noexcept { return underlying; }
            range_position_type<range> base_pos(position pos) { return std::get<0>(pos); }

            size_type size() const noexcept { return count - std::get<1>(first); }
            void resize(size_type n) noexcept { count = std::get<1>(first) + n; }

        private:
            const range* underlying;
            position first;
            size_type count;
        };

        template <typename Range>
        class counted_bidirectional_range : public counted_multi_pass_range<Range>
        {
        public:
            using typename counted_multi_pass_range<Range>::position;
            using typename counted_multi_pass_range<Range>::difference_type;

        public:
            using counted_multi_pass_range<Range>::counted_multi_pass_range;

        public:
            position& dec_pos(position& pos) const
            {
                this->base().dec_pos(pos.first());
                --pos.second();
                return pos;
            }
        };

        template <typename Range>
        class counted_random_access_range : public counted_bidirectional_range<Range>
        {
        public:
            using typename counted_bidirectional_range<Range>::position;
            using typename counted_bidirectional_range<Range>::difference_type;
            using typename counted_bidirectional_range<Range>::reference;
            using typename counted_bidirectional_range<Range>::size_type;

        public:
            using counted_bidirectional_range<Range>::counted_bidirectional_range;

        public:
            position end_pos() const
            {
                position pos = this->begin_pos();
                this->advance_pos(pos, this->size());
                return pos;
            }
            void end_pos(position pos) noexcept
            {
                this->resize(pos.second() - this->begin_pos().second());
            }

            position& advance_pos(position& pos, difference_type n) const
            {
                this->base().advance_pos(pos.first(), n);
                pos.second() += n;
                return pos;
            }

            difference_type distance(position p1, position p2) const noexcept
            {
                return p2.second() - p1.second();
            }

            reference operator [] (size_type n) const
            {
                position pos = this->begin_pos();
                this->advance_pos(pos, n);
                this->at_pos(pos);
            }
            reference at(size_type n) const
            {
                if (n >= this->size())
                    throw std::out_of_range("element index out of range");
                return (*this)[n];
            }
        };

        // reverse ranges
        template <typename Range, typename TerminationPredicate>
        class reverse_bidirectional_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using position = range_position_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

        public:
            reverse_bidirectional_range() : r(nullptr), first(), term() { }
            reverse_bidirectional_range(const Range& range, range_position_type<Range> first, const TerminationPredicate& term)
                : r(&range), first(first), term(term) { }
            reverse_bidirectional_range(const Range& range, range_position_type<Range> first, TerminationPredicate&& term)
                : r(&range), first(first), term(stdext::move(term)) { }

            friend bool operator == (const reverse_bidirectional_range& a, const reverse_bidirectional_range& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.first == b.first
                    && a.term == b.term;
            }
            friend bool operator != (const reverse_bidirectional_range& a, const reverse_bidirectional_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) { first = pos; }

            bool is_end_pos(position pos) const noexcept { return r != nullptr && term(*r, pos); }

            position& inc_pos(position& pos) const { return r->dec_pos(pos); }
            position& dec_pos(position& pos) const { return r->inc_pos(pos); }
            reference at_pos(position pos) const { return r->at_pos(prev_pos(*r, pos)); }

            const range& base() { return *r; }

        private:
            friend class reverse_random_access_range<Range, TerminationPredicate>;

            const range* r;
            position first;
            TerminationPredicate term;
        };

        template <typename Range, typename TerminationPredicate>
        class reverse_random_access_range : public reverse_bidirectional_range<Range, TerminationPredicate>
        {
        public:
            using position = typename reverse_bidirectional_range<Range, TerminationPredicate>::position;
            using difference_type = typename reverse_bidirectional_range<Range, TerminationPredicate>::difference_type;
            using reference = typename reverse_bidirectional_range<Range, TerminationPredicate>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using reverse_bidirectional_range<Range, TerminationPredicate>::reverse_bidirectional_range;

        public:
            position& advance_pos(position& pos, difference_type n) const
            {
                return this->r->advance_pos(pos, -n);
            }

            difference_type distance(position p1, position p2) const noexcept
            {
                return this->r->distance(p2, p1);
            }

            reference operator [] (size_type n)
            {
                auto pos = this->begin_pos();
                this->advance_pos(pos, n);
                return this->at_pos(pos);
            }
        };

        template <typename Range>
        class delimited_reverse_bidirectional_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using position = range_position_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

        public:
            delimited_reverse_bidirectional_range() : r(nullptr), first(), last() { }
            delimited_reverse_bidirectional_range(const Range& range, range_position_type<Range> first, range_position_type<Range> last)
                : r(&range), first(first), last(last) { }

            friend bool operator == (const delimited_reverse_bidirectional_range& a, const delimited_reverse_bidirectional_range& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.first == b.first
                    && a.last == b.last;
            }
            friend bool operator != (const delimited_reverse_bidirectional_range& a, const delimited_reverse_bidirectional_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) { first = pos; }

            position end_pos() const noexcept { return last; }
            void end_pos(position pos) { last = pos; }

            bool is_end_pos(position pos) const noexcept { return pos == last; }

            position& inc_pos(position& pos) const { return r->dec_pos(pos); }
            position& dec_pos(position& pos) const { return r->inc_pos(pos); }
            reference at_pos(position pos) const { return r->at_pos(prev_pos(*r, pos)); }

            const range& base() { return *r; }

        private:
            friend class delimited_reverse_random_access_range<Range>;

            const range* r;
            position first, last;
        };

        template <typename Range>
        class delimited_reverse_random_access_range : public delimited_reverse_bidirectional_range<Range>
        {
        public:
            using position = typename delimited_reverse_bidirectional_range<Range>::position;
            using difference_type = typename delimited_reverse_bidirectional_range<Range>::difference_type;
            using reference = typename delimited_reverse_bidirectional_range<Range>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using delimited_reverse_bidirectional_range<Range>::delimited_reverse_bidirectional_range;

        public:
            position& advance_pos(position& pos, difference_type n) const
            {
                return this->r->advance_pos(pos, -n);
            }

            difference_type distance(position p1, position p2) const noexcept
            {
                return this->r->distance(p2, p1);
            }

            size_type size() const noexcept
            {
                return size_type(distance(this->first, this->last));
            }

            void resize(size_type n)
            {
                this->last = this->first;
                advance_pos(this->last, n);
            }

            reference operator [] (size_type n)
            {
                auto pos = this->begin_pos();
                this->advance_pos(pos, n);
                return this->at_pos(pos);
            }
            reference at(size_type n)
            {
                if (n >= size())
                    throw std::out_of_range("element index out of range");
                return (*this)[n];
            }
        };

        template <typename Range>
        class counted_reverse_bidirectional_range
        {
        public:
            using range_category = range_category<Range>;
            using value_type = range_value_type<Range>;
            using difference_type = range_difference_type<Range>;
            using reference = range_reference_type<Range>;
            using range = Range;

            using size_type = std::make_unsigned_t<difference_type>;
            using position = std::tuple<range_position_type<Range>, size_type>;

        public:
            counted_reverse_bidirectional_range() : r(nullptr), first(), count(0) { }
            counted_reverse_bidirectional_range(const Range& range, range_position_type<Range> first, size_type count)
                : r(&range), first(first, 0), count(count) { }

            friend bool operator == (const counted_reverse_bidirectional_range& a, const counted_reverse_bidirectional_range& b) noexcept
            {
                return (a.r == b.r || (a.r != nullptr && b.r != nullptr && *a.r == *b.r))
                    && a.first == b.first
                    && a.count == b.count;
            }
            friend bool operator != (const counted_reverse_bidirectional_range& a, const counted_reverse_bidirectional_range& b) noexcept
            {
                return !(a == b);
            }

        public:
            position begin_pos() const noexcept { return first; }
            void begin_pos(position pos) { first = pos; }

            bool is_end_pos(position pos) const noexcept { return std::get<1>(pos) == count; }

            position& inc_pos(position& pos) const
            {
                r->dec_pos(std::get<0>(pos));
                ++std::get<1>(pos);
                return pos;
            }

            position& dec_pos(position& pos) const
            {
                r->inc_pos(std::get<0>(pos));
                --std::get<1>(pos);
                return pos;
            }

            reference at_pos(position pos) const { return r->at_pos(prev_pos(*r, std::get<0>(pos))); }

            size_type size() const noexcept { return count - std::get<1>(first); }
            void resize(size_type n) { count = std::get<1>(first) + n; }

            const range& base() const noexcept { return *r; }
            range_position_type<Range> base_pos(position pos) const { return std::get<1>(pos); }

        private:
            friend class counted_reverse_random_access_range<Range>;

            const range* r;
            position first;
            size_type count;
        };

        template <typename Range>
        class counted_reverse_random_access_range : public counted_reverse_bidirectional_range<Range>
        {
        public:
            using position = typename counted_reverse_bidirectional_range<Range>::position;
            using difference_type = typename counted_reverse_bidirectional_range<Range>::difference_type;
            using reference = typename counted_reverse_bidirectional_range<Range>::reference;
            using size_type = std::make_unsigned_t<difference_type>;

        public:
            using counted_reverse_bidirectional_range<Range>::counted_reverse_bidirectional_range;

        public:
            position& advance_pos(position& pos, difference_type n) const
            {
                this->r->advance_pos(pos.first(), -n);
                pos.second() += n;
                return pos;
            }

            difference_type distance(position p1, position p2) const noexcept
            {
                return p2.second() - p1.second();
            }

            position end_pos() const
            {
                auto pos = this->begin_pos();
                advance_pos(pos, this->size());
                return pos;
            }

            void end_pos(position pos)
            {
                this->resize(pos.second() - this->begin_pos().second());
            }

            reference operator [] (size_type n)
            {
                auto pos = this->begin_pos();
                advance_pos(pos, n);
                return this->at_pos(pos);
            }
            reference at(size_type n)
            {
                if (n >= this->size())
                    throw std::out_of_range("element index out of range");
                return (*this)[n];
            }
        };
    }

    // range generator

    template <typename Range>
    class range_generator
    {
    public:
        using iterator_category = generator_tag;
        using value_type = stdext::range_value_type<Range>;
        using difference_type = stdext::range_difference_type<Range>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using range = Range;

    public:
        range_generator() : r() { }
        range_generator(range& r) : r(&r) { }

    public:
        friend bool operator == (const range_generator& a, const range_generator& b)
        {
            return a.r == b.r;
        }
        friend bool operator != (const range_generator& a, const range_generator& b)
        {
            return !(a == b);
        }

        friend void swap(range_generator& a, range_generator& b)
        {
            swap(a.r, b.r);
        }

    public:
        reference operator * () const { return head(*r); }
        pointer operator -> () const { return &head(*r); }
        range_generator& operator ++ () { drop_first(*r); return *this; }
        iterator_proxy<range_generator> operator ++ (int)
        {
            iterator_proxy<range_generator> value(head(*r));
            drop_first(*r);
            return value;
        }
        explicit operator bool () const { return r != nullptr && !is_empty(*r); }

    private:
        range* r;
    };

    template <typename Range, typename TerminationPredicate, STDEXT_REQUIRES(is_range<Range>::value)>
    auto make_range(const Range& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return delegated_range<Range, std::decay_t<TerminationPredicate>>(range, pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename DelegatedPredicate, typename TerminationPredicate>
    auto make_range(const delegated_range<Range, DelegatedPredicate>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return delegated_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename TerminationPredicate>
    auto make_range(const delimited_range<Range>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return delegated_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename DelegatedPredicate, typename TerminationPredicate>
    auto make_range(const reverse_range<Range, DelegatedPredicate>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return reverse_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename TerminationPredicate>
    auto make_range(const delimited_reverse_range<Range>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return reverse_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Iterator, typename DelegatedPredicate, typename TerminationPredicate>
    auto make_range(const iterator_range<Iterator, DelegatedPredicate>& range, identity_type_t<Iterator> pos, TerminationPredicate&& pred)
    {
        discard(range);
        return iterator_range<Iterator, std::decay_t<TerminationPredicate>>(pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Iterator, typename TerminationPredicate>
    auto make_range(const delimited_iterator_range<Iterator>& range, identity_type_t<Iterator> pos, TerminationPredicate&& pred)
    {
        discard(range);
        return iterator_range<Iterator, std::decay_t<TerminationPredicate>>(pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto make_range(const Range& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_range<Range>(range, p1, p2);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_range(const delegated_range<Range, TerminationPredicate>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_range<Range>(range.base(), p1, p2);
    }

    template <typename Range>
    auto make_range(const delimited_range<Range>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_range<Range>(range.base(), p1, p2);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_range(const reverse_range<Range, TerminationPredicate>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_reverse_range<Range>(range, p1, p2);
    }

    template <typename Range>
    auto make_range(const delimited_reverse_range<Range>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_reverse_range<Range>(range, p1, p2);
    }

    template <typename Iterator, typename TerminationPredicate>
    auto make_range(const iterator_range<Iterator, TerminationPredicate>& range, identity_type_t<Iterator> p1, identity_type_t<Iterator> p2)
    {
        discard(range);
        return delimited_iterator_range<Iterator>(p1, p2);
    }

    template <typename Iterator>
    auto make_range(const delimited_iterator_range<Iterator>& range, identity_type_t<Iterator> p1, identity_type_t<Iterator> p2)
    {
        discard(range);
        return delimited_iterator_range<Iterator>(p1, p2);
    }

    template <typename Iterator, typename TerminationPredicate, STDEXT_REQUIRES(is_iterator<Iterator>::value)>
    auto make_range(Iterator i, TerminationPredicate&& pred)
    {
        return iterator_range<Iterator, std::decay_t<TerminationPredicate>>(i, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Iterator, STDEXT_REQUIRES(is_iterator<Iterator>::value)>
    auto make_range(Iterator i, Iterator j)
    {
        return delimited_iterator_range<Iterator>(i, j);
    }

    template <typename RangeProvider, STDEXT_REQUIRES(is_stl_range_provider<RangeProvider>::value)>
    auto make_range(RangeProvider& provider)
    {
        return make_range(provider.begin(), provider.end());
    }

    template <typename Range, typename TerminationPredicate>
    auto make_range(range_iterator<Range> i, TerminationPredicate&& pred)
    {
        return make_range(i.base_range(), i.base_pos(), stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range>
    auto make_range(range_iterator<Range> i, range_iterator<Range> j)
    {
        assert(i.base_range() == j.base_range());
        return make_range(i.base_range(), i.base_pos(), j.base_pos());
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto make_counted_range(const Range& range, range_position_type<Range> pos, range_size_type<Range> count)
    {
        return counted_range<Range>(range, pos, count);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_counted_range(const delegated_range<Range, TerminationPredicate>& range, range_position_type<Range> pos, range_size_type<Range> count)
    {
        return counted_range<Range>(range.base(), pos, count);
    }

    template <typename Range>
    auto make_counted_range(const delimited_range<Range>& range, range_position_type<Range> pos, range_size_type<Range> count)
    {
        return counted_range<Range>(range.base(), pos, count);
    }

    template <typename Range>
    auto make_counted_range(const counted_range<Range>& range, range_position_type<counted_range<Range>> pos, range_size_type<counted_range<Range>> count)
    {
        return counted_range<Range>(range.base(), range.base_pos(pos), count);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_counted_range(const reverse_range<Range, TerminationPredicate>& range, range_position_type<Range> pos, range_size_type<Range> count)
    {
        return counted_reverse_range<Range>(range.base(), pos, count);
    }

    template <typename Range>
    auto make_counted_range(const delimited_reverse_range<Range>& range, range_position_type<Range> pos, range_size_type<Range> count)
    {
        return counted_reverse_range<Range>(range.base(), pos, count);
    }

    template <typename Range>
    auto make_counted_range(const counted_reverse_range<Range>& range, range_position_type<counted_reverse_range<Range>> pos, range_size_type<counted_reverse_range<Range>> count)
    {
        return counted_reverse_range<Range>(range.base(), range.base_pos(pos), count);
    }

    template <typename Iterator, typename TerminationPredicate>
    auto make_counted_range(const iterator_range<Iterator, TerminationPredicate>& range, identity_type_t<Iterator> pos, iterator_size_type<Iterator> count)
    {
        discard(range);
        return counted_iterator_range<Iterator>(pos, count);
    }

    template <typename Iterator>
    auto make_counted_range(const delimited_iterator_range<Iterator>& range, identity_type_t<Iterator> pos, iterator_size_type<Iterator> count)
    {
        discard(range);
        return counted_iterator_range<Iterator>(pos, count);
    }

    template <typename Iterator>
    auto make_counted_range(const counted_iterator_range<Iterator>& range, range_position_type<counted_iterator_range<Iterator>> pos, range_size_type<counted_iterator_range<Iterator>> count)
    {
        return counted_iterator_range<Iterator>(range.base_pos(pos), count);
    }

    template <typename Iterator, STDEXT_REQUIRES(is_iterator<Iterator>::value)>
    auto make_counted_range(Iterator it, iterator_size_type<Iterator> count)
    {
        return counted_iterator_range<Iterator>(it, count);
    }

    template <typename Range>
    auto make_counted_range(range_iterator<Range> it, range_size_type<Range> count)
    {
        return make_counted_range(it.base_range(), it.base_pos(), count);
    }

    template <typename Range, typename TerminationPredicate, STDEXT_REQUIRES(is_bidirectional_range<Range>::value)>
    auto make_reverse_range(const Range& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return reverse_range<Range, std::decay_t<TerminationPredicate>>(range, pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename DelegatedPredicate, typename TerminationPredicate>
    auto make_reverse_range(const delegated_range<Range, DelegatedPredicate>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return reverse_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename TerminationPredicate>
    auto make_reverse_range(const delimited_range<Range>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return reverse_range<Range, std::decay_t<TerminationPredicate>>(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename DelegatedPredicate, typename TerminationPredicate>
    auto make_reverse_range(const reverse_range<Range, DelegatedPredicate>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return make_range(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename TerminationPredicate>
    auto make_reverse_range(const delimited_reverse_range<Range>& range, range_position_type<Range> pos, TerminationPredicate&& pred)
    {
        return make_range(range.base(), pos, stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, typename TerminationPredicate>
    auto make_reverse_range(const counted_reverse_range<Range>& range, range_position_type<counted_reverse_range<Range>> pos, TerminationPredicate&& pred)
    {
        return make_range(range.base(), range.base_pos(pos), stdext::forward<TerminationPredicate>(pred));
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value)>
    auto make_reverse_range(const Range& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_reverse_range<Range>(range, p1, p2);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_reverse_range(const delegated_range<Range, TerminationPredicate>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_reverse_range<Range>(range.base(), p1, p2);
    }

    template <typename Range>
    auto make_reverse_range(const delimited_range<Range>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return delimited_reverse_range<Range>(range.base(), p1, p2);
    }

    template <typename Range, typename TerminationPredicate>
    auto make_reverse_range(const reverse_range<Range, TerminationPredicate>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return make_range(range.base(), p2, p1);
    }

    template <typename Range>
    auto make_reverse_range(const delimited_reverse_range<Range>& range, range_position_type<Range> p1, range_position_type<Range> p2)
    {
        return make_range(range.base(), p2, p1);
    }

    template <typename Range>
    auto make_reverse_range(const counted_reverse_range<Range>& range, range_position_type<counted_reverse_range<Range>> p1, range_position_type<counted_reverse_range<Range>> p2)
    {
        return make_range(range.base(), range.base_pos(p2), range.base_pos(p1));
    }

    template <typename Range, STDEXT_REQUIRES(is_bidirectional_range<Range>::value && is_delimited_range<Range>::value)>
    auto make_reverse_range(const Range& range)
    {
        return make_reverse_range(range, range.begin_pos(), range.end_pos());
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value)>
    auto make_iterator(const Range& range, range_position_type<Range> pos)
    {
        return range_iterator<Range>(range, pos);
    }

    template <typename Iterator, typename TerminationPredicate>
    auto make_iterator(const iterator_range<Iterator, TerminationPredicate>& range, identity_type_t<Iterator> pos)
    {
        discard(range);
        return pos;
    }

    template <typename Iterator>
    auto make_iterator(const delimited_iterator_range<Iterator>& range, identity_type_t<Iterator> pos)
    {
        discard(range);
        return pos;
    }

    template <typename Iterator>
    auto make_iterator(const counted_iterator_range<Iterator>& range, range_position_type<counted_iterator_range<Iterator>> pos)
    {
        return range.base_pos(pos);
    }

    template <typename Range, STDEXT_REQUIRES(is_range<Range>::value && !is_std_input_range_v<Range>)>
    auto make_generator(Range& range)
    {
        return range_generator<Range>(range);
    }

    template <typename Elem, typename Range, STDEXT_REQUIRES(std::is_convertible<Elem, range_value_type<Range>>::value)>
    auto make_consumer(Range& range)
    {
        using value_type = stdext::range_value_type<Range>;
        return [range = &range](value_type value)
        {
            if (range == nullptr || is_empty(*range))
                return false;
            head(*range) = value;
            drop_first(*range);
            return true;
        };
    }
}

#endif
