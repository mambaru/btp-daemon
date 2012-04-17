//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_CLOCK_HPP
#define FAS_FILTERS_AD_CLOCK_HPP

#include <fas/aop/aspect.hpp>

#include <fas/system/system.hpp>

#include <fas/misc/stat.hpp>

namespace fas { namespace adv { 

namespace aa = ::fas::aop;
namespace am = ::fas::misc;

struct _clock_{};

struct _clock_start_{};
struct _clock_finish_{};
struct _span_reg_{};


/** Делает временные отметки при каждом вызове. Может использоватся
  * для замера временног интервала прохождения цепочки вызовов адвайсов
  * (для этого адвайс ad_clock ставится первым в цепочке вызовов, а
  * последний адвайс, может получить время прохождения цепочки в
  * миллисекундах (double) путем вызова метода get_timespan() .
  */
template<typename N, typename R = void>
class ad_clock_old
{
  timeval _tv;
public:

   timeval get_timeval() const { return _tv;}

   timeval get_span() const 
   {
      timeval now;
      ::gettimeofday(&now, 0);
      now.tv_sec -= ( _tv.tv_sec + (_tv.tv_usec > now.tv_usec) );
      now.tv_usec = _tv.tv_usec < now.tv_usec ? now.tv_usec - _tv.tv_usec  : _tv.tv_usec - now.tv_usec;
      return now;
   }

  /** Возвращает интервал времени в миллисекундах с момента последнего 
    * вызова этого адвайса (operator() )
    * @return интервал времени в миллисекундах
    */
  double get_timespan()
  {
    timeval now;
    ::gettimeofday(&now, 0);
    return ( now.tv_sec*1000 + double(now.tv_usec)/1000 )-
           ( _tv.tv_sec*1000 + double(_tv.tv_usec)/1000 );
  }


  /** Возвращает времени в миллисекундах с момента начала
    * отчета до вызова этого адвайса (operator() )
    *(зависит от системы и применяймого 
    * системного вызова, для time() это интервл с 1970 года).
    * @return интервал времени в миллисекундах с момента 
    * начала отсчета.
    */
  double get_time()
  {
    return _tv.tv_sec*1000 + double(_tv.tv_usec)/1000;
  }

  template<typename T>
  R operator()(T& t)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename T0>
  R operator()(T& t, T0 t0)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0);
  }

  template<typename T, typename T0, typename T1>
  R operator()(T& t, T0 t0, T1 t1)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0, t1);
  }

  template<typename T, typename T0, typename T1, typename T2>
  R operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0, t1, t2);
  }
};

/** Новая версия..
  */

/*
Доделать - достаточно одного адвайса для замера времени
template<typename N = void, typename R = void>
class ad_clock
{
public:
  template<typename T>
  R operator()(T& t)
  {
    timeval start, finish;
    ::gettimeofday(&start, 0);
    R result = t.get_aspect().template get<N>()(t);
    ::gettimeofday(&finish, 0);
    _finalize(t, start, finish);
    return result;
  }

  template<typename T, typename P0, typename P1>
  R operator()(T& t, P0 p0, P1 p1)
  {
    timeval start, finish;
    ::gettimeofday(&start, 0);
    R result = t.get_aspect().template get<N>()(t, p0, p1);
    ::gettimeofday(&finish, 0);
    _finalize(t, start, finish);
    return result;
  }


  template<typename T>
  void _finalize()(T& t, timeval start, timeval finish)
  {
    finish.tv_sec -= ( start.tv_sec + (start.tv_usec > finish.tv_usec) );
    finish.tv_usec = start.tv_usec < finish.tv_usec ? finish.tv_usec - start.tv_usec  : start.tv_usec - finish.tv_usec;
    t.get_aspect().template get<_span_reg_>().span(tv);
  }

};
*/


template<typename N = void, typename R = void>
class ad_clock_start
{
  timeval _tv;
public:

   timeval get_timeval() const { return _tv;}

