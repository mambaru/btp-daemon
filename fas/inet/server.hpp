//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_SERVER_HPP
#define FAS_INET_SERVER_HPP

#include <fas/aop/aspect.hpp>
#include <fas/aop/advice.hpp>

//#include <fas/adv/io/ad_concurrent.hpp>

#include <fas/mux/imux.hpp>

#include <fas/inet/pool.hpp>
#include <fas/inet/types.hpp>
#include <fas/inet/constants.hpp>
#include <fas/inet/pooled_item.hpp>
#include <fas/inet/connection.hpp>

#include <fas/system/inet.hpp>


#include <stdexcept>

namespace fas { namespace inet {

namespace ad = ::fas::adv;
// namespace adio = ::fas::adv::io;
namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace af = ::fas::filter;
namespace am = ::fas::mux;
namespace asi = ::fas::system::inet;

struct _server_{};
// typedef af::_mutex_ _mutex_;

template<class S>
class ad_server
{
public:
  typedef S server;
  server* get_server() { return _server; }
  const server* get_server() const { return _server; }
  server* operator ->() { return _server;}
  const server* operator ->() const { return _server;}
  void set_server(server* value){ _server = value; }
private:
  server* _server;
};

class ad_no_server { public: void set_server(void* /*value*/){ } };

template<typename T, bool>
struct make_connection_aspect
{
  typedef aa::aspect< aa::advice< aa::tag<_server_>, ad_server<T> > > type;
};

template<typename T>
struct make_connection_aspect<T, false>
{
  typedef aa::aspect< aa::advice< aa::tag<_server_>, ad_no_server > > type;
};

template<bool>
struct f_set_server
{
  template<typename C, typename S>
  void operator()(C& c, S* s )
  {
    c.get_aspect().template get<_server_>().set_server(s);
  }
};

template<>
struct f_set_server<false>
{
  template<typename C, typename S>
  void operator()(C& , S*  )
  {
  }
};

struct _on_server_start_{};
struct _on_server_stop_{};
struct _on_server_accept_{};
struct _on_server_accept_error_{};
struct _on_server_ready_read_{};
struct _on_server_ready_write_{};
struct _on_server_ready_urgent_{};
struct _on_server_ready_error_{};

struct server_stub_tags
  : aa::tag_list_n<
      _on_server_ready_read_,
      _on_server_ready_write_,
      _on_server_ready_urgent_,
      _on_server_ready_error_,
      _on_server_start_,
      _on_server_accept_,
      _on_server_accept_error_,
      _on_server_stop_
    >::type
{};

struct server_stubs: aa::advice< server_stub_tags, ad::ad_stub<> > {};

struct mutex_advice: aa::advice< aa::tag<af::_mutex_>, ad::ad_mutex<> >{};

struct server_advice_list
  : ap::type_list_n<
          server_stubs,
          mutex_advice
    >::type {};

struct server_aspect: aa::aspect< server_advice_list >{};

template<bool B>
struct conditional { operator bool() const { return B;} };

/// !!!Ахтунг! серваку тож нужен мьютекс если исползуеться for_each!!!
template<typename A = aa::aspect<>,
         typename SA = aa::aspect<>,
         bool NSP = false,
         net_protocol_t NP = IPv4,
         transport_protocol_t TP = TCP,
         typename C = mux_connection<> >
class server_base:
  public aa::aspect_class<server_aspect, SA>,
  public am::imux_observer<typename C::desc_type>
{
public:
  typedef server_base<A, SA, NSP, NP, TP, C> self;
  typedef aa::aspect_class<server_aspect, SA > super;
  typedef typename super::aspect aspect;
  typedef typename aa::advice_cast<af::_mutex_, aspect >::advice mutex_advice;
  typedef typename mutex_advice::mutex_type mutex_type;
  mutex_type& get_mutex() const
  {
    return static_cast<const mutex_advice&>(*this).get_mutex();
  }

  typedef typename C::template rebind<
            typename aa::aspect_merge< 
               typename make_connection_aspect<self, NSP>::type, A
            >::type
          >::type connection_type;

  typedef typename connection_type::aspect connection_aspect;
 
  typedef typename connection_type::desc_type desc_type;

  template<typename AA /*= A*/,
           typename SASA /*= SA*/,
           bool NSPNSP /*= NSP*/,
           net_protocol_t NPNP /*= NP*/,
           net_protocol_t TPTP /*= TP*/,
           typename CC /*= C*/>
  struct rebind
  {
    typedef server_base< AA, SASA, NSPNSP, NPNP, TPTP, CC > type;
  };

  // struct _a: connection_type {};
  typedef pooled_item<connection_type> pooled_type;
  typedef pool< pooled_type, mutex_type > pool_type;
  typedef am::imux<desc_type> imux;
  typedef typename pool_type::iterator iterator;
  enum {net_protocol = NP, transport_protocol = TP};
  enum {used_server_advice = NSP };

  server_base()
    : _started(false)
    , _nonblock(false)
    , _keepalive(false)
    , _mux(0)
    , _listen( static_cast<desc_type>(-1) )
    , _max_connections( static_cast<size_t>(-1) ) 
    , _listen_queue_size(SOMAXCONN)
  {}

  iterator begin() { return _pool.abegin(); }
  iterator end() { return _pool.aend(); }

  server_base( const self& s)
    : super( static_cast<const super&>(s) )
    , _started(false)
    , _nonblock(s._nonblock)
    , _keepalive(s._keepalive)
    , _mux(s._mux)
    , _listen(-1)
    , _pool(s._pool)
    , _max_connections(s._max_connections)
    , _listen_queue_size(s._listen_queue_size)
  {
    f_set_server<used_server_advice>()(get_prototype(), this);
  }

  virtual ~server_base() { }

  operator bool () const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _started;
  }

