#include "epollfd.h"

#include <string>
#include <iostream>

int main ( )
{
    std::ostream & out = std::cout;

    epollfd e(10);
    std::function<void()> in_call = [&out, &e, &in_call]()
    {
        out << "in\n";
        e.subscribe(0, EPOLLIN, in_call, nullptr);
    };
    e.subscribe(0, EPOLLIN, in_call, nullptr);
    while (1)
    {
        e.cycle();
        std::string str;
        std::cin >> str;
    }
}
