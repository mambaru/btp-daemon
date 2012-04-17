//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_CLIENT_HPP
#define FAS_INET_CLIENT_HPP

#include <fas/aop/aspect.hpp>
#include <fas/pattern/type_list.hpp>
#include <fas/inet/types.hpp>
#include <fas/inet/constants.hpp>
#include <fas/inet/connection.hpp>
#include <fas/mux/mux_filter.hpp>
#include <fas/system/inet.hpp>

#include <list>
#include <vector>
#include <cstring>

namespace fas { namespace inet {

namespace ad = ::fas::adv;
namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace am = ::fas::mux;
namespace asi = ::fas::system::inet;
namespace af = ::fas::filter;

struct _on_client_connect_{};
struct _on_client_connected_{};
struct _on_connect_error_{};
struct _on_client_error_{};
struct _on_client_close_{};

struct ad_client_error
{
  template<typename T>
  void operator()(T& t, const asi::socket_error& err)
  {
    throw err;
  }
};

struct on_client_connect_stub: aa::advice< aa::tag<_on_client_connect_>, ad::ad_stub<> > {}; // сразу после вызова connect
struct on_client_connected_stub: aa::advice< aa::tag<_on_client_connected_>, ad::ad_stub<> > {}; // сразу после вызова connect
struct on_client_close_stub: aa::advice< aa::tag<_on_client_close_>, ad::ad_stub<> > {}; // нигде не вызывается
struct on_connect_error_stub: aa::advice< aa::tag<_on_connect_error_>, ad_client_error > {}; 
struct on_client_error_stub: aa::advice< aa::tag<_on_connect_error_>, ad_client_error > {}; 

struct client_advice_list
  : ap::type_list_n<
          on_client_connect_stub,
          on_client_close_stub,
          on_connect_error_stub,
          on_client_error_stub,
          on_client_connected_stub
#ifdef WIN32
          , am::recv_advice
          , am::send_advice
#endif
        >::type 
{};

//struct cal: client_advice_list{};

struct client_aspect: aa::aspect< client_advice_list > {};

/*
template<typename C, typename A>
struct make_client_super_class
{
  typedef typename C::template rebind<
               typename aa::aspect_merge<client_aspect, A>::type
             >::type type;
};*/

template<template<typename, typename> class C , typename A, typename F>
struct make_client_super_class
{
  typedef C<typename aa::aspect_merge<client_aspect, A>::type, F> type;
};
template<typename A = aa::aspect<>,
         net_protocol_t NP = IPv4,
         transport_protocol_t TP = TCP,
         template<typename, typename> class C  = connection,
         typename F = af::binary_filter<>
         //typename C = connection< client_aspect >
         >
class client
  : public make_client_super_class<C, A, F>::type
  //: public make_client_super_class< C<client_aspect, F >, A>::type
{
public:
  typedef client<A, NP, TP, C, F> self;
  typedef typename make_client_super_class<C, A, F>::type super;
  //typedef typename make_client_super_class< C<client_aspect, F >, A>::type super;
  typedef typename super::aspect aspect;

  typedef typename super::read_return_type read_return_type;
  typedef typename super::write_return_type write_return_type;
  typedef typename super::read_size_type read_size_type;
  typedef typename super::write_size_type write_size_type;

  typedef typename super::size_type size_type;
  typedef typename super::mutex_type mutex_type;

  template<typename AA = A, net_protocol_t NPNP = NP, transport_protocol_t TPTP = TP, 
          template<typename, typename> class  CC = C, typename FF = F >
  struct rebind
  {
    typedef client< AA, NPNP, TPTP, CC, FF> type;
  };


  typedef typename super::desc_type desc_type;

  enum {net_protocol = NP, transport_protocol = TP};

  client():_nonblock(false), _nonblock_connect(false), _connect_in_progress(false), _socket(-1){}

  ~client()
  { /*typename mutex_type::scoped_lock sl( self::get_mutex() ); self::_close(*this);*/ 
  }

  bool connect(const std::string& addr, unsigned short port, const std::string& baddr = std::string(), unsigned short bport = 0 )
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_connect(*this, addr, port, baddr, bport);
  }

  bool connect(const std::string& addr, const std::string& baddr = std::string())
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_connect(*this, addr, baddr);
  }

