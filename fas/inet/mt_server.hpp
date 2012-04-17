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
#include <fas/adv/ad_mutex.hpp>
// #include <pthread.h>

#ifndef HAVE_BOOST_THREAD_HPP
#error HAVE_BOOST_THREAD_HPP not defined. boost/thread library needed for include this file
#endif



#include <vector>
#include <string>
#include <iostream>

namespace aa = ::fas::aop;
namespace ad = ::fas::adv;
namespace ap = ::fas::pattern;
namespace am = ::fas::mux;
namespace ast = ::fas::system::thread;

namespace fas { namespace inet {

struct _on_mt_server_thread_exception_{};
struct _on_mt_server_thread_select_{};

struct on_mt_server_thread_exception_stub
        : aa::advice< aa::tag<_on_mt_server_thread_exception_>, ad::ad_stub<> > {};

struct on_mt_server_thread_select_stub
        : aa::advice< aa::tag<_on_mt_server_thread_select_>, ad::ad_stub<> > {};

struct _on_mt_server_start_{};
struct _on_mt_server_ready_read_{};
struct _on_mt_server_ready_write_{};
struct _on_mt_server_ready_urgent_{};
struct _on_mt_server_ready_error_{};
struct _on_mt_server_accept_{};
struct _on_mt_server_accept_error_{};
struct _on_mt_server_stop_{};


struct mt_server_stub_tags
  : aa::tag_list_n<
      _on_mt_server_start_,
      _on_mt_server_ready_read_,
      _on_mt_server_ready_write_,
      _on_mt_server_ready_urgent_,
      _on_mt_server_ready_error_,
      _on_mt_server_accept_,
      _on_mt_server_accept_error_,
      _on_mt_server_stop_
    >::type
{};

struct mt_mutex_advice: aa::advice< aa::tag<af::_mutex_>, ad::ad_mutex<ast::mutex> >{};

struct mt_server_stubs: aa::advice< mt_server_stub_tags, ad::ad_stub<> > {};

struct mt_server_aspect
  : aa::aspect<
      ap::type_list_n<
        mt_mutex_advice,
        mt_server_stubs/*,
        on_mt_server_thread_select_stub,
        on_mt_server_thread_exception_stub*/
      >::type
    >
{};

template< typename S,
          typename A,
          typename SA>
struct make_child_server_super_class
{
  typedef typename S::template rebind<
      typename aa::aspect_merge<aa::aspect<mt_mutex_advice>, A  >::type,
      typename aa::aspect_merge<aa::aspect<mt_mutex_advice>, SA >::type,
      false,
      S::net_protocol, S::transport_protocol,
      typename S::connection_type
    >::type type;
};


template< typename A /*= aa::aspect<>*/,
          typename AS /*= mt_server_aspect*/,
          typename S /*= default_server*/
        >
class child_server
: public make_child_server_super_class< S, A, AS>::type
{
public:
  typedef child_server<A, AS, S> self;
  typedef typename make_child_server_super_class< S, A, AS >::type super;
  typedef typename super::desc_type desc_type;
  typedef typename super::mutex_type mutex_type;
  typedef typename super::connection_type connection_type;
  typedef int pipe_desc_type;

  child_server( )
    : _drp(-1)
  {
  }
  
    template<typename AA,
           typename SASA,
           typename SS>
  struct rebind
  {
    typedef child_server< AA, SASA, SS > type;
  };


  void set_rd( desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _drp = d;
  }

  desc_type get_rd( ) const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _drp;
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
      if (s == 0 )
        super::_stop(*this);
      else if ( s == sizeof(cli) )
      {
        if (super::_get_status())
        {
          if ( cli == static_cast<desc_type>(-1))
            super::_stop(*this);
          else
            super::_accept(*this, cli, address_t() );
        }
        else
          fas::system::inet::close(cli);
      }
    }
  }
private:
  pipe_desc_type _drp;
};

template<typename S, typename M>
class mt_thread
{
  typedef S server_type;
  typedef M mux_type;
  typedef typename server_type::mutex_type mutex_type;
public:
  mt_thread(): _server(0){}
  mt_thread(const mt_thread& mtt): _server(mtt._server){}

  mt_thread(server_type* s): _server(s){}

  void operator()()
  {
    mux_type m;
    _server->set_mux(&m);
//    _server->idle_start();
    m.set_rhandler(_server->get_rd(), _server);

    for (;*_server;) 
    {
      try
      {
         m.select(1000);
      }
      catch(std::exception& e)
      {
        typename mutex_type::scoped_lock sl(_server->get_mutex() );
//        aa::get_advice<_on_mt_server_thread_exception_>(*_server)(*_server, e);
      }
      catch(...)
      {
        typename mutex_type::scoped_lock sl(_server->get_mutex() );
//        aa::get_advice<_on_mt_server_thread_exception_>(*_server)(*_server);
      }
    }

//#warning доделать 
    m.select(0);
    fas::system::close(_server->get_rd());
  }

public:
  server_type* _server;
};


