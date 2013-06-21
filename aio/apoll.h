#pragma once

#include "async_ops.h"
#include <list>

struct apoll
{
    apoll (epollfd & efd)
        : _efd(efd)
    { }

    template <typename Buffer>
    void aread (int fd, Buffer & buf, std::function<void()> cont, std::function<void()> cont_err)
    {
        auto it = _add_back();
        auto deleter = _make_deleter(it);
        *it = async_read(_efd, fd, buf, compose(deleter, cont), compose(deleter, cont_err));
    }

    template <typename Buffer>
    void awrite (int fd, Buffer & buf, std::function<void()> cont, std::function<void()> cont_err)
    {
        auto it = _add_back();
        auto deleter = _make_deleter(it);
        *it = async_write(_efd, fd, buf, compose(deleter, cont), compose(deleter, cont_err));
    }

    void aacept (int sockfd, sockaddr * addr, socklen_t * addrlen, std::function<void(int)> cont, std::function<void()> cont_err)
    {
        auto it = _add_back();
        auto deleter = _make_deleter(it);
        *it = async_accept(_efd, sockfd, addr, addrlen, [cont, deleter](int fd)
        {
            deleter();
            cont(fd);
        },
        [cont_err, deleter]()
        {
            deleter();
            cont_err();
        });
    }

private:
    std::list<async_op>::iterator _add_back ( )
    {
        _ops.emplace_back();
        return std::prev(_ops.end());
    }

    std::function<void()> _make_deleter (std::list<async_op>::iterator it)
    {
        return [this, it]()
        {
            this->_ops.erase(it);
        };
    }

    epollfd & _efd;
    std::list<async_op> _ops;
};
