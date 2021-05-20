#include "enumerate.h"
#include <catch.hpp>

TEST_CASE("basic vector")
{
	std::vector a{ 1, 2, 3 };
	auto i{ 0 };

	for (auto&& [index, item] : a | views::enumerate)
	{
		REQUIRE(index == i);
		REQUIRE(item == i + 1);
		++i;
	}
}

TEST_CASE("basic const vector")
{
	std::vector const a{ 1, 2, 3 };
	auto i{ 0 };

	for (auto&& [index, item] : a | views::enumerate)
	{
		REQUIRE(index == i);
		REQUIRE(item == i + 1);
		++i;
	}
}

TEST_CASE("transform")
{
	std::vector const a{ 1, 2, 3 };
	auto i{ 0 };

	for (auto&& [index, item] : a | std::views::transform([](auto i) { return i - 1; }) | views::enumerate)
	{
		REQUIRE(index == i);
		REQUIRE(item == i);
		++i;
	}
}
#include <iostream>
TEST_CASE("modify vector")
{
	std::vector a{ 1, 2, 3 };
	auto i{ 0 };

	for (auto&& [index, item] : a | views::enumerate)
	{
		REQUIRE(index == i);
		REQUIRE(item == i + 1);
		--item;
		++i;
	}

	i = 0;

	for (auto&& [index, item] : a | views::enumerate)
	{
		REQUIRE(index == i);
		REQUIRE(item == i);
		++i;
	}

	REQUIRE(a == std::vector{ 0, 1, 2 });
}

TEST_CASE("unsized view")
{
	for (auto&& [i, j] : std::views::iota(0) | views::enumerate | std::views::take(10))
	{
		REQUIRE(i == j);
	}
}