template<typename A = aa::aspect<>,   // connection aspect
         typename SA = aa::aspect<>,  // server_aspect
         typename AMS = aa::aspect<>, // mt_server_aspect
         bool NPS = false,
         typename M = am::best_mux,
         typename S = server<>// default_server
 >
class mt_server
  : public aa::aspect_class<mt_server_aspect, AMS>
  , public am::imux_observer<typename S::desc_type>
{
private:
  typedef aa::aspect_class<mt_server_aspect, AMS> super;
  typedef mt_server<A, SA, AMS, NPS, M,  S> self;

  typedef typename aa::aspect_merge< 
               typename make_connection_aspect<self, NPS>::type, A
            >::type prototype_conn_aspect;

  typedef int pipe_desc_type;
              

  typedef child_server<prototype_conn_aspect, SA, S> server_prototype;
public:
  typedef typename server_prototype::desc_type desc_type;
  typedef typename server_prototype::connection_type connection_type;
  typedef typename server_prototype::connection_aspect connection_aspect;
  typedef am::imux<desc_type> imux;
  
  typedef typename super::aspect aspect;
  enum {net_protocol = S::net_protocol, transport_protocol = S::transport_protocol};
  enum {used_server_advice = NPS };

  struct child_info
  {
    desc_type rd;
    desc_type wd;
    server_prototype* chd; 
  };
  typedef std::vector<child_info> childs;
  typedef typename childs::iterator iterator;

private:

  mt_server(const self& ) {}

  
public:
  typedef typename aa::advice_cast<af::_mutex_, aspect >::advice mutex_advice;
  typedef typename mutex_advice::mutex_type mutex_type;

  mt_server()
    : _started(false)
    , _nonblock(false)
    , _keepalive(false)
    , _listen_queue_size(SOMAXCONN)
    , _mux(0)
    , _max_threads(1)
    , _count(0)
  {
    f_set_server<used_server_advice>()( get_server_prototype().get_prototype(), this);
  }

  mutex_type& get_mutex() const
  {
    return static_cast<const mutex_advice&>(*this).get_mutex();
  }

  mutex_type& get_mutex()
  {
    return static_cast<mutex_advice&>(*this).get_mutex();
  }
  
  void set_max_threads(size_t n) 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _max_threads = n; 
  }

  size_t get_max_threads(size_t n) const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _max_threads; 
  }

  void set_mux(imux* mux)
  {
    _server_prototype->set_mux(mux);

    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _mux = mux;
  }

  imux* get_mux() 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _mux;
  }

  iterator begin() { return _childs.begin(); }
  iterator end() { return _childs.end(); }

  connection_type& get_prototype() { return _server_prototype.get_prototype(); }
  const connection_type& get_prototype() const { return _server_prototype.get_prototype(); }

  connection_type* operator->(){ return &(_server_prototype.get_prototype()); }
  const connection_type* operator->() const{ return &(_server_prototype.get_prototype());}
  
  server_prototype& operator* () { return _server_prototype; }
  const server_prototype& operator* () const { return _server_prototype; }
  
  server_prototype& get_server_prototype() { return _server_prototype; }
  const server_prototype& get_server_prototype() const { return _server_prototype; }
  
  void set_nonblock(bool value = true)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _nonblock = value;
    _server_prototype.set_nonblock(value);
  }

  void set_keepalive(bool value)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _keepalive = value;
    _server_prototype.set_keepalive(value);
  }
  
  void set_max_connections(size_t value)
  {
    _server_prototype.set_max_connections(value);
  }
  
  size_t get_max_connections() const
  {
    return _server_prototype.get_max_connections();
  }

  size_t get_connection_count() const 
  {
    size_t result = 0;
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    typename childs::const_iterator beg = _childs.begin();
    typename childs::const_iterator end = _childs.end();
    for ( ;beg!=end; ++beg)
      result += beg->chd->get_connection_count();
    return result;
  }

  size_t get_pooled_connection_count() const 
  {
    size_t result = 0;
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    typename childs::const_iterator beg = _childs.begin();
    typename childs::const_iterator end = _childs.end();
    for ( ;beg!=end; ++beg)
      result += beg->chd->get_pooled_connection_count();
    return result;
  }

  template<typename F>
  F for_each(F f) const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    typename childs::const_iterator beg = _childs.begin();
    typename childs::const_iterator end = _childs.end();
    for ( ;beg!=end; ++beg)
      f = beg->chd->for_each(f);
    return f;
  }


  size_t size() const 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return _threads.size();
  }

  operator bool () const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _started;
  }

  void start(const std::string& addr, unsigned short port)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_start(*this, addr, port);
  }

  void start(unsigned short port)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_start( *this, std::string(""), port);
  }

  void start()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_start( *this);
  }

  void stop()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_stop(*this);
  }

  void join()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_join();
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


  void set_listen_queue_size(int value)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _listen_queue_size = value;
  }