  bool get_status () const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _started;
  }

  connection_type& get_prototype() { return _pool.get_prototype(); }
  const connection_type& get_prototype() const { return _pool.get_prototype(); }

  connection_type* operator->(){ return &_pool.get_prototype();}
  const connection_type* operator->() const{ return &_pool.get_prototype();}

  size_t get_max_connections() const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _max_connections;
  }

  void set_max_connections(size_t value) 
  { 
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _max_connections = value;
  }

  void set_mux(imux* mux)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _mux = mux;
    get_prototype().set_mux(mux);
  }

  imux* get_mux() 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _mux;
  }

  size_t get_connection_count() const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _pool.active_count();
  }

  size_t get_pooled_connection_count() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _pool.pooled_count();
  }

  void start(const std::string& addr, unsigned short port)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_start(*this, addr, port);
  }

  void start(unsigned short port)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_start(*this, "", port);
  }

  void start()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_start(*this);
  }

  void idle_start(address_t a)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_idle_start(*this, a);
  }


  void stop()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_stop(*this);
  }

  connection_type* accept()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_accept(*this);
  }

  void set_nonblock(bool value = true)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _nonblock = value;
  }

  void set_keepalive(bool value)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _keepalive = value;
  }

  void set_listen_queue_size(int value)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _listen_queue_size = value;
  }

  template<typename F>
  F for_each(F f) const
  {
    typedef typename pool_type::mutex_type pool_mutex;
    typename pool_mutex::scoped_lock sl( _pool.get_mutex() );
    return std::for_each(_pool.abegin(), _pool.aend(), f);
  }

  template<typename F>
  F for_each_all(F f) const
  {
    typedef typename pool_type::mutex_type pool_mutex;
    typename pool_mutex::scoped_lock sl( _pool.get_mutex() );
    f = std::for_each(_pool.abegin(), _pool.aend(), f);
    return std::for_each(_pool.pbegin(), _pool.pend(), f);
  }

  template<typename F>
  F for_each_pooled(F f) const
  {
    typedef typename pool_type::mutex_type pool_mutex;
    typename pool_mutex::scoped_lock sl( _pool.get_mutex() );
    return std::for_each(_pool.pbegin(), _pool.pend(), f);
  }


  template<typename T>
  void start_on_same_socket(const T &t) {
	if (_started) return;
	_listen = t._listen;

	if (_mux) _mux->set_rhandler( _listen, this);

	_started = true;
  }

