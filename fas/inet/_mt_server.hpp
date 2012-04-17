//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_MT_SERVER_HPP
#define FAS_INET_MT_SERVER_HPP


#include <fas/system/system.hpp>
#include <fas/system/inet.hpp>
#include <fas/system/thread.hpp>
#include <fas/inet/server.hpp>
#include <fas/mux/best_mux.hpp>
#include <pthread.h>



#include <vector>
#include <string>
#include <iostream>

namespace fas { namespace inet {

namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace am = ::fas::mux;

struct _on_mt_server_thread_exception_{};
struct _on_mt_server_thread_select_{};

struct on_mt_server_thread_exception_stub
        : aa::advice< ap::tag<_on_mt_server_thread_exception_>, aa::ad_stub<> > {};

struct on_mt_server_thread_select_stub
        : aa::advice< ap::tag<_on_mt_server_thread_select_>, aa::ad_stub<> > {};

/*
struct on_mt_server_start{};
struct on_mt_server_ready_read{};
struct on_mt_server_ready_write{};
struct on_mt_server_ready_urgent{};
struct on_mt_server_ready_error{};
struct on_mt_server_start{};
struct on_mt_server_accept{};
struct on_mt_server_accept_error{};
struct on_mt_server_stop{};


struct on_server_ready_read_stub: aa::advice< ap::tag<on_server_ready_read>, af::ad_stub<> > {};
struct on_server_ready_write_stub: aa::advice< ap::tag<on_server_ready_write>, af::ad_stub<> > {};
struct on_server_ready_urgent_stub: aa::advice< ap::tag<on_server_ready_urgent>, af::ad_stub<> > {};
struct on_server_ready_error_stub: aa::advice< ap::tag<on_server_ready_error>, af::ad_stub<> > {};
*/

struct mt_server_aspect
  : aa::aspect<
      ap::type_list_n<
        on_mt_server_thread_select_stub,
        on_mt_server_thread_exception_stub
      >::type
    >
{};

template< typename S,
          typename A,
          typename SA,
          net_protocol_t NP,
          transport_protocol_t TP>
struct make_child_server_super_class
{
  typedef typename S::template rebind<
      A,
      typename aa::aspect_merge<mt_server_aspect, SA>::type,
      NP, TP
    >::type type;
};

struct default_server: server<aa::aspect<>, mt_server_aspect, IPv4, TCP>{};

template< typename A = aa::aspect<>,
          typename AS = mt_server_aspect,
          net_protocol_t NP = IPv4,
          transport_protocol_t TP = TCP,
          typename S = default_server
        >
class child_server
: public make_child_server_super_class< S, A, AS, NP, TP >::type
{
public:
  typedef child_server<A, AS, NP, TP, S> self;
  typedef typename make_child_server_super_class< S, A, AS, NP, TP >::type super;
  typedef typename super::desc_type desc_type;
  typedef typename super::mutex_type mutex_type;

  void assign( desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _drp = d;
    if ( super::get_mux()!=0)
      super::get_mux()->set_rhandler(d, this);
  }

  virtual void ready_read(desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    if ( d != _drp )
    {
      super::_ready_read(*this, d);
    }
    else
    {
      desc_type cli;
      ssize_t s = fas::system::read(_drp, (char*)&cli, sizeof(cli));
      if ( s == sizeof(cli) )
      {
        super::_accept(*this, cli, address_t() );
      }
    }
  }

private:
  desc_type _drp;
};

template<typename A = aa::aspect<>,
         typename AS = server_aspect,
         net_protocol_t NP = IPv4,
         transport_protocol_t TP = TCP,
         typename M = am::best_mux,
         typename S = default_server
 >
class mt_server
  : public child_server<A, AS, NP, TP, S>
{
  
public:
  typedef mt_server<A, AS, NP, TP, M,  S> self;
  typedef child_server<A, AS, NP, TP, S> super;
  typedef child_server<A, AS, NP, TP, S> child;
  typedef typename super::desc_type desc_type;
  typedef M mux;
private:
  struct child_info
  {
    pthread_t tid;
    desc_type rd;
    desc_type wd;
    child* chd; 
  };
  typedef std::vector<child_info> threads;
  threads _threads;
  size_t _max_threads;
  size_t _count;

  
public:
  typedef typename threads::iterator iterator;
  typedef typename super::mutex_type mutex_type;

  mt_server():_max_threads(1), _count(0){}

  void set_max_threads(size_t n) 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    _max_threads = n; 
  }