  bool reconnect(bool check_and_close = false)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_reconnect(*this, check_and_close );
  }

  void close()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_close(*this);
  }

  void set_nonblock(bool nonblock, bool nonblock_connect = false)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _nonblock = nonblock;
    _nonblock_connect = nonblock_connect;
  }

  bool _get_status() const
  {
    return _connect_in_progress || super::_get_status();
  }

  bool get_status() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_status();
  }

  operator bool () const  { return self::get_status(); }


  /** Осуществляет чтение данных в пользовательский 
    * буффер. Если во время операции источник стал
    * закрыт, то вызывает метод release() */
  read_return_type read(char* d, read_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this, d, s);
  }

  /** Осуществляет чтение данных во внутренний буфер
    * буффер. Если во время операции источник стал
    * недоступен, то вызывает метод release() */
  read_return_type read()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this);
  }

  /** Осуществляет запись данных из пользовательского
    * буфера. Если во время операции получатель стал
    * недоступен, то вызывает метод release() */
  write_return_type write(const char* d, write_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this, d, s);
  }

  write_return_type write(const char* d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this, d, std::strlen(d) );
  }


  /** Осуществляет запись данных из внутреннего
    * буфера. Если во время операции получатель стал
    * недоступен, то вызывает метод release() */
  write_return_type write()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this);
  }

  /** Подключение в прогрессе...
      под windows нужно вызвать write и проверить еще раз, т.к. select не работает должным образом
    */
  bool in_progress() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _in_progress();
  }

  desc_type get_socket() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _get_socket();
  }

protected:

  bool _in_progress() const
  {
    return _connect_in_progress;
  }

  template<typename T>
  bool _connect(T& t, const asi::address_t& remote_address, const asi::address_t& baddr = asi::address_t() )
  {
    

    if ( remote_address.empty() )
      throw std::logic_error("fas::inet::client::_connect: remote address is empty");

    _connect_address = remote_address;
    _bind_address = baddr;

    if ( _in_progress() )
    {
      _connect_ready( t, _socket, -1);
    }
    else if ( _get_status() )
    {
      self::_close(t);
    }

    _buffers.clear();

    _socket = asi::socket( net_protocol, transport_protocol );

    if ( !baddr.empty() )
      asi::bind( _socket, baddr );

    if (_nonblock_connect)
      asi::nonblock( _socket );

    if (transport_protocol == asi::TCP) 
      asi::tcp_nodelay(_socket);

    bool cr = false;

    try
    {
      cr = asi::connect(_socket, remote_address);
    }
    catch(const asi::socket_error& err)
    {
       asi::close( _socket );
       _socket = -1;
       t.get_aspect().template get<_on_connect_error_>()(t, err );
       return false;
    }

    _connect_in_progress = !cr;

    asi::address_t local_address( asi::getsockname(_socket) );

    if ( _nonblock && !_nonblock_connect )
      asi::nonblock( _socket );

    if ( !_connect_in_progress )
    {
      
      super::_assign(t, _socket, _socket, local_address, remote_address);
      
    }

    
    t.get_aspect().template get<_on_client_connect_>()(t);
    if ( !_connect_in_progress )
      t.get_aspect().template get<_on_client_connected_>()(t);
    

    return cr;
  }


  template<typename T>
  bool _connect(T& t, const std::string& addr, unsigned short port, const std::string& baddr = std::string(), unsigned short bport = 0)
  {
    asi::address_t remote_address = asi::create_address(addr, port, net_protocol );
    asi::address_t baddress;
    if ( !baddr.empty() || bport!=0 )
      baddress = asi::create_address(baddr, bport, net_protocol);

    return _connect(t, remote_address, baddress);
  }

  template<typename T>
  bool _connect(T& t, const std::string& addr, const std::string& baddr = std::string() )
  {
    std::string::size_type colon_pos = addr.find(':');
    if ( colon_pos == std::string::npos )
      throw std::runtime_error("fas::inet::client::connect: port is not specified");

    if ( colon_pos == 0 )
      throw std::runtime_error("fas::inet::client::connect: address is not specified");

    if ( colon_pos >= addr.size()-1 || !isdigit(addr[colon_pos+1]))
      throw std::runtime_error("fas::inet::client::connect: port is not specified");

    std::string host = std::string(addr.begin(), addr.begin() + colon_pos );
    unsigned short port = std::atoi( &(*(addr.begin() + colon_pos + 1)));

    std::string bhost;
    unsigned short bport = 0;
    colon_pos = baddr.find(':');

    if ( colon_pos == std::string::npos )
      bhost = baddr;
    else
    {
      if ( colon_pos > 0 )
        bhost = std::string(baddr.begin(), baddr.begin() + colon_pos );
      bport = std::atoi( &(*(baddr.begin() + colon_pos + 1)));
    }

    return self::_connect(t, host, port, bhost, bport) ;
  }

