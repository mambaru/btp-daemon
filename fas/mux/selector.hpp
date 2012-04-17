//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//


#ifndef FAS_MUX_SELECTOR_H
#define FAS_MUX_SELECTOR_H

#include <fas/mux/imux.hpp>
#include <fas/mux/types.hpp>
#include <fas/mux/imux_observer.hpp>
#include <fas/system/system.hpp>

#include <vector>
#include <set>

#ifdef WIN32
#pragma warning (disable:4127)
#else 
#include <sys/select.h>
#include <sys/time.h>
#endif

#include <iostream>

namespace fas{ namespace mux {

class selector
  : public imux<descriptor_t>
{
  //bool _reset;
public:
  typedef selector self;
  typedef imux<descriptor_t>::desc_type desc_type;
  typedef imux<descriptor_t>::imux_observer imux_observer;

  typedef std::set<descriptor_t> modify_set;

  virtual ~selector(){}

  selector(): /*_reset(false),*/ _fd_size(0)
  {
    FD_ZERO(&_rs);
    FD_ZERO(&_ws);
    FD_ZERO(&_us);
  }

  imux_observer* set_rhandler(desc_type d, imux_observer* mh)
  {
    /*_reset = true;
    _modify_set.insert(d);
    std::cerr<< "set_rhandler " << int(d) << " - " << bool(mh!=0) << std::endl;*/

    if ( _rhandlers.size() < static_cast<size_t>(d + 1) )
      _rhandlers.resize(d+1, static_cast<imux_observer*>(0) );

    imux_observer* old = _rhandlers[d];
    _rhandlers[d] = mh;
    if (mh == 0)
      FD_CLR(d, &_rs);
    else
      FD_SET(d, &_rs);

    _set_fd_size(d, mh!=0);
    return old;
  }

  imux_observer* set_whandler(desc_type d, imux_observer* mh)
  {
/*    _reset = true;
    _modify_set.insert(d);
*/
    if ( _whandlers.size() < static_cast<size_t>(d + 1) )
      _whandlers.resize(d+1, static_cast<imux_observer*>(0) );

    imux_observer* old = _whandlers[d];
    _whandlers[d] = mh;
    if (mh == 0)
      FD_CLR(d, &_ws);
    else
      FD_SET(d, &_ws);

    _set_fd_size(d, mh!=0);
    return old;
  }

  imux_observer* set_uhandler(desc_type d, imux_observer* mh)
  {
    /*_reset = true;
    _modify_set.insert(d);
*/
    if ( _uhandlers.size() < static_cast<size_t>(d + 1) )
      _uhandlers.resize(d+1, static_cast<imux_observer*>(0) );

    imux_observer* old = _uhandlers[d];
    _uhandlers[d] = mh;
    if (mh == 0)
      FD_CLR(d, &_us);
    else
      FD_SET(d, &_us);

    _set_fd_size(d, mh!=0);
    return old;
  }

  void set_handlers(desc_type d, imux_observer* mh)
  {

    self::set_rhandler(d, mh);
    self::set_whandler(d, mh);
    self::set_uhandler(d, mh);
  }


  imux_observer* reset_rhandler(desc_type d)
  {
    return self::set_rhandler(d, 0);
  }

  imux_observer* reset_whandler(desc_type d)
  {
    return self::set_whandler(d, 0);
  }

  imux_observer* reset_uhandler(desc_type d)
  {
    return self::set_uhandler(d, 0);
  }

  void reset_handlers(desc_type d)
  {
    self::reset_rhandler(d);
    self::reset_whandler(d);
    self::reset_uhandler(d);
  }

  bool select(long timeout)
  {

    timeval tv={timeout/1000, (timeout%1000)*1000};

    if ( !_rhandlers.empty() ) __rs_out = _rs;
    if ( !_whandlers.empty() ) __ws_out = _ws;
    if ( !_uhandlers.empty() ) __us_out = _us;

#ifdef WIN32
	if ( _fd_size == 0 ) 
  {
    ::fas::system::sleep(timeout);
		return false;
  }
#endif

    if ( int sr = ::select(
                    _fd_size,
                    _rhandlers.empty() ? 0 : &__rs_out,
                    _whandlers.empty() ? 0 : &__ws_out,
                    _uhandlers.empty() ? 0 : &__us_out,
                    timeout < 0 ? 0 : &tv
                  )
       )
    {
      if (sr == -1)
      {
        int err = fas::system::error_code();
        if (err == EINTR)
          return 0;
        else
          throw fas::system::system_error( "fas::selector::select: " );
      }
      else
      {
        // _modify_set.clear();
        for (int i=0; i<_fd_size; i++ )
        {
          //_reset = false;
          //if ( !_reset || _modify_set.find(i)==_modify_set.end() )
          {
            if ( i < static_cast<int>( _rhandlers.size() )  && FD_ISSET(i, &__rs_out) && _rhandlers[i]!=0 )
              _rhandlers[i]->ready_read(i);
          }

          //if ( !_reset || _modify_set.find(i)==_modify_set.end() )
          {
            if ( i < static_cast<int>( _whandlers.size() ) && FD_ISSET(i, &__ws_out) && _whandlers[i]!=0 )
              _whandlers[i]->ready_write(i);
          }

          //if ( !_reset || _modify_set.find(i)==_modify_set.end() )
          {
            if ( i < static_cast<int>( _uhandlers.size() ) && FD_ISSET(i, &__us_out) && _uhandlers[i]!=0 )
              _uhandlers[i]->ready_urgent(i);
          }
          // if (_reset) break;
        }
      }

      return true;
    }
    return false;
  }

private:
  size_t _get_max_d() const
  {
    return 0;
  }

  void _set_fd_size(desc_type d, bool set)
  {
    if (set)
    {
      if ( static_cast<int>(d+1) > _fd_size)
        _fd_size = static_cast<int>(d)+1;
    }
    else
    {
      if ( static_cast<int>(d) == _fd_size - 1 )
        while ( _fd_size!=0 &&
                !FD_ISSET(_fd_size-1, &_rs) &&
                !FD_ISSET(_fd_size-1, &_ws) &&
                !FD_ISSET(_fd_size-1, &_us)
              ) _fd_size--;
    }
  }

  typedef std::vector<imux_observer*> handlers_interfaces;

  handlers_interfaces _rhandlers;
  handlers_interfaces _whandlers;
  handlers_interfaces _uhandlers;

  fd_set _rs;
  fd_set _ws;
  fd_set _us;

  fd_set __rs_out;
  fd_set __ws_out;
  fd_set __us_out;

  int _fd_size;
  modify_set _modify_set;

};

}}

#endif //MIGASHKO_INET_SELECTOR_H