  size_t get_max_threads(size_t n) const 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return _max_threads; 
  }

  iterator begin() { return _threads.begin(); }
  iterator end() { return _threads.end(); }

  size_t size() const 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return _threads.size();
  }

  void start(const std::string& addr, unsigned short port)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_start(*this, addr, port);
  }

  void start(unsigned short port)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::start( std::string(""), port);
  }

  virtual void ready_read(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_ready_read(*this, d);
  }

  virtual void ready_write(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_ready_write(*this, d);
  }

  virtual void ready_urgent(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_ready_urgent(*this, d);
  }

  virtual void ready_error(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_ready_error(*this, d);
  }

private:

  template<typename T>
  void _start(T& t, const std::string& addr, unsigned short port)
  {
    _threads.resize(_max_threads);

    for (volatile size_t i = 0; i < _max_threads; ++i)
    {
      int fds[2];
      int pr = pipe(fds);

      if (pr == 0)
      {
        ::fas::system::inet::nonblock(fds[1]);
        _threads[i].rd = fds[0];
        _threads[i].wd = fds[1];
        _threads[i].chd = new child(*this);
        int err = pthread_create( &(_threads[i].tid), 0, _server_thread, (void*)&(_threads[i]) );
        if (err == EAGAIN)
          throw std::runtime_error("mt_server::_start: EAGAIN");
        else if (err == EINVAL)
          throw std::runtime_error("mt_server::_start: EINVAL");
        else if (err == EPERM)
          throw std::runtime_error("mt_server::_start: EPERM");
      }
    }
    super::_start(*this, addr, port);
    aa::get_advice<_on_server_start_>(t)(t);
  }


  template<typename T>
  void _ready_read(T& t, desc_type d)
  {
    if (_count >= _max_threads ) _count = 0;

    address_t a;
    desc_type cli = ::fas::system::inet::accept(d, a);
    if (cli > 0)
    {
      aa::get_advice<_on_server_accept_>(t)(t, d, a);

      if ( sizeof(int) != ::fas::system::write( _threads[_count++].wd, (char*)&cli, sizeof(cli) ) )
        ::fas::system::inet::close(cli);
    }
  }

  template<typename T>
  void _ready_write(T& t, desc_type d)
  {
    aa::get_advice<_on_server_ready_write_>(t)(t, d);
  }

  template<typename T>
  void _ready_urgent(T& t, desc_type d)
  {
    aa::get_advice<_on_server_ready_urgent_>(t)(t, d);
  }

  template<typename T>
  void _ready_error(T& t, desc_type d)
  {
    self::_stop(t);
    aa::get_advice<_on_server_ready_error_>(t)(t, d);
  }

  template<typename T>
  void _stop(T& t)
  {
    typename threads::iterator itr = _threads.begin();

    for ( ; itr!=_threads.end(); ++itr )
    {
      close(itr->rd);
      close(itr->wd);
    }

    for ( ; itr!=_threads.end(); ++itr )
      pthread_join(itr->tid, 0);

    aa::get_advice<_on_server_stop_>(t)(t);
  }


  static void* _server_thread(void* param)
  {
    mux m;
    child_info *ci = reinterpret_cast<child_info*>(param);
    ci->chd->set_mux(&m);
    ci->chd->assign(ci->rd);
    ci->chd->start();
    for (;;) 
    {
      try
      {
         m.select(-1);
      }
      catch(std::exception& e)
      {
        aa::get_advice<_on_mt_server_thread_exception_>(*ci->chd)(*ci->chd, e);
        // std::cerr<<"loop: "<<e.what()<<std::endl;
      }
      catch(...)
      {
        aa::get_advice<_on_mt_server_thread_exception_>(*ci->chd)(*ci->chd);
        // std::cerr<<"loop: "<<"unknown error"<<std::endl;
      }
    }
    return 0;
  }
};

}}
#endif
