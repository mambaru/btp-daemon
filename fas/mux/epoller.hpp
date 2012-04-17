//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MAX_EPOLLER_HPP
#define FAS_MAX_EPOLLER_HPP

#include <vector>
#include <set>
#include <sys/epoll.h>

#include <fas/mux/imux.hpp>
#include <fas/mux/types.hpp>
#include <fas/mux/imux_observer.hpp>
#include <fas/system/system.hpp>

#include <iostream>

namespace fas{ namespace mux{


class epoller 
   : public imux<int>
{
  
public:
  typedef imux<int> interface;
  typedef imux<int>::imux_observer imux_observer;
  typedef int desc_type;
  typedef std::set<descriptor_t> modify_set;

  epoller(int size = 1024): _events_size(size)
  {
    _events = new ::epoll_event[_events_size];
    _depoll = epoll_create(_events_size);
  }

  ~epoller()
  {
    close(_depoll);
    delete _events;
  }

  virtual void set_handlers(desc_type d, imux_observer * sh)
  {
    set_rhandler(d, sh);
    set_whandler(d, sh);
    set_uhandler(d, sh);
  }

  virtual imux_observer * set_rhandler(desc_type d, imux_observer * sh)
  {
    return _set_handler(d, sh, EPOLLIN);
  }

  virtual imux_observer * set_whandler(desc_type d, imux_observer * sh)
  {
    return _set_handler(d, sh, EPOLLOUT);
  }

  virtual imux_observer * set_uhandler(desc_type d, imux_observer * sh)
  {
    return _set_handler(d, sh, EPOLLPRI);
  }

  virtual imux_observer * reset_rhandler(desc_type d)
  {
    return _reset_handler(d, 0, EPOLLIN);
  }

  virtual imux_observer * reset_whandler(desc_type d)
  {
    return _reset_handler(d, 0, EPOLLOUT);
  }

  virtual imux_observer * reset_uhandler(desc_type d)
  {
    return _reset_handler(d, 0, EPOLLPRI);
  }

  virtual void reset_handlers(desc_type d)
  {
    reset_rhandler(d);
    reset_whandler(d);
    reset_uhandler(d);
  }

  virtual bool select(long timeout)
  {
    int nfds = epoll_wait(_depoll, _events, _events_size, timeout);

    if (nfds<0)
    {
      int err = errno;
      if ( EINTR!=err)
        throw std::runtime_error( std::string("epoller::select calling epoll_wait:  ") + strerror(errno) );
      return false;
    }

    for (int i=0;i<nfds;i++)
    {
      desc_type d = _events[i].data.fd;
      int flags_set = 0;
      /// во время прохождения цикла соединение может быть сброшено предедущими обработчиками
      if ( d < static_cast<desc_type>(_handlers.size()) )
        flags_set = _handlers[d].first.events;

      if ( _events[i].events & (EPOLLERR /*| EPOLLERR*/ | EPOLLHUP) )
      {
        _handlers[d].second->ready_error(d);
      }
      else
      {
        if (_events[i].events & EPOLLIN & flags_set)
          _handlers[d].second->ready_read(d);

        if (_events[i].events & EPOLLOUT & flags_set)
          _handlers[d].second->ready_write(d);

        if (_events[i].events & EPOLLPRI & flags_set )
          _handlers[d].second->ready_urgent(d);
      }

    }
    return true;
  }

protected:

  imux_observer *_set_handler( desc_type d, imux_observer * sh, int event_falag )
  {
    imux_observer* oldHandler = 0;
    if (_handlers.size() <= (size_t)d)
      _handlers.resize( d + 1);

    if (false == _handlers[d].ready)
    {
      _handlers[d].ready = true;
      _handlers[d].first.events = event_falag;
      _handlers[d].first.data.fd = d;
      _handlers[d].second = sh;

      if ( epoll_ctl(_depoll, EPOLL_CTL_ADD, d, &_handlers[d].first) < 0)
        throw std::runtime_error( std::string("epoller::_set_handle1: ") + strerror(errno) );
    }
    else
    {
      oldHandler = _handlers[d].second;
      _handlers[d].second = sh;
      int mode = _handlers[d].first.events !=0 ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

      _handlers[d].first.events |= event_falag ;

      if ( epoll_ctl(_depoll, mode, d, &_handlers[d].first) < 0)
      {
        int e = _handlers[d].first.events;
        e &=~ event_falag ;
        if (e & EPOLLIN) 
          throw std::runtime_error( std::string("epoller::_set_handle, error flag EPOLLIN, ") + strerror(errno) );
        if (e & EPOLLOUT)
          throw std::runtime_error( std::string( "epoller::_set_handle, error flag EPOLLOUT, ") + strerror(errno) );
        if (e & EPOLLPRI) 
          throw std::runtime_error( std::string("epoller::_set_handle, error flag EPOLLPRI, ") + strerror(errno) );
        if (e & EPOLLERR)
          throw std::runtime_error( std::string("epoller::_set_handle, error flag EPOLLERR, ") + strerror(errno) );

        throw std::logic_error( std::string("epoller::_set_handle: ") + strerror(errno) );
      }
    }
    return oldHandler;
  }

  imux_observer *_reset_handler(desc_type d, imux_observer * sh, int event_falag)
  {
    imux_observer* oldHandler = 0;
    if ((size_t)d >=_handlers.size() || _handlers[d].ready == false )
      throw std::runtime_error("epoller::_reset_handle: descriptor not found");
    else if ( (_handlers[d].first.events & (EPOLLIN | EPOLLOUT | EPOLLPRI ))!=0)
    {
      oldHandler = _handlers[d].second;
      _handlers[d].first.events &= ~event_falag;
      _handlers[d].first.data.fd = d;

      if ( (_handlers[d].first.events & (EPOLLIN | EPOLLOUT | EPOLLPRI ))==0)
      {
         if ( epoll_ctl(_depoll, EPOLL_CTL_DEL, d, &_handlers[d].first) < 0)
           throw std::runtime_error( std::string("epoller::_reset_handle: ") + strerror(errno) );
       _handlers[d].ready = false;
      }
      else
        if ( epoll_ctl(_depoll, EPOLL_CTL_MOD, d, &_handlers[d].first) < 0)
          throw std::runtime_error( std::string("epoller::_reset_handle: ") + strerror(errno) );
    }
    return oldHandler;
  }

private:

  desc_type _depoll;

  struct EventHandler
  {
    epoll_event first;
    imux_observer* second;
    bool ready;
  };

  typedef std::vector<EventHandler> Handlers;

  Handlers _handlers;

  epoll_event *_events;
  size_t _events_size;
};

}}

#endif
