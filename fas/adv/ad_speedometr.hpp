//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_STATISTICS_HPP
#define FAS_FILTERS_AD_STATISTICS_HPP

#include <algorithm>

#include <sys/time.h>
#include <stdexcept>

namespace fas { namespace adv { 


/** @param T - тип времени
  * @param M == 1000 для милисекунд, 1000000 для микросекунд 
  * @param R - счетчик для расчета реальной скорости
  */
template<typename T = double, int M = 1000, int R = 10, int V = 100 >
class command_rate
{
public:

  typedef T time_type;

  command_rate()
    : _start_time(0)
    , _last_time(0)
    , _rate_count(0)
    , _total_count(0)
    , _average_rate(0)
  {}

  void clear()
  {
    _start_time = 0;
    _last_time = 0;
    _rate_count = 0;
    _total_count = 0;
    _average_rate = 0;
  }
  
  double get_rate() const
  {
    if ( _last_time > _start_time )
      return double(_rate_count*M) / ( _last_time - _start_time );
    return 0;
  }

  double get_average_rate() const
  {
    return _average_rate;
  }

  long get_count() const
  {
    return _total_count;
  }

  void calc_rate(time_type t)
  {
    if ( _start_time == 0 )
      _start_time = t;

    _last_time = t;
    ++_rate_count;

    if ( _rate_count == R )
    {
      _rate_count /= 2;
      _start_time = _last_time - ( _last_time - _start_time )/2;
    }
  }

  void calc_average(time_type span)
  {
    ++_total_count;
    if ( _total_count%V == 0 )
      _average_rate = double(M)/span;
    else
    {
      _average_rate += double(M)/span;
      _average_rate /= 2;
    }
  }

  void calc(time_type t, time_type span)
  {
    this->calc_rate(t);
    this->calc_average(span);
  }
  
private:
  time_type _start_time;
  time_type _last_time;
  long _rate_count;
  long _total_count;
  double _average_rate;
};

template<typename S, int M>
class ad_speedometr
{
  typedef S speedometr;
  typedef typename speedometr::time_type time_type;

  speedometr _common;
  speedometr _command[M];
public:

  template<typename T>
  void clear(T&)
  {
    _common.clear();
    std::for_each(_command, _command + M, std::mem_fun_ref<void, &speedometr::clear, void>() );
  }

  double get_rate() const
  {
    return _common.get_rate();
  }

  double get_average_rate() const
  {
    return _common.get_average_rate();
  }

  void calc_rate(time_type t)
  {
    _common.calc_rate(t);
  }

  void calc_average(time_type span)
  {
    _common.calc_average(span);
  }

  void calc(time_type t, time_type span)
  {
    _common.calc(t, span);
  }

  long get_count() const
  {
    return _common.get_count();
  }

  speedometr& operator[](int i)
  {
    if ( i >= M )
      throw std::out_of_range();
    return _command[i];
  }

  const speedometr& operator[](int i) const
  {
    if ( i >= M )
      throw std::out_of_range("ad_speedometr<typename, int>");
    return _command[i];
  }

  void calc(int i, time_type t, time_type span)
  {
    if ( i >= M )
      throw std::out_of_range("ad_speedometr<typename, int>");
    _common.calc(t, span);
    _command[i].calc(t, span);
  }
};


}}

#endif