public:

  template<typename T>
  bool _reconnect(T& t, bool check_and_close)
  {
    if ( _in_progress() )
      return false;

    /*
     if ( _in_progress() )
      return false;
    if ( _get_status() )
    {
      if (check_and_close)
        self::_close(t);
      else
        return true;
    }
    */

    return self::_connect(t, _connect_address, _bind_address );
  }

  template<typename T>
  void _close(T& t)
  {
    
    _buffers.clear();
    _connect_in_progress = false;

    
    asi::shutdown( _socket );
    
    _socket = -1;
    super::_close(t);
    
  }

public:

  template<typename T>
  read_return_type _read( T& t, char* d, read_size_type s )
  {
    
    if (_connect_in_progress)
      _check_and_write(t);

    if ( !_connect_in_progress )
      return super::_read(t, d, s);
    return -1;
  }

  template<typename T>
  read_return_type _read(T& t)
  {
    

    if (_connect_in_progress)
      _check_and_write(t);

    if ( !_connect_in_progress )
      return super::_read(t);
    return -1;
  }

  template<typename T>
  write_return_type _write(T& t, const char* d, write_size_type s)
  {
    
    if (_connect_in_progress)
      _check_and_write(t);

    if ( !_connect_in_progress )
      return super::_write(t, d, s);
    else if ( s > 0)
    {
      _buffers.push_back( buffer_type(d, d + s) );
      // std::copy(d, d + s, std::back_inserter(_buffer) );
    }
    return -1;
  }

  template<typename T>
  write_return_type _write(T& t)
  {
    
    if (_connect_in_progress)
      return _check_and_write(t);

    if ( !_connect_in_progress )
      return super::_write(t);
    return -1;
  }

private:
  template<typename T>
  write_return_type __write_buffer(T& t)
  {
    write_return_type write_size = 0;

    if ( !_buffers.empty() )
    {
      buffers_list::iterator itr = _buffers.begin();
      for (;itr!=_buffers.end(); ++itr)
      {
        write_return_type wr = super::_write(t, &((*itr)[0]), (*itr).size() );

        if ( static_cast<size_t>(wr) != itr->size() )
          throw std::runtime_error("client::__write_buffer: error write buffer");

        write_size += wr;
      }
      _buffers.clear();
      return write_size;
    }
    return -1;
  }
protected:
  template<typename T>
  write_return_type _check_and_write(T& t)
  {
    if ( fas::system::inet::check_socket( _socket ) )
    {
      if ( _connect_in_progress )
         return _connect_ready(t, _socket, 0);
      else
         return __write_buffer(t);
    }
    return -1;
    
  }

  // err > 0 - код ошибки
  // err == 0 ок
  // err == -1 - отмена
  // err == -2 - прочие ошибки
  template<typename T>
  write_return_type _connect_ready(T& t, desc_type d, int err = 0 )
  {
    
    if ( d!=_socket)
      throw std::runtime_error("client::_connect_ready: invalid socket");

    if ( _connect_in_progress )
    {
      _connect_in_progress = false;
      if ( err == 0 )
      {
        asi::address_t local_address( asi::getsockname(_socket) );
        asi::address_t remote_address( asi::getpeername(_socket) );
        super::_assign(t, _socket, _socket, local_address, remote_address);
        t.get_aspect().template get<_on_client_connected_>()(t);
        return __write_buffer(t);
      }
      else
      {
        _buffers.clear();
        if ( _socket )
          asi::close(_socket);

        _socket = -1;

        if ( err > 0 )
          t.get_aspect().template get<_on_connect_error_>()(t, asi::socket_error( "fas::inet::client::connect ",  err) );
        else if ( err == -2 )
          t.get_aspect().template get<_on_connect_error_>()(t, asi::socket_error( "fas::inet::client::connect: ошибка неблокируемого подключения ", -1) );

        return -1;
      }
    }
    return __write_buffer(t);
  }

  desc_type _get_socket() const { return _socket; }


private:
  bool _nonblock;
  bool _nonblock_connect;
  bool _connect_in_progress;
  typedef std::vector<char> buffer_type;
  typedef std::list< buffer_type > buffers_list;
  buffers_list _buffers;
  desc_type _socket;
protected:
  asi::address_t _connect_address; // для неблокируемого connect
  asi::address_t _bind_address; // для неблокируемого connect
};
/*
template< typename A = aa::aspect<>,
          template<typename, typename> class C = mux_connection_base,
          typename F = connection<aa::aspect<>, am::mux_filter_base<> >
        >
class mux_connection
  : public C<A, F>// mux_connection_base<A, F>
*/

