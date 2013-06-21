#pragma once

#include <functional>

template <typename F1, typename F2>
std::function<void()> compose2 (F1 f1, F2 f2)
{
    return [f1, f2]()
    {
        f1();
        f2();
    };
}

std::function<void()> compose ( )
{
    return [] {};
}

template <typename First, typename ... Other>
std::function<void()> compose (First first, Other ... other)
{
    return compose2(first, compose(other...));
}

