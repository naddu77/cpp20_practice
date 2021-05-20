#pragma once

#include "common.h"
#include <ranges>
#include <iterator>
#include <algorithm>
#include <functional>

template <std::ranges::forward_range V, std::predicate<std::ranges::range_reference_t<V>, std::ranges::range_reference_t<V>> F>
class chunk_by_view
{
public:
	chunk_by_view() = default;
	chunk_by_view(V v, F f)
		: base_{ std::move(v) }, func_{ std::move(f) }
	{

	}

	constexpr auto begin() requires (!simple_view<V>)
	{
		return iterator<false>{ std::ranges::begin(base_), this };
	}

	constexpr auto begin() const requires (std::ranges::range<V const>)
	{
		return iterator<true>{ std::ranges::begin(base_), this };
	}

	constexpr std::default_sentinel_t end() const
	{
		return std::default_sentinel;
	}

	auto& base()
	{
		return base_;
	}

	auto const& base() const
	{
		return base_;
	}

private:
	V base_;
	F func_;

	friend struct iterator;

	template <bool Const>
	struct iterator
	{
		template <typename T>
		using constify = std::conditional_t<Const, std::add_const_t<T>, T>;

		std::ranges::iterator_t<constify<V>> current;
		std::ranges::iterator_t<constify<V>> end_of_current_range;
		constify<chunk_by_view>* parent;

		using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
		using pointer_type = std::add_pointer_t<value_type>;
		using iterator_category = std::forward_iterator_tag;

		void find_end_of_current_range()
		{
			auto first_failed{ std::adjacent_find(current, std::end(parent->base_), std::not_fn(parent->func_)) };

			end_of_current_range = std::ranges::next(first_failed, 1, std::end(parent->base_));
		}

		iterator() = default;
		constexpr iterator(std::ranges::iterator_t<constify<V>> current, constify<chunk_by_view>* parent)
			: current{ std::move(current) }, parent(parent)
		{
			find_end_of_current_range();
		}

		constexpr iterator(iterator<!Const> i) requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<V const>>
			: current{ std::move(i.current) }, end_of_current_range{ std::move(i.end_of_current_range) }, parent(i.parent)
		{

		}

		constexpr auto operator*()
		{
			return std::ranges::subrange{ current, end_of_current_range };
		}

		constexpr iterator& operator++()
		{
			current = end_of_current_range;
			find_end_of_current_range();
			
			return *this;
		}

		constexpr iterator operator++(int)
		{
			auto tmp{ *this };

			++*this;

			return tmp;
		}

		friend constexpr bool operator==(iterator const& lhs, iterator const& rhs)
		{
			return lhs.current == rhs.current;
		}

		friend constexpr bool operator==(iterator const& lhs, std::default_sentinel_t)
		{
			return lhs.current == std::ranges::end(lhs.parent->base());
		}

		friend struct iterator<!Const>;
	};
};

template <typename R, typename F>
chunk_by_view(R&&, F f)->chunk_by_view<std::views::all_t<R>, F>;

namespace views
{
	namespace detail
	{
		template <typename F>
		struct chunk_by_closure
		{
			F f;

			template <std::ranges::forward_range R>
			friend constexpr auto operator|(R&& r, chunk_by_closure&& c)
			{
				return chunk_by_view(std::forward<R>(r), std::move(c.f));
			}
		};

		struct chunk_by_fn
		{
			template <typename F>
			constexpr auto operator()(F f) const
			{
				return chunk_by_closure<F>{ std::move(f) };
			}
		};
	}

	constexpr inline detail::chunk_by_fn chunk_by;
}

