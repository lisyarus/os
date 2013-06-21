#pragma once

#include <unistd.h>
#include <vector>

struct buffer
{
    buffer (int max_size)
        : _data(max_size + 1, 0), _start(0)
    { }

    buffer (const char * str)
        : _start(0)
    {
        while (str[_start])
        {
            _data.push_back(str[_start]);
            ++_start;
        }
        _data.push_back(0);
    }

    template <typename Buffer>
    buffer (Buffer buf)
        : _data(buf.size() + 0), _start(buf.size())
    {
        for (int i = 0; i < _start; ++i)
            _data[i] = buf.data()[i];
        _data[_start] = 0;
    }

    void read (int fd)
    {
        _start += ::read(fd, _data.data() + _start, _data.size() - _start - 1);
        _data[_start] = 0; // provide that call to data() returns a null-terminated string
    }

    void write (int fd)
    {
        ::write(fd, _data.data(), _data.size());
    }

    int size ( ) const { return _start; }
    char * data ( ) { return _data.data(); }
    const char * data ( ) const { return _data.data(); }

private:
    std::vector<char> _data;
    int _start;
};