template < typename A = aa::aspect<>,
           net_protocol_t NP = IPv4,
           transport_protocol_t TP = TCP,
           template<typename, typename> class C = mux_connection_base,
           typename F = connection<aa::aspect<>, am::mux_filter_base<> >
         >
class mux_client_base
  // : public client<A, NP, TP, C>
  : public client<A, NP, TP, C, F>
{
  bool connect(const std::string& , unsigned short , const std::string& = std::string(), unsigned short = 0 ) { return false; }

  bool connect(const std::string& , const std::string&  = std::string()) { return false; }

  bool reconnect(bool = false) { return false;  }

  void close(){ }


public:
  typedef mux_client_base<A, NP, TP, C, F> self;
  typedef client<A, NP, TP, C, F> super;
  typedef typename super::mutex_type mutex_type;
  typedef typename super::desc_type desc_type;

  template<typename AA = A, net_protocol_t NPNP = NP, transport_protocol_t TPTP = TP,
           template<typename, typename> class CC = C, typename FF = F>
  struct rebind
  {
    typedef mux_client_base<AA, NPNP, TPTP, CC, FF > type;
  };

  template<typename T>
  void _ready_read(T& t, desc_type d)
  {
    
    if ( super::_in_progress() )
      throw std::logic_error("fas::inet::mux_client_base::_ready_read: ready read for progress connect");
      // super::_check_and_write(t);
    super::_ready_read(t, d);
  }

  template<typename T>
  void _ready_write(T& t, desc_type d)
  {
    
    if ( super::_in_progress() )
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      super::_connect_ready(t, d, asi::getsockerror(d) );
    }
    else
      super::_ready_write(t, d);
  }

  template<typename T>
  void _ready_urgent(T& t, desc_type d)
  {
   
    if ( super::_in_progress() )
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      int err =  asi::getsockerror(d);
      super::_connect_ready(t, d, err!=0 ? err : -2 ); // под виндой код ошибки
    }
    else
      super::_ready_urgent(t, d);
  }

  template<typename T>
  void _ready_error(T& t, desc_type d)
  {
    super::_ready_error(t, d);
    if ( super::_in_progress() )
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      int err =  asi::getsockerror(d);
      super::_connect_ready(t, d, err!=0 ? err : -2 ); // под виндой код ошибки
    }
  }

  template<typename T>
  bool _connect(T& t, const asi::address_t& addr, const asi::address_t& baddr = asi::address_t() )
  {
    
    if ( super::_in_progress() )
    {
      
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      super::_connect_ready(t, super::_get_socket(), -1);
      
    }
    else if ( super::_get_status() )
    {
      if ( !this->release(false) )
      {
        this->release(false);
      }
    }

    if ( super::_in_progress() )
      return false;

    if ( super::_get_status() )
      return true;


    bool result = super::_connect(t, addr, baddr);
    
    if (super::_in_progress())
    {
    
      super::_set_wd( super::_get_socket() ); // Устанавливает сокет в обход _assign
      super::_set_rd( super::_get_socket() ); // Устанавливает сокет в обход _assign
    
      super::_set_write_handler(t); // ignore status
#ifdef WIN32
      super::_set_urgent_handler(t); // ignore status
#endif
    }
    
    return result;
  }


  template<typename T>
  bool _connect(T& t, const std::string& addr, unsigned short port, const std::string& baddr = std::string(), unsigned short bport = 0)
  {

    if ( super::_in_progress() )
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      super::_connect_ready(t, super::_get_socket(), -1);
    }

    if ( super::_get_status() )
    {
      if ( !this->release(false) )
        this->release(false);
    }

    if ( super::_in_progress() )
      return false;

    if ( super::_get_status() )
      return true;


    bool result = super::_connect(t, addr, port, baddr, bport);
    if (super::_in_progress())
    {
      super::_set_wd( super::_get_socket() ); // Устанавливает сокет в обход _assign
      super::_set_rd( super::_get_socket() ); // Устанавливает сокет в обход _assign
      super::_set_write_handler(t);
#ifdef WIN32
      super::_set_urgent_handler(t); // ignore status
#endif
    }
    return result;
  }

  template<typename T>
  bool _connect(T& t, const std::string& addr, const std::string& baddr = std::string())
  {
    if ( super::_in_progress() )
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      super::_connect_ready(t, super::_get_socket(), -1);
    }

    if ( super::_get_status() )
    {
      if ( !t.release(false) )
        t.release(false);
    }

    if ( super::_in_progress() )
      return false;

    if ( super::_get_status() )
      return true;

    bool result = super::_connect(t, addr, baddr);


    if (super::_in_progress())
    {
      super::_set_wd( super::_get_socket() ); // Устанавливает сокет в обход _assign
      super::_set_rd( super::_get_socket() ); // Устанавливает сокет в обход _assign
      super::_set_write_handler(t); // ignore status
#ifdef WIN32
      super::_set_urgent_handler(t); // ignore status
#endif
    }

    return result;
  }

  template<typename T>
  bool _reconnect(T& t, bool check_and_close)
  {

    if ( super::_in_progress() )
      return false;

    if ( super::_get_status() )
    {
      if (check_and_close)
      {
    
        if ( !this->release(false) )
        {
    
          this->release(false);
        }
    
      }
      else
      {
    
        return true;
      }
    }

    if ( super::_in_progress() )
      return false;

    if ( super::_get_status() )
      return true;

    bool result = self::_connect(t, super::_connect_address, super::_bind_address );

    return result;
  }


  template<typename T>
  bool _release(T& t)
  {
    if (super::_in_progress())
    {
      super::_reset_write_handler(t, true);
#ifdef WIN32
      super::_reset_urgent_handler(t);
#endif
      super::_connect_ready(t, super::_get_socket(), -1);
    }

    if ( super::_release(t) )
    {
      // Если во время super::_release(t) не случился рекоонект
      if ( !super::_get_status() )
        super::_close(t);
      return true;
    }
    return false;
  }

  /**@return hard = true : true - закрытие прошло нормально, false - в жестком режиме
             hard = false : true - закрытие прошло нормально, true - остались данные, закроется при следующем slect 
    */
  template<typename T>
  bool _close(T& t, bool hard = false )
  {
    
    bool r = this->release(false);
    if (!r && hard)
      this->release(false);
    
    return r;
  };
};

