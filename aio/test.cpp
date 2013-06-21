#include "epollfd.h"

#include <string>
#include <iostream>

#include <fcntl.h>

int main (int argc, char ** argv)
{
    int fd = 0;
    std::ostream & out = std::cout;
    bool quit = false;

    epollfd e(10);
    std::function<void()> in_call = [fd, &out, &e, &in_call]()
    {
        out << "in\n";
        e.subscribe(fd, EPOLLIN, in_call, nullptr);
    };
    e.subscribe(fd, EPOLLIN, in_call, nullptr);
    e.subscribe(fd, EPOLLHUP, [&out, &quit](){out << "EPOLLHUP\n"; quit = true;}, nullptr);
    while (!quit)
    {
        e.cycle();
        std::string str;
        std::cin >> str;
    }
}
