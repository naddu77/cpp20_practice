#pragma once

#include <ranges>
#include <iterator>
#include <algorithm>

namespace detail
{
    template <typename C>
    concept reservable = std::ranges::input_range<C> && !std::ranges::view<C> && 
        requires (C c, std::ranges::range_size_t<C> s)
    {
        c.reserve(s);
    };

    template <typename>
    constexpr inline bool always_false = false;

    //R is a nested range that can be converted to the nested container C
    template <typename C, typename R>
    concept matroshkable = std::ranges::input_range<C> && std::ranges::input_range<R> &&
        std::ranges::input_range<std::ranges::range_value_t<C>> &&
        std::ranges::input_range<std::ranges::range_value_t<R>> &&
        !std::ranges::view<std::ranges::range_value_t<C>> &&
        std::indirectly_copyable<
        std::ranges::iterator_t<std::ranges::range_value_t<R>>,
        std::ranges::iterator_t<std::ranges::range_value_t<C>>
        >;

    template <std::ranges::input_range R>
    struct fake_input_iterator
    {
        using iterator_category = std::input_iterator_tag;
        using value_type = std::ranges::range_value_t<R>;
        using difference_type = std::ranges::range_difference_t<R >;
        using pointer = std::ranges::range_value_t<R>*;
        using reference = std::ranges::range_reference_t<R >;

        reference operator*();
        fake_input_iterator& operator++();
        fake_input_iterator operator++(int);
        fake_input_iterator() = default;
        friend bool operator==(fake_input_iterator a, fake_input_iterator b);
    };

    template <template <typename...> typename C, typename R, typename... Args>
    concept construct_container_from = requires (R r, Args... args)
    {
        C(std::forward<R>(r), std::forward<Args>(args)...);
    };

    template <template <class...> class C, class R, class... Args>
    concept construct_container_from_iterators = requires (Args... args, fake_input_iterator<R> i) 
    {
        C(i, i, std::forward<Args>(args)...);
    };

    template <template <typename...> typename C, std::ranges::input_range R, typename... Args>
    auto ctad_container() {

        if constexpr (construct_container_from<C, R, Args...>) 
        {
            return decltype(C(std::declval<R>(), std::declval<Args>()...)){};
        }
        else if constexpr (construct_container_from_iterators<C, R, Args...>) 
        {
            using iter = fake_input_iterator<R>;
            return decltype(C(std::declval<iter>(), std::declval<iter>(), std::declval<Args>()...)){};
        }
        else
        {
            static_assert(detail::always_false<R>, "C is not constructible from R");
        }
    }
}

template <std::ranges::input_range C, std::ranges::input_range R, typename... Args>
requires (!std::ranges::view<C>)
constexpr C to(R&& r, Args&&... args)
{
    //Construct from range
    if constexpr (std::constructible_from<C, R, Args...>) 
    {
        return C(std::forward<R>(r), std::forward<Args>(args)...);
    }
    //Construct and copy (potentially reserving memory)
    else if constexpr (std::constructible_from<C, Args...> && std::indirectly_copyable<std::ranges::iterator_t<R>, std::ranges::iterator_t<C>>) 
    {
        C c(std::forward<Args>(args)...);

        if constexpr (std::ranges::sized_range<R> && detail::reservable<C>) 
        {
            c.reserve(std::ranges::size(r));
        }

        std::ranges::copy(std::forward<R>(r), std::inserter(c, std::end(c)));

        return c;
    }
    //Nested case
    else if constexpr (detail::matroshkable<C, R>)
    {
        C c(std::forward<Args...>(args)...);

        auto v{ r | std::views::transform([](auto&& elem) {
            return to<std::ranges::range_value_t<C>>(elem);
            }) };

        std::ranges::copy(v, std::inserter(c, std::end(c)));

        return c;
    }
    //Construct from iterators
    else if constexpr (std::constructible_from<C, std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, Args...>)
    {
        return C(std::ranges::begin(r), std::ranges::end(r), std::forward<Args>(args)...);
    }
    else 
    {
        static_assert(detail::always_false<C>, "C is not constructible from R");
    }
}

template <template <typename...> typename C, std::ranges::input_range R, typename... Args, typename ContainerType = decltype(detail::ctad_container<C, R, Args...>())>
constexpr auto to(R&& r, Args&&... args) -> ContainerType
{
    return to<ContainerType>(std::forward<R>(r), std::forward<Args>(args)...);
}

namespace detail
{
    template <std::ranges::input_range C, typename... Args>
    struct closure_range
    {
        template <class... A>
        closure_range(A&&... as) 
            :args_(std::forward<A>(as)...)
        {

        }

        std::tuple<Args...> args_;
    };

    template <std::ranges::input_range R, std::ranges::input_range C, typename... Args>
    auto constexpr operator|(R&& r, closure_range<C, Args...>&& c) 
    {
        return std::apply([&r](auto&&... inner_args) {
            return to<C>(std::forward<R>(r));
            }, std::move(c.args_));
    }

    template <template <class...> class C, class... Args>
    struct closure_ctad
    {
        template <class... A>
        closure_ctad(A&&... as)
            : args_(std::forward<A>(as)...)
        {

        }

        std::tuple<Args...> args_;
    };

    template <std::ranges::input_range R, template <typename...> typename C, typename... Args>
    auto constexpr operator|(R&& r, closure_ctad<C, Args...>&& c)
    {
        return std::apply([&r](auto&&... inner_args) {
            return to<C>(std::forward<R>(r));
            }, std::move(c.args_));
    }
}

template <template <typename...> typename C, typename... Args>
constexpr auto to(Args&&... args) 
{
    return detail::closure_ctad<C, Args...>{ std::forward<Args>(args)... };
}

template <std::ranges::input_range C, typename... Args>
constexpr auto to(Args&&... args) 
{
    return detail::closure_range<C, Args...>{ std::forward<Args>(args)... };
}