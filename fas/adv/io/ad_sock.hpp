//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MUX_AD_SOCK_HPP
#define FAS_MUX_AD_SOCK_HPP

#include <fas/unp.h>
#include <fas/system/system.hpp>
#include <fas/system/inet.hpp>
#include <fas/adv/io/desc_holder.hpp>

namespace fas { namespace adv { namespace io {


#ifdef WIN32

class ad_recv
  : public desc_holder<SOCKET>
{
public:
  typedef desc_holder<SOCKET> super;
  typedef super::desc_type desc_type;

  typedef int return_type;
  typedef int size_type;

  return_type operator() (char* d, size_type s )
  {
    return_type rt = fas::system::inet::recv(super::get_d(), d, s);
    if (rt == 0)
      super::set_status(false);
    return rt;
  }
};

#else

class ad_recv
  : public desc_holder<int>
{
public:
  typedef desc_holder<int> super;
  typedef super::desc_type desc_type;

  typedef ssize_t return_type;
  typedef size_t size_type;

  return_type operator() (char* d, size_type s )
  {
    return_type rt = fas::system::inet::recv(super::get_d(), d, s);
    if (rt == 0)
      super::set_status(false);
    return rt;
  }
};

#endif

////////////////////////////////////////////////

#ifdef WIN32

class ad_send
  : public desc_holder<SOCKET>
{
public:
  typedef desc_holder<SOCKET> super;
  typedef super::desc_type desc_type;

  typedef int return_type;
  typedef int size_type;

  return_type operator() (const char* d, size_type s )
  {
    return_type rt = fas::system::inet::send(super::get_d(), d, s);
    if (rt == 0)
      super::set_status(false);
    return rt;
  }
};

#else

class ad_send
  : public desc_holder<int>
{
public:
  typedef desc_holder<int> super;
  typedef super::desc_type desc_type;

  typedef ssize_t return_type;
  typedef size_t size_type;

  return_type operator() (const char* d, size_type s )
  {
    return_type rt = fas::system::inet::send(super::get_d(), d, s);
    if (rt == 0)
      super::set_status(false);
    return rt;
  }
};

#endif

}}}

#endif
