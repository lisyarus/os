#include <stdexcept>

#include <unistd.h>

struct autofd
{
    autofd (int fd)
        : _fd(fd)
    {
        if (_fd < 0)
            throw std::runtime_error("file descriptor below zero");
    }

    autofd (autofd const &) = delete;

    autofd (autofd && other)
        : _fd(other._fd)
    {
        other._fd = -1;
    }

    autofd & operator = (autofd const &) = delete;

    autofd & operator = (autofd && other)
    {
        _fd = other._fd;
        other._fd = -1;
        return *this;
    }

    operator bool ( ) const
    {
        return _fd >= 0;
    }

    int operator * ( ) const
    {
        return _fd;
    }

    ~ autofd ( )
    {
        close(_fd);
    }

private:
    int _fd;
};
