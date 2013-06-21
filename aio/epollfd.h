#pragma once

#include "autofd.h"

#include <functional>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <sys/epoll.h>

#include <iostream>

struct epollfd
{
    using cont_type = std::function<void()>;

    epollfd (int max_size)
        : _max_size(max_size), _epollfd(epoll_create(max_size))
    {
        if (_epollfd == -1)
            throw("failed to create epoll");
    }

    void subscribe (int fd, int what, cont_type cont, cont_type cont_err)
    {
        auto cont_it = _find_cont(fd, what);
        auto event_it = _find_event(fd);

        if (cont_it.first == _conts.end() || cont_it.second == cont_it.first->second.end())
        {
            _conts[fd][what].cont = cont;
            _conts[fd][what].cont_err = cont_err;
        }
        else
        {
            cont_it.second->second.cont = cont;
            cont_it.second->second.cont_err = cont_err;
        }

        if (event_it == _events.end())
        {
            epoll_event event;
            event.events = what;
            event.data.fd = fd;
            _events.push_back(event);
            if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &_events.back()))
            {
                _events.pop_back();
                perror("hmmm");
                throw std::runtime_error("epoll add failed");
            }
        }
        else
        {
            event_it->events |= what;
            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &(*event_it)))
            {
                event_it->events ^= what;
                throw std::runtime_error("epoll modify failed");
            }
        }
    }

    void unsubscribe (int fd, int what)
    {
        auto cont_it = _find_cont(fd, what);
        if (cont_it.first != _conts.end())
        {
            auto event_it = _find_event(fd);
            event_it->events &= ~what;
            if (event_it->events == 0)
            {
                if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &(*event_it)))
                {
                    event_it->events |= what;
                    throw std::runtime_error("epoll delete failed");
                }
                _events.erase(event_it);
            }
            _conts.erase(cont_it.first);
            if (_conts.count(fd) == 0)
                _conts.erase(fd);
        }
    }

    void cycle ( )
    {
        if (_events.size() == 0) return;
        std::vector<epoll_event> events(_max_size);
        int count = epoll_wait(_epollfd, events.data(), _max_size, -1);
        if (count == -1)
            throw std::runtime_error("epoll wait failed");
        for (int e = 0; e < count; ++e)
        {
            epoll_event & event = events[e];
            if ((event.events & EPOLLERR) != 0)
            {
                for (auto c : _conts[event.data.fd])
                    c.second.cont_err();
                _conts.erase(event.data.fd);
            }
            else for (auto inner_it = _conts[event.data.fd].begin(); inner_it != _conts[event.data.fd].end();)
            {
                if ((inner_it->first & event.events) != 0)
                {
                    auto cont = inner_it->second.cont;
                    auto next_it = std::next(inner_it);
                    unsubscribe(event.data.fd, inner_it->first);
                    cont();
                    inner_it = next_it;
                }
                else
                    ++inner_it;
            }
        }
    }

    ~ epollfd ( )
    {
        close(_epollfd);
    }

private:
    int _max_size;
    int _epollfd;

    struct action
    {
        cont_type cont, cont_err;
    };

    std::map<int, std::map<int, action>> _conts;

    std::list<epoll_event> _events;

    std::pair<std::map<int, std::map<int, action>>::iterator,  std::map<int, action>::iterator> _find_cont (int fd, int what)
    {
        auto map_it = _conts.find(fd);
        std::map<int, action>::iterator inner_it;
        if (map_it != _conts.end())
            inner_it = map_it->second.find(what);
        return std::make_pair(map_it, inner_it);
    }

    std::list<epoll_event>::iterator _find_event (int fd)
    {
        return std::find_if(_events.begin(), _events.end(), [fd](epoll_event const & event)
        {
            return event.data.fd == fd;
        });
    }
};
