#pragma once

#include "epollfd.h"
#include "func.h"

#include <sys/types.h>
#include <sys/socket.h>

struct async_op
{
    async_op ( )
        : _finished(new bool(true))
    { }

    async_op (epollfd & efd, int fd, int what)
        : _finished(new bool(false)), _pefd(&efd), _fd(fd), _what(what)
    { }

    async_op (async_op const &) = delete;

    async_op (async_op && other)
        : _finished(other._finished), _pefd(other._pefd),  _fd(other._fd), _what(other._what)
    {
        other._finished = nullptr;
    }

    async_op & operator = (async_op const &) = delete;

    async_op & operator = (async_op && other)
    {
        _unsubscribe();
        delete _finished;
        _pefd = other._pefd;
        _finished = other._finished;
        other._finished = nullptr;
        _fd = other._fd;
        _what = other._what;
        return *this;
    }

    ~ async_op ( )
    {
        _unsubscribe();
        delete _finished;
    }

    bool finished ( ) const { return (_finished == 0) || *_finished; }
    int fd ( ) const { return _fd; }
    int what ( ) const { return _what; }

protected:
    void _subscribe (std::function<void()> cont, std::function<void()> cont_err)
    {
        _pefd->subscribe(_fd, _what, cont, cont_err);
    }

    std::function<void()> _finisher ( )
    {
        return [this]()
        {
            *(this->_finished) = true;
        };
    }

private:
    bool * _finished;
    epollfd * _pefd;
    int _fd, _what;

    void _unsubscribe ( )
    {
        if (_finished)
            if (!*_finished)
                _pefd->unsubscribe(_fd, _what);
    }
};

struct async_read
    : async_op
{
    async_read ( ) = default;

    template <typename Buffer>
    async_read (epollfd & efd, int fd, Buffer & buf, std::function<void()> cont, std::function<void()> cont_err)
        : async_op(efd, fd, EPOLLIN)
    {
        _subscribe(compose(_finisher(), [fd, &buf, cont]()
        {
            buf.read(fd);
            cont();
        }),
        compose(_finisher(), [cont_err]()
        {
            cont_err();
        }));
    }
};

struct async_write
    : async_op
{
    async_write ( ) = default;

    template <typename Buffer>
    async_write (epollfd & efd, int fd, Buffer & buf, std::function<void()> cont, std::function<void()> cont_err)
        : async_op(efd, fd, EPOLLOUT)
    {
        _subscribe(compose(_finisher(), [fd, &buf, cont]()
        {
            buf.write(fd);
            cont();
        }),
        compose(_finisher(), [cont_err]()
        {
            cont_err();
        }));
    }
};

struct async_accept
    : async_op
{
    async_accept ( ) = default;

    async_accept (epollfd & efd, int sockfd, sockaddr * addr, socklen_t * addrlen, std::function<void(int)> cont, std::function<void()> cont_err)
        : async_op(efd, sockfd, EPOLLIN)
    {
        _subscribe(compose(_finisher(), [sockfd, addr, addrlen, cont, cont_err]()
        {
            int fd = accept(sockfd, addr, addrlen);
            if (fd < 0)
                cont_err();
            else
                cont(fd);
        }),
        compose(_finisher(), [cont_err]()
        {
            cont_err();
        }));
    }
};
