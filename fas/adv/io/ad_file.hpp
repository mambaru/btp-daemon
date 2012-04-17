//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_FILE_HPP
#define FAS_FILTERS_AD_FILE_HPP

#include <fas/unp.h>
#include <fas/system/system.hpp>
#include <fas/adv/io/desc_holder.hpp>

namespace fas { namespace adv { namespace io{

#ifdef WIN32

class ad_file_read
  : public desc_holder<HANDLE>
{
public:
  typedef desc_holder<HANDLE> super;
  typedef super::desc_type desc_type;

  typedef DWORD return_type;
  typedef DWORD size_type;

  return_type operator() (char* d, size_type s )
  {
    return_type rt = 0;
    if ( ::ReadFile(super::get_d(), d, s, &rt, 0) )
    {
      if (rt == 0) 
        super::set_status(false);
      return rt;
    }

    super::set_status(false);

    throw fas::system::system_error("ad_file_read: ");
  }
};

#else

class ad_file_read
  : public desc_holder<int>
{
public:
  typedef desc_holder<int> super;
  typedef super::desc_type desc_type;

  typedef ssize_t return_type;
  typedef size_t size_type;

  return_type operator() (char* d, size_type s )
  {
    return_type rt  = fas::system::read(super::get_d(), d, s);
    /*
    if ( rt < 0 ) 
    {
      int err = fas::system::error_code();
      if (err==EWOULDBLOCK || err==EAGAIN  ) 
        return rt;
      else if (err==EBADF || err == EFAULT || err==EINVAL || 
               err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
          throw fas::system::system_error("fas::system::read/_read: ");
      else
        return 0;
    }
    */
    if (rt == 0 )
      super::set_status(false);
    return rt;
  }
};

#endif

////////////////////////////////////////////////

#ifdef WIN32

class ad_file_write
  : public desc_holder<HANDLE>
{
public:
  typedef desc_holder<HANDLE> super;
  typedef super::desc_type desc_type;

  typedef DWORD return_type;
  typedef DWORD size_type;

  return_type operator() (const char* d, size_type s )
  {
    return_type rt = 0;
    if ( ::WriteFile(super::get_d(), d, s, &rt, 0) )
    {
      if (rt == 0) 
        super::set_status(false);
      return rt;
    }

    super::set_status(false);

    throw fas::system::system_error("ad_file_write: ");
  }
};

#else

class ad_file_write
  : public desc_holder<int>
{
public:
  typedef desc_holder<int> super;
  typedef super::desc_type desc_type;

  typedef ssize_t return_type;
  typedef size_t size_type;

  return_type operator() (const char* d, size_type s )
  {
    
    return_type rt  = fas::system::write( super::get_d(), d, s );

    /*
    return_type ret = ::write(super::get_d(), d, s);
    if ( ret < 0 ) 
    {
        int err = fas::system::error_code();
        if ( err==EWOULDBLOCK || err==EAGAIN )
          return ret;
        else if (err==EBADF || err == EFAULT || err==EINVAL ||
             err == ENOMEM ||  err == ENOTCONN || err == ENOTSOCK)
        {
          throw fas::system::system_error("fas::system::_write/write: ");
        }
        else
          return 0;
    }*/

    if (rt == 0 )
      super::set_status(false);

    return rt;
  }
};

#endif

#endif

}}}
