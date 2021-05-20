#pragma once

#include "common.h"
#include <iterator>
#include <ranges>
#include <type_traits>
#include <xutility>

template <std::ranges::input_range T, std::integral U = std::size_t>
requires std::ranges::view<T>
class enumerate_view
	: public std::ranges::view_interface<enumerate_view<T>>
{
public:
	enumerate_view() = default;
	enumerate_view(T base, U pos = U{})
		: base_(std::move(base)), pos_(pos)
	{

	}

	constexpr auto begin()
		requires(!simple_view<T>)
	{
		return iterator<false>(std::ranges::begin(base_), pos_);
	}

	constexpr auto begin() const
		requires simple_view<T>
	{
		return iterator<true>(std::ranges::begin(base_), pos_);
	}

	constexpr auto end()
	{
		return sentinel<false>(std::ranges::end(base_));
	}

	constexpr auto end()
		requires std::ranges::common_range<T> && std::ranges::sized_range<T>
	{
		return iterator<false>{ std::ranges::end(base_), static_cast<std::ranges::range_difference_t<T>>(size()) };
	}

	constexpr auto end() const
		requires std::ranges::range<T const>
	{
		return sentinel<true>{ std::ranges::end(base_) };
	}

	constexpr auto end() const
		requires std::ranges::common_range<T const> && std::ranges::sized_range<T>
	{
		return iterator<true>{ std::ranges::end(base_), static_cast<std::ranges::range_difference_t<T>>(size()) };
	}

	constexpr auto size()
		requires std::ranges::sized_range<T>
	{
		return std::ranges::size(base_);
	}

	constexpr auto size() const
		requires std::ranges::sized_range<T const>
	{
		return std::ranges::size(base_);
	}

	constexpr auto base() const
		requires std::copy_constructible<T>
	{
		return base_;
	}

	constexpr T base()&&
	{
		return std::move(base_);
	}

private:
	T base_;
	U pos_;

	template <bool Const>
	class sentinel;
	template <bool Const>
	class iterator
	{
		using Base = std::conditional_t<Const, T const, T>;
		using count_type = decltype([] {
			if constexpr (std::ranges::sized_range<Base>)
				return std::ranges::range_size_t<Base>();
			else
				return std::make_unsigned_t<std::ranges::range_difference_t<Base>>();
			}());

		std::ranges::iterator_t<Base> current_{};
		count_type pos_ = 0;

	public:
		using iterator_category = typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category;
		using reference = std::pair<count_type, std::ranges::range_reference_t<Base>>;
		using value_type = std::pair<count_type, std::ranges::range_value_t<Base>>;
		using difference_type = std::ranges::range_difference_t<Base>;

		iterator() = default;
		constexpr explicit iterator(std::ranges::iterator_t<Base> current, difference_type pos)
			: current_{ std::move(current) }, pos_{ static_cast<count_type>(pos) }
		{

		}

		constexpr iterator(iterator<!Const> i)
			requires Const&& std::convertible_to<
			std::ranges::iterator_t<T>,
			std::ranges::iterator_t<Base>
			>
			: current_{ std::move(i.current_) }, pos_{ i.pos_ }
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
			return reference{ pos_, *current_ };
		}

		constexpr iterator& operator++()
		{
			++current_;
			++pos_;

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

			++pos_;
			++current_;

			return temp;
		}

		constexpr iterator& operator--()
			requires std::ranges::bidirectional_range<Base>
		{
			--pos_;
			--current_;

			return **this;
		}

		constexpr iterator operator--(int)
			requires std::ranges::bidirectional_range<Base>
		{
			auto temp{ *this };

			--pos_;
			--current_;

			return temp;
		}

		constexpr iterator& operator+=(difference_type x)
			requires std::ranges::random_access_range<Base>
		{
			pos_ += x;
			current_ += x;

			return *this;
		}

		constexpr iterator& operator-=(difference_type x)
			requires std::ranges::random_access_range<Base>
		{
			pos_ -= x;
			current_ -= x;

			return *this;
		}

		constexpr decltype(auto) operator[](difference_type n) const
			requires std::ranges::random_access_range<Base>
		{
			return reference{ static_cast<difference_type>(pos_ + n), *(current_ + n) };
		}

		friend constexpr bool operator==(iterator const& x, iterator const& y)
			requires std::equality_comparable<std::ranges::iterator_t<Base>>
		{
			return x.current_ == y.current_;
		}

		friend constexpr bool operator==(std::unreachable_sentinel_t const& x, iterator const& y)
		{
			return false;
		}

		friend constexpr auto operator<(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return x.current_ < y.current_;
		}

		friend constexpr auto operator>(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return x.current_ > y.current_;
		}

		friend constexpr auto operator<=(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return x.current_ <= y.current_;
		}

		friend constexpr auto operator>=(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return x.current_ >= y.current_;
		}

		friend constexpr auto operator<=>(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>&& std::three_way_comparable<std::ranges::iterator_t<Base>>
		{
			return x.current_ <=> y.current_;
		}

		friend constexpr iterator operator+(iterator const& x, difference_type y)
			requires std::ranges::random_access_range<Base>
		{
			return iterator{ x } += y;
		}

		friend constexpr iterator operator+(difference_type x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return y + x;
		}

		friend constexpr iterator operator-(iterator const& x, difference_type y)
			requires std::ranges::random_access_range<Base>
		{
			return iterator{ x } - y;
		}

		friend constexpr iterator operator-(iterator const& x, iterator const& y)
			requires std::ranges::random_access_range<Base>
		{
			return y - x;
		}

		template <bool>
		friend class sentinel;
		friend class iterator<!Const>;
	};

	template <bool Const>
	class sentinel
	{
		using Base = std::conditional_t<Const, T const, T>;
		std::ranges::sentinel_t<Base> end_{};

	public:
		sentinel() = default;
		constexpr explicit sentinel(std::ranges::sentinel_t<Base> end)
			:end_{ std::move(end) }
		{

		}

		constexpr sentinel(sentinel<!Const> other)
			requires Const&& std::convertible_to<std::ranges::sentinel_t<T>, std::ranges::sentinel_t<Base>>
			: end_{ std::move(other.end_) }
		{

		}

		constexpr std::ranges::sentinel_t<Base> base() const
		{
			return end_;
		}

		friend constexpr bool operator==(iterator<Const> const& x, sentinel const& y)
		{
			return x.base() == y.end_;
		}

		// Note: std::views(0) | views::enumerate 하면 컴파일 에러 나서 추가함
		friend constexpr bool operator==(iterator<!Const> const& x, sentinel const& y)
		{
			return x.base() == y.end_;
		}

		friend constexpr std::ranges::range_difference_t<Base> operator-(iterator<Const> const& x, sentinel const& y)
			requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
		{
			return x.base() - y.end_;
		}

		friend constexpr std::ranges::range_difference_t<Base> operator-(sentinel const& x, iterator<Const> const& y)
			requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
		{
			return x.end_ - y.base();
		}
	};
};

template <typename T>
enumerate_view(T&&)->enumerate_view<std::views::all_t<T>>;

template <typename T, std::integral U>
enumerate_view(T&&, U)->enumerate_view<std::views::all_t<T>, U>;

namespace views
{
	namespace detail
	{
		class enumerate_fn
		{
		public:
			template <std::ranges::viewable_range T>
			constexpr auto operator()(T&& t) const
			{
				return enumerate_view{ std::forward<T>(t) };
			}

			template <std::ranges::viewable_range T>
			friend constexpr auto operator|(T&& t, enumerate_fn)
			{
				return enumerate_view{ std::forward<T>(t) };
			}
		};
	}

	inline constexpr detail::enumerate_fn enumerate;
}