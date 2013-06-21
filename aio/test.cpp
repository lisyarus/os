#include "async_ops.h"
#include "buffer.h"
#include "apoll.h"

#include <string>
#include <iostream>
#include <thread>

#include <fcntl.h>

int main (int argc, char ** argv)
{
    epollfd efd(10);
    buffer buf("Hello, world!\n");

    apoll ap(efd);
    ap.awrite(1, buf, [](){}, nullptr);

    efd.cycle();
}
