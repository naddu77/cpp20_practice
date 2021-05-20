// generator.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "generator.h"
#include <iostream>
#include <ranges>

template <typename T>
generator<T> iota(T t = T{ 0 })
{
    while (true)
    {
        co_yield t;

        ++t;
    }
}

int main()
{
    for (auto n : iota<int>() | std::views::take(10))
        std::cout << n << ' ';

    std::cout << '\n';

    for (auto n : iota(10) | std::views::take(10))
        std::cout << n << ' ';

    std::cout << '\n';
}