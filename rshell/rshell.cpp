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
        if (bind(sfd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        close(sfd);
    }
    if (ai == nullptr) 
    {
        std::cerr << "Failed to bind" << std::endl;
        return 1;
    }

    freeaddrinfo(result);

    {
        int opt = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, 4) != 0)
        {
            std::cerr << "Failed to set socket option" << std::endl;
            return 0;
        }
    }

    if (listen(sfd, 5) != 0)
    {
        std::cerr << "Failed to listen" << std::endl;
        return 0;
    }
   
    while (true)
    {
        int sd;
        if ((sd = accept(sfd, nullptr, nullptr)) == -1)
            continue;

        if (fork())
        {
            // parent
            close(sd);
        }
        else
        {
            // child
            close(sfd);
            dup2(sd, 0);
            dup2(sd, 1);
            dup2(sd, 2);
            std::cout << "Hello, %username%!" << std::endl;
            return 0;
        }
    }
}