protected:
  bool _get_status () const
  {
    return _started;
  }

  template<typename T>
  void _ready_read(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
      _on_server_ready_read_,
      aspect >::advice on_server_ready_read;
    static_cast<on_server_ready_read&>(t)(t, d);

    if ( conditional<transport_protocol  == TCP>() )
      self::_accept(t);
    else
      self::_accept_udp(t);
  }

  template<typename T>
  void _ready_write(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
      _on_server_ready_write_,
      aspect >::advice on_server_ready_write;
    static_cast<on_server_ready_write&>(t)(t, d);
  }

  template<typename T>
  void _ready_urgent(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
      _on_server_ready_urgent_,
      aspect >::advice on_server_ready_urgent;
    static_cast<on_server_ready_urgent&>(t)(t, d);
  }

  template<typename T>
  void _ready_error(T& t, desc_type d)
  {
    typedef typename aa::advice_cast<
      _on_server_ready_error_,
      aspect >::advice on_server_ready_error;
    static_cast<on_server_ready_error&>(t)(t, d);

    self::_stop(t);
  }

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

  template<typename T>
  void _start(T& t)
  {
    if (_started) return;

    _listen = asi::socket( net_protocol, transport_protocol );

    if ( conditional<transport_protocol == TCP>() )
      asi::reuseaddr( _listen );

    if (_nonblock) asi::nonblock( _listen );

    asi::bind(_listen, _local_address);

    if ( conditional<transport_protocol  == TCP>() )
      asi::listen(_listen, _listen_queue_size);
    else
    {
// #warning "заглушка"
      
      int optval = 16777216;
      socklen_t optlen = sizeof(optval);

      int rc = ::setsockopt(_listen, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&optval), optlen);
      if (rc < 0)
      {
        int err = errno;
        fprintf(stderr, "setsockopt failed to set SO_RCVBUF in %d, error = %s\n ", optval, strerror(err));
        fprintf(stderr, "some data gramms will be lost\n");
      }
    }

    if (_mux)
      _mux->set_rhandler( _listen, &t);

    typedef typename aa::advice_cast<
       _on_server_start_,
       aspect >::advice on_server_start;
     static_cast<on_server_start&>(t)(t, _local_address);

    _started = true;
  }


  template<typename T>
  void _idle_start(T& t, const address_t& a)
  {
    if (_started) return;
    _local_address = a;
    _started = true;
    typedef typename aa::advice_cast<
       _on_server_start_,
       aspect >::advice on_server_start;
     static_cast<on_server_start&>(t)(t, _local_address);
  }

  template<typename T>
  connection_type* _accept(T& t)
  {
    if ( !_started )
      return 0;

    connection_type* pc = 0;
    address_t remote_address;
    typename connection_type::desc_type d;
    if ( (d = asi::accept(_listen, remote_address)) != SOCKET_ERROR )
    {
      if (_max_connections <= _pool.active_count() )
      {
        asi::close( d );
        return 0;
      }

      if (_nonblock) 
        asi::nonblock( d );

      asi::tcp_nodelay(d);

      if (_keepalive /*> 0*/ )
        asi::keepalive(d, _keepalive);

      if ( (pc = _pool.create() )!=0 )
        pc->assign(d, _local_address, remote_address);

      typedef typename aa::advice_cast<
         _on_server_accept_,
         aspect >::advice on_server_accept;
      static_cast<on_server_accept&>(t)(t, d, remote_address);
    }
    else
    {
      typedef typename aa::advice_cast<
         _on_server_accept_error_,
         aspect >::advice on_server_accept_error;
      static_cast<on_server_accept_error&>(t)(t, _local_address);
    }
    return pc;
  }

  char _buffer[65635];
  template<typename T>
  connection_type* _accept_udp(T& t)
  {
    /// Ахтунг! пока только прием udp-пакетов!!!

    address_t remote_address;
    int s = asi::recvfrom(_listen, _buffer, sizeof(_buffer), remote_address);
    if ( s > 0 ) 
    {
      // std::cout << "UDP READ [ "<< std::string(_buffer, _buffer + s ) << "] " << int (_buffer[s - 2])  << " "<< int (_buffer[s - 1]) << std::endl;
      this->_pool.get_prototype().get_aspect().template get<af::_on_read_>()( _pool.get_prototype() , _buffer, s);
    }
    return &(_pool.get_prototype());
  }


  template<typename T>
  connection_type* _accept(T& t, const desc_type& d, const address_t& a)
  {

    if (!_started ||  _max_connections <= _pool.active_count() )
    {
      asi::close(d);
      return 0;
    }

    connection_type* pc = 0;

    if (_nonblock) 
      asi::nonblock( d );

    asi::tcp_nodelay(d);

    if (_keepalive > 0 )
        asi::keepalive(d, _keepalive);


    if ( (pc = _pool.create() )!=0 )
    {
      pc->assign( d, _local_address, a);
    }

    typedef typename aa::advice_cast<
       _on_server_accept_,
       aspect >::advice on_server_accept;
    static_cast<on_server_accept&>(t)(t, d, a);
    return pc;
  }

  template<typename T>
  void _stop(T& t)
  {
    if (!_started) return;
    _started = false;

    if ( _listen != -1)
    {
      if ( _mux  )
        _mux->reset_rhandler( _listen );

      asi::close(_listen);

      _listen = static_cast<desc_type>(-1);
    }

    _pool.release();
    if ( _mux  ) _mux->select(0);
    if ( _pool.process_free() > 0  )
    {
      _pool.release();
      while ( _pool.process_free() > 0 )
        _mux->select(-1);
      if ( _pool.active_count() > 0 )
        throw std::logic_error("hard server stop");
    }


    typedef typename aa::advice_cast<
       _on_server_stop_,
       aspect >::advice on_server_stop;
    static_cast<on_server_stop&>(t)(t, _local_address);

    

  }


