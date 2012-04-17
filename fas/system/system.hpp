//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_SYSTEM_SYSTEM_H
#define FAS_SYSTEM_SYSTEM_H

#include <fas/unp.h>
#include <fas/system/types.hpp>

#include <errno.h>
#include <string>
#include <cstdlib>
#include <stdexcept>



namespace fas{ namespace system {

inline int error_code()
{
#ifdef WIN32
  return ::GetLastError();
#else
  return errno;
#endif
}

inline std::string strerror(int lasterror)
{
#ifdef WIN32
  LPVOID lpMsgBuf;
  FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    lasterror,
    0, // Default language
    (LPSTR) &lpMsgBuf,
    0,
    NULL 
  );

  char errbuf[256] = {0};
  _snprintf(errbuf, 255, "%d - %s", lasterror, reinterpret_cast<char*>(lpMsgBuf));
  LocalFree( lpMsgBuf );
  return errbuf;
  /*
  std::string message;
  message = reinterpret_cast<char*>(lpMsgBuf);
  LocalFree( lpMsgBuf );
  return message;
  */
#else
  return ::strerror(lasterror);
#endif
}

struct system_error
  : public std::runtime_error
{
  explicit system_error(const std::string& msg)
    : std::runtime_error(msg + strerror(error_code()))
  {
  }
};

inline ssize_t read(const descriptor_t& d, char* buff, size_t s)
{
#ifndef WIN32
  ssize_t ret = ::read(d, buff, s);
#else
  ssize_t ret = ::_read(d, buff, static_cast<unsigned int>(s));
#endif
  if ( ret < 0 ) 
  {
#ifndef WIN32
    int err = error_code();
    if (err==EWOULDBLOCK || err==EAGAIN || err == EINTR ) 
      return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL || 
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
      throw system_error("fas::system::read/_read: ");
    else
      return 0;
#endif
    throw system_error("fas::system::read/_read: ");
  }
  return ret;
}


inline ssize_t write(const descriptor_t& d, const char* buff, size_t s)
{
#ifndef WIN32 
  ssize_t ret = ::write(d, buff, s);
#else
  ssize_t ret = ::_write(d, buff, static_cast<unsigned int>(s) );
#endif

  if ( ret < 0 ) 
  {
#ifndef WIN32
    int err = error_code();
    if ( err==EWOULDBLOCK || err==EAGAIN || err == EINTR )return ret;
    else if (err==EBADF || err == EFAULT || err==EINVAL ||
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
    {
      throw system_error("fas::system::_write/write: ");
    }
    else
      return 0;
#endif
      throw system_error("fas::system::write/_write: ");
  }
  return ret;
}

inline void close(const descriptor_t& d)
{
#ifdef WIN32 
  if ( -1 == ::_close(d))
#else
  if ( -1 == ::close(d))
#endif
    throw system_error("fas::system::close: ");;
}

inline void sleep(int ms)
{
#ifdef WIN32 
  ::Sleep(ms);
#else
  timeval tv={ms/1000, (ms%1000)*1000};
  ::select(0, 0, 0, 0, &tv);
#endif
}

inline int dumpable()
{ 
#if HAVE_SYS_PRCTL_H 
  rlimit core = { RLIM_INFINITY, RLIM_INFINITY };
  return ::prctl(PR_SET_DUMPABLE, 1) || ::setrlimit(RLIMIT_CORE, &core) ? -1 : 0;
#endif
  return -1;
}

inline void daemonize()
{
#ifdef WIN32 
  return ;
#else
  int null = ::open("/dev/null", O_RDWR);
  if(-1 == null)
  {
    ::perror("/dev/null");
    ::exit(EXIT_FAILURE);
  }

  switch(::fork())
  {
  case 0:
    ::setsid();
    ::umask(0);
    ::close(0);
    ::close(1);
    ::close(2);
    ::dup2(null, 0);
    ::dup2(null, 1);
    ::dup2(null, 2);
    break;

  case -1: 
    ::perror("fork()");
    ::exit(EXIT_FAILURE);

  default: 
    ::exit(EXIT_SUCCESS);
  }
#endif
}


}}
#endif // FAS_SYSTEM_SYSTEM_H
