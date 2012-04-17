//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_SYSTEM_INET_HPP
#define FAS_SYSTEM_INET_HPP

#include <fas/system/system.hpp>

#include <vector>
#include <errno.h>
namespace fas{ namespace system { namespace inet {

// typedef system::socket_t socket_t;
/*
#ifdef WIN32
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif
*/
typedef std::vector<unsigned char> address_t;

typedef int net_protocol_t;
typedef int transport_protocol_t;

const net_protocol_t IPv4 = AF_INET;
const net_protocol_t IPv6 = AF_INET6;
const transport_protocol_t TCP = SOCK_STREAM;
const transport_protocol_t UDP = SOCK_DGRAM;

#ifdef WIN32
const int EINPROGRESS = WSAEWOULDBLOCK; //FIXME: 
const int EWOULDBLOCK = WSAEWOULDBLOCK;
const int ECONNABORTED = WSAECONNRESET;
const int ENOTSOCK = WSAENOTSOCK;
const int ENOTCONN = WSAENOTCONN;
const int ECONNREFUSED = WSAECONNREFUSED;
/*const int EINTR = WSAEINTR;
const int ENOTSOCK = WSAENOTSOCK;
const int ENOTCONN = WSAENOTCONN;
const int ENOMEM = WSA_NOT_ENOUGH_MEMORY;//FIXME:
const int EINVAL = WSAEINVAL;
const int EFAULT = WSAEFAULT;
const int ECONNREFUSED = WSAECONNREFUSED;
const int EBADF = WSAEBADF;*/
//const int EAGAIN = ::WSA_E_A

const int SHUT_RDWR = SD_BOTH;
typedef int socklen_t;
#else
//const int SOCKET_ERROR = -1;
#endif

/*
const net_protocol_t IPv4 = AF_INET;
const net_protocol_t IPv6 = AF_INET6;
const transport_protocol_t TCP = SOCK_STREAM;
const transport_protocol_t UDP = SOCK_DGRAM;
*/
class socket_error:
  public std::runtime_error
{
public:
  socket_error(const std::string& msg)
    : std::runtime_error(msg + fas::system::strerror(error_code())) 
  {}

  socket_error(const std::string& msg, int code)
    : std::runtime_error(msg + fas::system::strerror(code)) 
  {}
};

inline int error_code()
{
#ifdef WIN32
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

inline void init()
{
#ifdef WIN32
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
 
  wVersionRequested = MAKEWORD( 2, 2 );
 
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) 
    throw std::runtime_error("fas::system::inet::init: can't initialize WSA");
  
 
  if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
  {
    WSACleanup( );
    std::runtime_error("fas::system::inet::init: can't initialize WSA (not supported ver)"); 
  }
 
#endif
}

inline address_t create_address(unsigned short port, net_protocol_t np = IPv4)
{
  switch(np)
  {
  case IPv6: throw std::runtime_error("fas::system::inet::create_address: IPv6 unsupported");
  case IPv4:{
      sockaddr_in addr={0};
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(port);
      return address_t((char*)&addr, (char*)&addr + sizeof(addr));
    }
  }
  throw std::runtime_error("fas::system::inet::create_address: unknown net protocol");
}

inline address_t create_address(const std::string& addr_str, unsigned short port, net_protocol_t np = IPv4)
{
  switch(np)
  {
  case IPv6: throw std::runtime_error("fas::system::inet::create_address: IPv6 unsupported");
  case IPv4:
  {
    sockaddr_in addr={0};

    u_long iaddr = inet_addr( addr_str.c_str() );
    if ( iaddr != INADDR_NONE )
    {
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = iaddr;
    }
    else
    {
      hostent* h = gethostbyname(addr_str.c_str());

      if (h==0 || h->h_length < 1 )
        throw system_error("can not get address by name (gethostbyname) ");

      addr.sin_family = h->h_addrtype;
      addr.sin_addr.s_addr = *(u_long *) h->h_addr_list[0];
    }

    addr.sin_port = htons(port);
    return address_t((const char*)&addr, (const char*)&addr + sizeof(addr));

    }
  }
  throw std::logic_error("fas::system::inet::create_address: unknown net protocol");
}


inline address_t getsockname(socket_t sock)
{
  sockaddr_in6 addr={0};
  socklen_t s = sizeof(addr);
  if ( SOCKET_ERROR == ::getsockname(sock, (sockaddr*)&addr, &s)  )
    throw socket_error( "fas::system::inet::getsockname: " );
  return address_t( (const char*)&addr, (const char*)&addr + s );
}

// char *inet_ntoa(struct in_addr in);
inline bool address2string(const address_t& addr, std::string& ip)
{
  ip.clear();
  if ( addr.size() < sizeof(in_addr) )
    return false;
  char *chIp = inet_ntoa( *(reinterpret_cast<const in_addr*>(&(addr[0]))) );
  if (chIp==0)
    return false;
  ip=chIp;
  return true;
}

// gethostname()
inline std::string gethostname()
{
  char ch[255];
  if ( 0 == ::gethostname(ch, 255) )
    return ch;
  return std::string();
}

inline std::string gethostip()
{
   std::string host = gethostname();

   hostent* h = gethostbyname(host.c_str());
   if (h==0 || h->h_length < 1 )
        return std::string();

   char *chIp = inet_ntoa( *((in_addr*)(h->h_addr_list[0])) );
   if (chIp==0)
     return std::string();
   return chIp;
};



inline address_t getpeername(socket_t sock)
{
  sockaddr_in6 addr={0};
  socklen_t s = sizeof(addr);
  if ( SOCKET_ERROR == ::getpeername(sock, (sockaddr*)&addr, &s)  )
    throw socket_error( "fas::system::inet::getpeername: " );
  return address_t( (const char*)&addr, (const char*)&addr + s );
}

inline socket_t socket(net_protocol_t np = IPv4, transport_protocol_t tp = TCP )
{
  //throw;
  socket_t s = ::socket( np, tp, 0);
  if ( s == SOCKET_ERROR )
    throw socket_error( "fas::system::inet::socket: " );

  return s;
}

inline void listen(const socket_t d, int maxconn = SOMAXCONN )
{
  if ( ::listen(d, maxconn) != 0)
    throw socket_error("fas::system::inet::listen: ");
}

inline void bind(const socket_t d, const address_t& addr)
{
  if ( ::bind(d, reinterpret_cast<const sockaddr*>( &(addr[0]) ), static_cast<socklen_t>( addr.size() ) ) != 0 )
    throw socket_error("fas::system::inet::bind: ");
}

inline bool connect(const socket_t d, const address_t& addr)
{
  int n = ::connect( d, reinterpret_cast<const sockaddr*>( &(addr[0]) ), static_cast<socklen_t>( addr.size()) );
  if ( n != 0)
  {
    if ( error_code() == EINPROGRESS)
      return false;
    else
      throw socket_error("fas::system::inet::connect: ");
  }
  return true;
}

/*
inline socket_t accept(const socket_t d, address_t& a)
{
  sockaddr_in6 addr;
  socklen_t size = sizeof(addr);
  socket_t fd;
#ifndef WIN32
  if ( (fd = ::accept( d, reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t*>(&size) )) < 0)
#else
  if ( (fd = ::accept( d, reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t*>(&size))) == SOCKET_ERROR )
#endif
  {
    int err = error_code(); 
    if ( err == EWOULDBLOCK || err==ECONNABORTED || err==EINTR )
      return SOCKET_ERROR;
    else
      throw socket_error( "fas::system::inet::accept: " );
  }
  a.assign(reinterpret_cast<char*>(&addr), reinterpret_cast<char*>(&addr) + size);
  return fd;
};
*/

#ifdef WIN32
inline SOCKET accept(const SOCKET d, address_t& a)
{
  sockaddr_in6 addr;
  int size = sizeof(addr);
  SOCKET fd;
  if ( (fd = ::accept( d, reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t*>(&size))) == INVALID_SOCKET  )
  {
    int err = error_code(); 
    if ( err == WSAEINPROGRESS || err==WSAEWOULDBLOCK || err==WSAEINTR )
      return INVALID_SOCKET ;
    else
      throw socket_error( "fas::system::inet::accept: " );
  }

  a.assign(reinterpret_cast<char*>(&addr), reinterpret_cast<char*>(&addr) + size);
  return fd;
};
#else
inline int accept(const int d, address_t& a)
{
  sockaddr_in6 addr;
  socklen_t size = sizeof(addr);
  int fd;
  if ( (fd = ::accept( d, reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t*>(&size) )) < 0)
  {
    int err = error_code(); 
    if ( err == EWOULDBLOCK || err==ECONNABORTED || err==EINTR )
      return SOCKET_ERROR;
    else
      throw socket_error( "fas::system::inet::accept: " );
  }
  a.assign(reinterpret_cast<char*>(&addr), reinterpret_cast<char*>(&addr) + size);
  return fd;
};

#endif

#ifdef WIN32

inline SOCKET accept(const SOCKET d)
{
  address_t temp;
  return accept(d, temp); 
}

#else

inline int accept(int d)
{
  address_t temp;
  return accept(d, temp); 
}

#endif

/**
* 0 - закрыт
* <0 - кончились данные для неблокируемого сокета 
*/
#ifdef WIN32
inline int recv(const SOCKET d, char* buff, int s)
{
  int ret = ::recv(d, buff, s, 0);
  if (ret == SOCKET_ERROR)
  {
    int err = error_code();
    if (err == WSAEINTR || err == WSAEINPROGRESS || err == WSAEWOULDBLOCK 
        || err == WSAETIMEDOUT || err == WSATRY_AGAIN || err == WSA_IO_PENDING)
      return -1;
    if (err == WSAENETRESET || err == WSAESHUTDOWN || err == WSAEDISCON || err == WSAECONNRESET || err == WSAECONNABORTED)
      return 0;

    throw socket_error("fas::system::inet::recv: ");
  }
  return ret;
}
#else
inline ssize_t recv(const int d, char* buff, size_t s)
{
  ssize_t ret = ::recv(d, buff, s, 0);

  if ( ret < 0 ) 
  {
    int err = error_code();
    if (err==EWOULDBLOCK || err==EAGAIN || err == EINTR ) return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
         throw socket_error("fas::system::inet::recv: ");
    else return 0;
  }
  return ret;
}
#endif

#ifdef WIN32
inline int recvfrom(const SOCKET d, char* buff, int s, address_t& a)
{
  sockaddr_in6 addr;
  socklen_t size = sizeof(addr);

  int ret = ::recvfrom(d, buff, s, 0, (sockaddr*)&addr, &size);

  if ( ret < 0 ) 
  {
    int err = error_code();
    if (err == WSAEINTR || err == WSAEINPROGRESS || err == WSAEWOULDBLOCK 
        || err == WSAETIMEDOUT || err == WSATRY_AGAIN || err == WSA_IO_PENDING)
      return -1;
    if (err == WSAENETRESET || err == WSAESHUTDOWN || err == WSAEDISCON)
      return 0;

    throw socket_error("fas::system::inet::recvfrom: ");
  }

  a.assign((char*)&addr, (char*)&addr + size);
  return ret;
}
#else
inline ssize_t recvfrom(const int d, char* buff, size_t s, address_t& a)
{
  sockaddr_in6 addr;
  socklen_t size = sizeof(addr);

  ssize_t ret = ::recvfrom(d, buff, s, 0, (sockaddr*)&addr, &size);

  if ( ret < 0 ) 
  {
    int err = error_code();
    if (err!=EWOULDBLOCK && err!=EAGAIN && err!=ECONNREFUSED )
      throw socket_error("fas::system::inet::recvfrom: ");
  }

  a.assign((char*)&addr, (char*)&addr + size);
  return ret;
}

#endif


#ifdef WIN32
inline int send(const SOCKET d, const char* buff, int s)
{
again:
  int ret = ::send(d, buff, s, 0);
  if ( ret < 0 ) 
  {
    int err = error_code();
    if (err == WSAEINTR)
      goto again;

    if (err == WSAEINTR || err == WSAEINPROGRESS || err == WSAEWOULDBLOCK 
        || err == WSAETIMEDOUT || err == WSATRY_AGAIN || err == WSA_IO_PENDING)
      return -1;
    if (err == WSAENETRESET || err == WSAESHUTDOWN || err == WSAEDISCON
        || err == WSAECONNRESET || err == WSAECONNABORTED)
      return 0;

    throw socket_error("fas::system::inet::send: ");
  }
  return ret;
}
#else
inline ssize_t send(const int d, const char* buff, size_t s)
{
again:
  ssize_t ret = ::send(d, buff, s, 0);
  if ( ret < 0 ) 
  {
    int err = error_code();
    if (err == EINTR)
    {
      goto again;
    }

    if (err==EWOULDBLOCK || err==EAGAIN || err == EINTR ) return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
    {
         throw socket_error("fas::system::inet::send: ");
    }
    else {
      return 0;
   }
  }
  return ret;
}

#endif

#ifdef WIN32
inline int sendto(const SOCKET d, const char* buff, int s, const address_t& a) 
{
  int ret = ::sendto(d, buff, s, 0, (const sockaddr*)&(a[0]), (socklen_t)a.size() );
  if ( ret < 0 )
  {
    int err = error_code();
    if (err!=EWOULDBLOCK && err!=EAGAIN && err!=ECONNREFUSED )
      throw socket_error("fas::system::inet::sendto: ");
  }
  return ret;
}

#else

inline ssize_t sendto(const int d, const char* buff, size_t s, const address_t& a) 
{
  ssize_t ret = ::sendto(d, buff, s, 0, (const sockaddr*)&(a[0]), (socklen_t)a.size() );
  if ( ret < 0 )
  {
    int err = error_code();
    if (err!=EWOULDBLOCK && err!=EAGAIN && err!=ECONNREFUSED )
      throw socket_error("fas::system::inet::sendto: ");
  }
  return ret;
}

#endif

/* -- убрать
#ifdef WIN32

inline int read(const socket_t d, char* buff, size_t s)
{
#ifndef WIN32
  ssize_t ret = ::read(d, buff, s);
#else
  ssize_t ret = ::recv(d, buff, s, 0);
#endif
  if ( ret < 0 ) 
  {
    int err = error_code();

    if (err==EWOULDBLOCK || err==EAGAIN  ) return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
      throw socket_error("fas::system::inet::read/recv: ");
    else
      return 0;
  }
  return ret;
}

#else

inline ssize_t read(const int d, char* buff, size_t s)
{
  ssize_t ret = ::recv(d, buff, s, 0);
  if ( ret < 0 ) 
  {
    int err = error_code();

    if (err==EWOULDBLOCK || err==EAGAIN  ) return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
      throw socket_error("fas::system::inet::read/recv: ");
    else
      return 0;
  }
  return ret;
}

#endif
*/

/*
inline ssize_t write(const socket_t d, const char* buff, size_t s)
{

#ifdef WIN32 
  ssize_t ret = ::send(d, buff, s, 0);
#else
  ssize_t ret = ::write(d, buff, s);
#endif

  if ( ret < 0 ) 
  {
    int err = error_code();

    if (err==EWOULDBLOCK || err==EAGAIN  ) return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
    {
         throw socket_error("fas::system::inet::send/write: ");
    }
    else {
      return 0;
   }
  }
  return ret;
}
*/


inline void close(const socket_t d)
{
#ifndef WIN32
  if ( ::close(d) < 0 )
    throw socket_error( "fas::system::inet::close: " );
#else
  if ( ::closesocket(d) < 0 )
    throw socket_error( "fas::system::inet::close: " );
#endif
}

inline void shutdown(const socket_t& d) //throw(socket_error)
{
  if ( ::shutdown(d, SHUT_RDWR) < 0 )
    /*throw socket_error( "fas::system::inet::shutdown:" )*/ return;
}

inline int select( int nfds, fd_set* rs, fd_set* ws, fd_set* es, timeval* timeout)
{
  int n = ::select(nfds, rs, ws, es, timeout);

#ifdef WIN32
  if ( n == SOCKET_ERROR)
#else
  if ( n < 0)
#endif
  {
    int err = error_code();
    if (err == EINTR) return 0;
    else
      throw socket_error( "fas::system::inet::select:" );
  }

  return n;
}


inline void reuseaddr(const socket_t& d)
{
  int val = 1;
  if ( setsockopt((int)d, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val)))
    throw socket_error( "fas::system::inet::reuseaddr: " );
}

inline void tcp_nodelay(const socket_t& d)
{
  int val = 1;
  if ( setsockopt((int)d, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val)))
    throw socket_error( "fas::system::inet::tcp_nodelay: " );
}

/*
 * true - ok, false - in progress, throw other
 */
inline bool check_socket(const socket_t& d)
{
  int val = 0;
  socklen_t len = sizeof(val);
  if ( ::getsockopt( (int)d, SOL_SOCKET, SO_ERROR, (char*)&val, &len ) )
    throw socket_error( "fas::system::inet::check_socket1: " );

  if (val == 0) 
  {
    if ( ::send(d, 0, 0, 0) < 0 )
    {
      int err = error_code();

      if ( err==EWOULDBLOCK || err==EAGAIN  || err == EINTR)
        return false;
      if ( err == ENOTCONN )
        return false;
      else
        throw socket_error( "fas::system::inet::check_socket2: ", err );
    }
    return true;
  }


  if (val == EINPROGRESS) return false;

  if ( val==EWOULDBLOCK || val==EAGAIN  || val == EINTR)
        return true;
  throw socket_error( "fas::system::inet::check_socket3: ", val );
}

inline void nonblock(const socket_t d)
{
#ifdef WIN32
  DWORD in;
  DWORD out;
  DWORD out_size;
  int res = WSAIoctl(d, FIONBIO, &in, sizeof(in), &out, sizeof(out), &out_size, 0, 0 );

  if (res==SOCKET_ERROR)
    throw socket_error( "fas::system::inet::nonblock: " );

#else
  int val = fcntl(d, F_GETFL, 0);
  if ( val < 0 )
    throw socket_error( "fas::system::inet::nonblock: " );
  val = fcntl(d, F_SETFL, val | O_NONBLOCK );
  if ( val < 0 )
    throw socket_error( "fas::system::inet::nonblock: " );
#endif
}


inline int getsockerror(const socket_t d)
{
  int val = 0;
  socklen_t len = sizeof(val);
  if ( getsockopt((int)d, SOL_SOCKET, SO_ERROR, (char*)&val, &len))
    throw socket_error( "fas::system::inet::getsockerror: " );
  return val;
}

/** Протестировать

inline void so_rcvtimeo(const socket_t d, int msec)
{
  struct timeval tv={msec/1000,msec%1000};
  if (0 != setsockopt(d, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
    throw socket_error( "fas::system::inet::so_rcvtimeo: " );;
}

inline void so_sndtimeo(const socket_t d, int msec)
{
  struct timeval tv={msec/1000,msec%1000};
  if (0 != setsockopt(d, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)))
    throw socket_error( "fas::system::inet::so_rcvtimeo: " );;
}

*/

inline void keepalive(const socket_t d, bool set = true )
{
  int val = static_cast<int>(set);
  if ( setsockopt((int)d, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val)))
    throw socket_error( "fas::system::inet::keepalive: " );
  //return 0;
}


}}}

#endif // FAS_SYSTEM_INET_HPP
