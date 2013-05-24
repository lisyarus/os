// c library

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

// c++ library

#include <iostream>

// main

int main ( )
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    addrinfo *result;
    {
        int r;
        if ((r = getaddrinfo(nullptr, "8822", &hints, &result)) != 0)
        {
            std::cerr << "getaddrinfo: " << gai_strerror(r) << std::endl;
            return 1;
        }
    }

    int sfd;
    addrinfo * ai;
    for (ai = result; ai != nullptr; ai = ai->ai_next)
    {
        sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sfd == -1) continue;
        if (connect(sfd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        close(sfd);
    }
    if (ai == nullptr) 
    {
        std::cerr << "Failed to bind" << std::endl;
        return 1;
    }

    char buffer[512];
    while (read(sfd, buffer, 512) != 0)
    {
        std::cout << buffer;
    }

    freeaddrinfo(result);

}