/*
template< typename A = aa::aspect<>,
          template<typename, typename> class C = mux_connection_base,
          typename F = connection<aa::aspect<>, am::mux_filter_base<> >
        >
class mux_connection
  : public C<A, F>// mux_connection_base<A, F>
*/

template<typename A = aa::aspect<>,
         net_protocol_t NP = IPv4,
         transport_protocol_t TP = TCP,
         template<typename, typename> class C = mux_connection_base,
         typename F = connection<aa::aspect<>, am::mux_filter_base<> >
         // typename C = connection<A, am::mux_filter_base<> > >
         // typename C = mux_connection_base<A>
>
class mux_client
  // : public client<A, NP, TP, C>
  //: public mux_client_base<A, NP, TP, C<A, F> >
  : public mux_client_base<A, NP, TP, C, F>
{
public:
  typedef mux_client<A, NP, TP, C, F > self;
  // typedef client<A, NP, TP, C> super;
  typedef mux_client_base<A, NP, TP, C, F > super;
  typedef typename super::mutex_type mutex_type;
  typedef typename super::desc_type desc_type;

  template<typename AA = A, net_protocol_t NPNP = NP, transport_protocol_t TPTP = TP, template<typename, typename> class CC = C, typename FF = F >
  struct rebind
  {
    typedef mux_client< AA,  NPNP, TPTP, CC, FF> type;
  };

  bool connect(const std::string& addr, unsigned short port, const std::string& baddr = std::string(), unsigned short bport = 0)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return super::_connect(*this, addr, port, baddr, bport);
    /*typename mutex_type::scoped_lock sl( super::get_mutex() );
    bool result = super::_connect(*this, addr, port, baddr, bport);
    if (super::_in_progress())
      super::_set_write_handler(*this);
    return result;*/
  }

  bool connect(const std::string& addr, const std::string& baddr = std::string())
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return super::_connect(*this, addr, baddr);
    /*
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    bool result = super::_connect(*this, addr, baddr);
    if (super::_in_progress())
      super::_set_write_handler(*this);
    return result;
    */
  }

  bool close(bool hard = false)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return super::_close(*this, hard);
  }

  bool reconnect(bool check_and_close = false)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return self::_reconnect(*this, check_and_close);
  }

  virtual void ready_read(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_read(*this, d);
  }

  virtual void ready_write(desc_type d) 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_write(*this, d);
  }

  virtual void ready_urgent(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_urgent(*this, d);
  }

  virtual void ready_error(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_error(*this, d);
  }

  virtual bool release(bool lock = true ) 
  {
    if (!lock) return self::_release(*this);
    typename mutex_type::scoped_lock sl( super::get_mutex()/*, lock ???? в новом бусте не компилится*/ );
    return super::_release(*this);
  }

protected:

  /*template<typename T>
  bool _release(T& t)
  {
    if (super::_release(t))
    {
      super::_close(t);
      return true;
    }
    return false;
  }*/

};

}}

#endif //FAS_INET_CLIENT_HPP