protected:

  template<typename T>
  void _start(T& t, const std::string& addr, unsigned short port)
  {
    if (_started) return;
    
    if ( !addr.empty() )
      _local_address = asi::create_address(addr, port, net_protocol );
    else
      _local_address = asi::create_address(port, net_protocol );

    self::_start(t);
  }

  void _join()
  {
    if (!_childs.empty())
    {
      _threads.join_all();

      typename childs::iterator itr = _childs.begin();
      for ( ; itr!=_childs.end(); ++itr )
        delete itr->chd;
      _childs.clear();
    }
  }

  template<typename T>
  void _start(T& t)
  {
    if (_started ) return;

    self::_join();

    _childs.resize(_max_threads);
    for (volatile size_t i = 0; i < _max_threads; ++i)
    {
      int fds[2];
      int pr = pipe(fds);

      if (pr == 0)
      {
        ::fas::system::inet::nonblock(fds[1]);
        ::fas::system::inet::nonblock(fds[0]);

        _childs[i].rd = fds[0];
        _childs[i].wd = fds[1];
        _childs[i].chd = new server_prototype(_server_prototype);
        _childs[i].chd->set_rd( fds[0] );
        _childs[i].chd->idle_start(_local_address);
        _threads.create_thread( mt_thread<server_prototype, M>( _childs[i].chd ) );
      }
    }

    self::get_server_prototype().set_mux(_mux);

    _listen = asi::socket(
                 net_protocol,
                 transport_protocol
               );

    if (_nonblock) asi::nonblock( _listen );

    if (transport_protocol == TCP) 
      asi::reuseaddr( _listen );

    asi::bind(_listen, _local_address);

    if (transport_protocol  == TCP) 
      asi::listen(_listen, _listen_queue_size);

    if (_mux)
      _mux->set_rhandler( _listen, &t);

    typedef typename aa::advice_cast<
       _on_mt_server_start_,
       aspect >::advice on_mt_server_start;
     static_cast<on_mt_server_start&>(t)(t, _local_address);
    _started = true;
  }



  template<typename T>
  void _ready_read(T& t, desc_type d)
  {
    if (_count >= _max_threads ) _count = 0;

    address_t a;
    desc_type cli = ::fas::system::inet::accept(d, a);
    if (cli > 0)
    {

      if ( sizeof(int) != ::fas::system::write( _childs[_count++].wd, (char*)&cli, sizeof(cli) ) )
        ::fas::system::inet::close(cli);

    typedef typename aa::advice_cast<
       _on_mt_server_accept_,
       aspect >::advice on_mt_server_accept;
     static_cast<on_mt_server_accept&>(t)(t, _local_address);
    }
  }

  template<typename T>
  void _ready_write(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
       _on_mt_server_ready_write_,
       aspect >::advice on_mt_server_ready_write;
     static_cast<on_mt_server_ready_write&>(t)(t, _local_address);
  }

  template<typename T>
  void _ready_urgent(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
       _on_mt_server_ready_urgent_,
       aspect >::advice on_mt_server_ready_urgent;
    static_cast<on_mt_server_ready_urgent&>(t)(t, _local_address);
  }

  template<typename T>
  void _ready_error(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
       _on_mt_server_ready_error_,
       aspect >::advice on_mt_server_ready_error;
    static_cast<on_mt_server_ready_error&>(t)(t, _local_address);
    self::_stop(t);
  }

  template<typename T>
  void _stop(T& t)
  {
    if (!_started)
      return;


    if (_listen!=-1)
    {
      if ( _mux  )
        _mux->reset_rhandler( _listen );

      asi::close(_listen);

      _listen = -1;
    }

    typename childs::iterator itr = _childs.begin();

    for ( ; itr!=_childs.end(); ++itr )
    {
      fas::system::close(itr->wd);
      itr->chd->stop();
    }

    typedef typename aa::advice_cast<
       _on_mt_server_stop_,
       aspect >::advice on_mt_server_stop;
    static_cast<on_mt_server_stop&>(t)(t, _local_address);

    _started = false;
  }

  bool _started;
  bool _nonblock;
  bool _keepalive;
  int _listen_queue_size;
  imux* _mux;
  ast::thread_group _threads;
  childs _childs;

  size_t _max_threads;
  size_t _count;
  server_prototype _server_prototype;

  desc_type _listen;
  address_t _local_address;
};

}}

#endif
