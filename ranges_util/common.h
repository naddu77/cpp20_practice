#pragma once

#include <iterator>
#include <ranges>

namespace detail
{
	template <typename... Ts>
	constexpr auto common_iterator_category()
	{
		if constexpr ((std::ranges::random_access_range<Ts> && ...))
			return std::random_access_iterator_tag{};
		else if constexpr ((std::ranges::bidirectional_range<Ts> && ...))
			return std::bidirectional_iterator_tag{};
		else if constexpr ((std::ranges::forward_range<Ts> && ...))
			return std::forward_iterator_tag{};
		else if constexpr ((std::ranges::input_range<Ts> && ...))
			return std::input_iterator_tag{};
		else if constexpr ((std::ranges::output_range<Ts> && ...))
			return std::output_iterator_tag{};
		else
			static_assert(false, "There are types that are not iterators");
	}
}

template <typename... Ts>
using common_iterator_category = decltype(detail::common_iterator_category<Ts...>());

template <typename R>
concept simple_view = std::ranges::view<R> && std::ranges::range<R const> &&
	std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<R const>> &&
	std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<R const>>;

struct begin_tag_t{};
constexpr inline begin_tag_t begin_tag;
struct end_tag_t{};
constexpr inline end_tag_t end_tag;