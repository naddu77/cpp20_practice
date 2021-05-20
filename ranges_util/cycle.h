#pragma once

#include <iterator>
#include <ranges>
#include <type_traits>

template <std::ranges::forward_range T>
requires std::ranges::view<T>
class cycle_view
    : public std::ranges::view_interface<cycle_view<T>> 
{
    T base_;

    class sentinel {};

    template <bool Const>
    class iterator 
    {
        using Base = std::conditional_t<Const, const T, T>;

        std::ranges::iterator_t<Base> current_{};
        Base* base_ = nullptr;

    public:
        using iterator_category = typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category;
        using reference = std::ranges::range_reference_t<Base>;
        using value_type = std::ranges::range_value_t<Base>;
        using difference_type = std::ranges::range_difference_t<Base>;

        iterator() = default;
        constexpr explicit iterator(std::ranges::iterator_t<Base> current, Base* base)
            : current_{ std::move(current) }, base_{ base }
        {

        }

        constexpr iterator(iterator<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<T>,
            std::ranges::iterator_t<Base>
        >
            : current_{ std::move(i.current_) } 
        {
        
        }

        constexpr std::ranges::iterator_t<Base> base() const&
            requires std::copyable<std::ranges::iterator_t<Base>>
        {
            return current_;
        }

        constexpr std::ranges::iterator_t<Base> base()&&
        {
            return std::move(current_);
        }

        constexpr decltype(auto) operator*() const 
        {
            return *current_;
        }

        constexpr iterator& operator++() 
        {
            ++current_;

            if (current_ == std::ranges::end(*base_))
            {
                current_ = std::ranges::begin(*base_);
            }

            return *this;
        }
        constexpr void operator++(int) 
            requires(!std::ranges::forward_range<Base>) 
        {
            (void)operator++();
        }
        constexpr iterator operator++(int) 
            requires std::ranges::forward_range<Base> 
        {
            auto temp{ *this };

            this->operator++();

            return temp;
        }

        constexpr iterator& operator--() 
            requires std::ranges::bidirectional_range<Base>
        {
            if (current_ == std::ranges::begin(*base_))
            {
                current_ = std::ranges::end(*base_);
            }

            --current_;

            return *this;
        }

        constexpr iterator operator--(int)
            requires std::ranges::bidirectional_range<Base> 
        {
            auto const temp{ *this };

            this->operator--();

            return temp;
        }

        constexpr iterator& operator+=(difference_type x) 
            requires std::ranges::random_access_range<Base> 
        {
            auto const begin{ std::ranges::begin(*base_) };
            auto const end{ std::ranges::end(*base_) };
            auto const size{ end - begin };
            auto const distance_from_begin{ current_ - begin };
            auto const offset{ (distance_from_begin + x) % size };

            current_ = begin + static_cast<difference_type>(offset < 0 ? offset + size : offset);

            return *this;
        }

        constexpr iterator& operator-=(difference_type x)
            requires std::ranges::random_access_range<Base> 
        {
            this->operator+=(-x);

            return *this;
        }
        constexpr decltype(auto) operator[](difference_type n) const
            requires std::ranges::random_access_range<Base>
        {
            return *((*this) + n);
        }

        friend constexpr bool operator==(const iterator& x, const iterator& y)
            requires std::equality_comparable<std::ranges::iterator_t<Base>>
        {
            return x.current_ == y.current_;
        }

        friend constexpr bool operator==(const iterator<Const>&, const sentinel&)
        {
            return false;
        }

        friend constexpr auto operator<(const iterator& x, const iterator& y)
            requires std::ranges::random_access_range<Base>
        {
            return x.current_ < y.current_;
        }

        friend constexpr auto operator>(const iterator& x, const iterator& y)
            requires std::ranges::random_access_range<Base>
        {
            return x.current_ > y.current_;
        }

        friend constexpr auto operator<=(const iterator& x, const iterator& y)
            requires std::ranges::random_access_range<Base> 
        {
            return x.current_ <= y.current_;
        }

        friend constexpr auto operator>=(const iterator& x, const iterator& y)
            requires std::ranges::random_access_range<Base>
        {
            return x.current_ >= y.current_;
        }

        friend constexpr auto operator<=>(const iterator& x, const iterator& y)
            requires std::ranges::random_access_range<Base> && std::three_way_comparable<std::ranges::iterator_t<Base>>
        {
            return x.current_ <=> y.current_;
        }

        friend constexpr iterator operator+(const iterator& x, difference_type y)
            requires std::ranges::random_access_range<Base>
        {
            return iterator{ x } += y;
        }

        friend constexpr iterator operator+( difference_type x, const iterator& y)
            requires std::ranges::random_access_range<Base>
        {
            return y + x;
        }

        friend constexpr iterator operator-(const iterator& x, difference_type y)
            requires std::ranges::random_access_range<Base>
        {
            return iterator{ x } - y;
        }

        friend class iterator<!Const>;
    };

public:
    cycle_view() = default;
    cycle_view(T base)
        : base_(std::move(base))
    {

    }

    constexpr auto begin() 
    {
        return iterator<false>(std::ranges::begin(base_), std::addressof(base_));
    }

    constexpr auto begin() const 
    {
        return iterator<true>(std::ranges::begin(base_), std::addressof(base_));
    }

    constexpr auto end()
    { 
        return sentinel{}; 
    }

    constexpr T base() const& requires std::copy_constructible<T> 
    {
        return base_;
    }

    constexpr T base()&& 
    { 
        return std::move(base_); 
    }
};

template <class R>
cycle_view(R&&)->cycle_view<std::views::all_t<R>>;

namespace views
{
    namespace detail
    {
        class cycle_fn
        {
        public:
            template <std::ranges::viewable_range T>
            constexpr auto operator()(T&& t) const
            {
                return cycle_view{ std::forward<T>(t) };
            }

            template <std::ranges::viewable_range T>
            friend constexpr auto operator|(T&& t, cycle_fn) 
            {
                return cycle_view{ std::forward<T>(t) };
            }
        };
    }

    inline constexpr detail::cycle_fn cycle;
}