   timeval get_span() const 
   {
      timeval now;
      ::gettimeofday(&now, 0);
      now.tv_sec -= ( _tv.tv_sec + (_tv.tv_usec > now.tv_usec) );
      now.tv_usec = _tv.tv_usec < now.tv_usec ? now.tv_usec - _tv.tv_usec  : _tv.tv_usec - now.tv_usec;
      return now;
   }

  template<typename T>
  R operator()(T& t)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename T0>
  R operator()(T& t, T0 t0)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0);
  }

  template<typename T, typename T0, typename T1>
  R operator()(T& t, T0 t0, T1 t1)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0, t1);
  }

  template<typename T, typename T0, typename T1, typename T2>
  R operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    ::gettimeofday(&_tv, 0);
    return t.get_aspect().template get<N>()(t, t0, t1, t2);
  }
};

template<>
class ad_clock_start<void, void>
{
  timeval _tv;
public:

   timeval get_timeval() const { return _tv;}

   timeval get_span() const 
   {
      timeval now;
      ::gettimeofday(&now, 0);
      now.tv_sec -= ( _tv.tv_sec + (_tv.tv_usec > now.tv_usec) );
      now.tv_usec = _tv.tv_usec < now.tv_usec ? now.tv_usec - _tv.tv_usec  : _tv.tv_usec - now.tv_usec;
      return now;
   }

  template<typename T>
  void operator()(T& t)
  {
    ::gettimeofday(&_tv, 0);
  }

  template<typename T, typename T0>
  void operator()(T& t, T0 t0)
  {
    ::gettimeofday(&_tv, 0);
  }

  template<typename T, typename T0, typename T1>
  void operator()(T& t, T0 t0, T1 t1)
  {
    ::gettimeofday(&_tv, 0);
  }

  template<typename T, typename T0, typename T1, typename T2>
  void operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    ::gettimeofday(&_tv, 0);
  }
};

template<typename N = void, typename R = void>
class ad_clock_finish
{
public:
  template<typename T>
  R operator()(T& t)
  {
    _finalize(t);
    return t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename T0>
  R operator()(T& t, T0 t0)
  {
    _finalize(t);
    return t.get_aspect().template get<N>()(t, t0);
  }

  template<typename T, typename T0, typename T1>
  R operator()(T& t, T0 t0, T1 t1)
  {
    _finalize(t);
    return t.get_aspect().template get<N>()(t, t0, t1);
  }

  template<typename T, typename T0, typename T1, typename T2>
  R operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    _finalize(t);
    return t.get_aspect().template get<N>()(t, t0, t1, t2);
  }

private:

  template<typename T>
  void _finalize(T& t)
  {
    timeval tv = t.get_aspect().template get<_clock_start_>().get_span();
    //t.get_aspect().template get<_span_reg_>.span(t, tv);
    t.get_aspect().template get<_span_reg_>().span(tv);
  }
};

template<>
class ad_clock_finish<void, void>
{
public:
  template<typename T>
  void operator()(T& t)
  {
    _finalize(t);
  }

  template<typename T, typename T0>
  void operator()(T& t, T0 t0)
  {
    _finalize(t);
  }

  template<typename T, typename T0, typename T1>
  void operator()(T& t, T0 t0, T1 t1)
  {
    _finalize(t);
  }

  template<typename T, typename T0, typename T1, typename T2>
  void operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    _finalize(t);
  }

private:

  template<typename T>
  void _finalize(T& t)
  {
    timeval tv = t.get_aspect().template get<_clock_start_>().get_span();
    t.get_aspect().template get<_span_reg_>().span(tv);
  }
};

class ad_span_reg_stub
{
public:
  
  void span(timeval) { }
  void mark(int) {}
//  void span(timeval) {}
};

class ad_span_reg
{
  am::common_stat* _common_stat;

public:

  ad_span_reg(): _common_stat(0) {};

  void stat_object(am::common_stat* stat) { _common_stat = stat; }

  const am::common_stat* stat_object() const { return _common_stat;}

  void mark(int i) { if (_common_stat!=0) _common_stat->mark(i); };

  void span(timeval tv) { if (_common_stat!=0) _common_stat->marked_span(tv); };

};

}}

#endif