private:
  bool _started;
  bool _nonblock;
  bool _keepalive;
  imux* _mux;
  desc_type _listen;
  pool_type _pool;
  address_t _local_address;
  size_t _max_connections;
  int _listen_queue_size;
};


template<typename A = aa::aspect<>,
         typename SA = aa::aspect<>,
         bool NSP = false,
         net_protocol_t NP = IPv4,
         transport_protocol_t TP = TCP,
         typename C = mux_connection<> >
class server:
  public server_base<A, SA, NSP, NP, TP, C >
{
public:
  typedef server<A, SA, NSP, NP, TP, C> self;
  typedef server_base<A, SA, NSP, NP, TP, C > super;
  typedef typename super::aspect aspect;
  typedef typename super::desc_type desc_type;
  typedef typename super::connection_type connection_type;
  //typedef typename super::mutex_type mutex_advice;
  typedef typename super::mutex_type mutex_type;

  template<typename AA,
           typename SASA,
           bool NSPNSP,
           net_protocol_t NPNP,
           net_protocol_t TPTP,
           typename CC>
  struct rebind
  {
    typedef server< AA, SASA, NSPNSP, NPNP, TPTP, CC > type;
  };

  virtual ~server(){}

  virtual void ready_read(desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_ready_read(*this, d);
  }

  virtual void ready_write(desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_ready_write(*this, d);
  }

  virtual void ready_urgent(desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_ready_urgent(*this, d);
  }

  virtual void ready_error(desc_type d) 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_ready_error(*this, d);
  }

  void start(const std::string& addr, unsigned short port)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_start(*this, addr, port);
  }

  void start(unsigned short port)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_start(*this, "", port);
  }

  void start()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_start(*this);
  }

  void stop()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    super::_stop(*this);
  }

  connection_type* accept()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return super::_accept(*this);
  }

};

}}

#endif 
