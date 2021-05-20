// ranges_util.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define USE_CATCH2

#ifdef USE_CATCH2
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#else
#include "enumerate.h"
#include "cycle.h"
#include "cartesian_product.h"
#include "to.h"
#include "chunk_by.h"
#include "chunk_by_key.h"
#include "stride.h"
#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <list>
#include <map>
#include <deque>
#include <forward_list>
#include <numeric>

int main()
{
    std::vector<std::string> v{ "A", "B", "C" };

    for (auto&& [index, item] : views::enumerate(v))
    {
        std::cout << index << ". " << item << '\n';
    }

    for (auto&& [index, item] : v | views::enumerate)
    {
        std::cout << index << ". " << item << '\n';
    }

    for (auto&& item : views::cycle(v) | std::views::take(10))
    {
        std::cout << item << ' ';
    }

    std::cout << '\n';

    for (auto&& item : v | views::cycle | std::views::take(10))
    {
        std::cout << item << ' ';
    }

    std::cout << '\n';

    for (auto&& [a, b, c] : views::cartesian_product(v, v, v))
        std::cout << a << ' ' << b << ' ' << c << '\n';

    std::list<int> l(50);
    std::map<int, int> m;

    std::iota(std::begin(l), std::end(l), 1);

    // copy a list to a vector of the same type
    auto a = to<std::vector<int>>(l);

    //Specify an allocator
    //auto b = to<std::vector<int, Alloc>>(l, alloc);

    // copy a list to a vector of the same type, deducing value_type
    auto c = to<std::vector>(l);

    // copy to a container of types ConvertibleTo
    auto d = to<std::vector<long>>(l);

    //Supports converting associative container to sequence containers
    auto f = to<std::vector<std::pair<int, int>>>(m); //std::vector<std::pair<int,int>>

    //Supports converting sequence containers to associative ones
    auto g = to<std::map<int, int>>(f); //std::map<int,int>

    ////Pipe syntax
    //auto h = l | std::views::take(42) | to<std::vector>();

    ////Pipe syntax with allocator
    //auto h = l | std::views::take(42) | to<std::vector>(alloc);

    //The pipe syntax also support specifying the type and conversions
    //auto i = l | std::views::take(42) | to<std::vector<long>>();

    // Note: STL 버그로 아직 실행 안됨
    //auto i = l | std::views::take(42);

    //for (auto ee : i)
    //    std::cout << ee << std::endl;

    //// Nested ranges
    //std::list<std::forward_list<int>> lst = { {0, 1, 2, 3}, {4, 5, 6, 7} };
    //auto vec1 = to<std::vector<std::vector<int>>>(lst);
    //auto vec2 = to<std::vector<std::deque<double>>>(lst);

    //{
    //    std::list<int> a{ 0, 1, 2 };
    //    auto b = a | views::cycle | std::views::take(10) | std::views::transform([](auto x) { return x * 2; }) | to<std::vector>();
    //    std::vector<int> res{ 0, 2, 4, 0, 2, 4, 0, 2, 4, 0 };
    //}
    
    // chunk_by
    {
        struct Cat
        {
            std::string name;
            int age;
        };

        std::vector<Cat> cats {
            {"potato", 12},
            {"bard", 12},
            {"soft boy", 9},
            {"vincent van catto", 12},
            {"oatmeal", 12},
        };

        for (auto&& group : cats | views::chunk_by([](auto&& left, auto&& right) { return left.age == right.age; }))
        {
            std::cout << "Group\n";
            std::cout << "{\n";
            
            for (auto&& [index, cat] : group | views::enumerate)
            {
                std::cout << "\t{ index: " << index << ", name: " << cat.name << ", age: " << cat.age << " }\n";
            }

            std::cout << "}\n";
        }
    }

    // chunk_by_key
    {
        struct Cat
        {
            std::string name;
            int age;
        };

        std::vector<Cat> cats{
            {"potato", 12},
            {"bard", 12},
            {"soft boy", 9},
            {"vincent van catto", 12},
            {"oatmeal", 12},
        };

        for (auto&& [i, group] : cats | views::chunk_by_key([](auto&& cat) { return cat.age; }))
        {
            std::cout << "Group(Key: " << i << ")\n";
            std::cout << "{\n";

            for (auto&& [index, cat] : group | views::enumerate)
            {
                std::cout << "\t{ index: " << index << ", name: " << cat.name << ", age: " << cat.age << " }\n";
            }

            std::cout << "}\n";
        }
    }

    // stride_view
    {
        std::vector v{ 0, 1, 2, 3, 4, 5, 6, 7};

        for (auto&& e : v | views::stride(3))
        {
            std::cout << e << ' ';
        }
    }
}

#endif