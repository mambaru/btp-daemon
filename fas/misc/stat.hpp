#ifndef FAS_MISC_STAT_HPP
#define FAS_MISC_STAT_HPP

#include <fas/system/system.hpp>
#include <limits.h>
#include <vector>

namespace fas{ namespace misc{ 

inline bool operator < ( const timeval left,  const timeval right)
{
  if ( left.tv_sec < right.tv_sec ) 
    return true;
  if ( left.tv_sec == right.tv_sec )
    return left.tv_usec < right.tv_usec;
  return false;
}

// Статистика по одной команде (вызова метода)
class command_stat
{
public:
  command_stat()
    : _count(0)
  {
    this->clear();
  }

  void clear()
  {
    _count = 0;
    timeval init={0,0};
    last_span = init;      // Последний интервал
    max_span = init;       // Максимальный интервал
    min_span = init;       // Минимальный интервал
    average_span = init;   // Средний интервал
    sum_span = init;       // суммарный интервал
    min_span.tv_sec = LONG_MAX;
    min_span.tv_usec = LONG_MAX;
    _middle_count = 0;
    _middle_rate = init;
    ::gettimeofday(&_middle_start, 0);
    _middle_max = 2;
  }

  void span(timeval tv)
  {
    if ( _middle_count == _middle_max )
    {
      timeval now;
      ::gettimeofday(&now, 0);
      now.tv_sec -= _middle_start.tv_sec;
      if ( now.tv_usec >= _middle_start.tv_usec )
        now.tv_usec -= _middle_start.tv_usec;
      else
      {
        now.tv_usec = 1000000 + now.tv_usec - _middle_start.tv_usec ;
        now.tv_sec -= 1;
      }
      _correct(now);
      // в now время исполнения _middle_max операций
      // средняя (реальная) скорость выпонения для _middle_max операций в сек

      _middle_rate = rate(now);

      // средняя (реальная) скорость выпонения для 1 операции (надо разделить на _middle_max)

      // скорость выполнения _middle_max/1000 операций 
      long rate_1000 = ( _middle_rate.tv_sec*1000 + _middle_rate.tv_usec/1000 );
      rate_1000 /= _middle_max;
      _middle_rate.tv_sec = rate_1000/1000;
      _middle_rate.tv_usec = ( rate_1000 - _middle_rate.tv_sec*1000 ) * 1000;


      _middle_count = 0;
      ::gettimeofday(&_middle_start, 0);

      // корректируем _middle_max
      if ( now.tv_sec > 0 )
        _middle_max /= 2;
      else
        _middle_max *= 2;

      if ( _middle_max < 2 )
        _middle_max = 2;

    }
    else
      _middle_count++;


    ++_count;
    _correct(tv);

    last_span = tv;

    if ( max_span < tv ) max_span = tv;
    if ( tv < min_span ) min_span = tv;

    if ( average_span.tv_sec == 0 && average_span.tv_usec == 0 )
    {
      average_span = tv;
    }
    else
    {
      average_span.tv_sec += tv.tv_sec;
      average_span.tv_sec /= 2;

      average_span.tv_usec += tv.tv_usec;
      average_span.tv_usec /= 2;
    }

    _correct(average_span);

    sum_span.tv_sec += tv.tv_sec;
    sum_span.tv_usec += tv.tv_usec;
    _correct(sum_span);
  }

  timeval get_last() const    { return last_span; }      // Последний интервал
  timeval get_max() const     { return max_span; }       // Максимальный интервал
  timeval get_min() const     
  {
    return min_span.tv_sec != LONG_MAX ? min_span : timeval(); 
  }       // Минимальный интервал
  timeval get_average() const { return average_span; }   // Средний интервал
  timeval get_sum() const     { return sum_span; }       // суммарный интервал

  timeval get_middle_rate() const { return _middle_rate; }
  long get_middle_max() const { return _middle_max; }

  long count() const { return _count; }
  // Скорость исполнения (опреаций в секунду)
  // tv_seс - целая часть
  // tv_useс - дробная часть часть (милионная)
  // 0;500000 => 0;2
  static timeval rate(const timeval& tv)
  {
    if (tv.tv_sec == 0 && tv.tv_usec == 0) 
      return timeval();

    timeval result;
    long rate_1000 = 1000000000/(tv.tv_sec*1000000 + tv.tv_usec);
    result.tv_sec = rate_1000 / 1000;
    result.tv_usec = (rate_1000 - result.tv_sec *1000) * 1000;
    return result;
  }

private:

  void _correct(timeval& tv) const
  {
    while ( tv.tv_usec >= 1000000 )
    {
      ++tv.tv_sec;
      tv.tv_usec -= 1000000;
    }
  }

private:
  timeval last_span;      // Последний интервал
  timeval max_span;       // Максимальный интервал
  timeval min_span;       // Минимальный интервал
  timeval average_span;   // Средний интервал
  timeval sum_span;       // сумма интервалов интервал
  long _count;            // Общий счетчик

  long _middle_count;     // Счетчик расчета средней скорости
  long _middle_max;       // Максимальное значение _middle_count (адаптивно корректируется, изначально 2)
  timeval _middle_start;  // Время начала расчета средней скорости
  timeval _middle_rate;   // Текущая скорость после сброса _middle_count ()
};

class common_stat
{

public:

  typedef std::vector<command_stat> stat_list;

  common_stat(): _imark(-1) {}

  command_stat& operator[]( int i )
  {
    if (i >= int(_stat_list.size()))
      _stat_list.resize(i+1);
    return _stat_list[i];
  }

  command_stat operator[]( int i ) const 
  {
    if (i >= int(_stat_list.size()))
      return command_stat();
    return _stat_list[i];
  }

  const stat_list&  get_stat_list( ) const
  {
    return _stat_list;
  }

  void clear( )
  {
    _imark = -1;
    _stat_list.clear();
  }

  void mark(int i) { _imark = i; }
  bool marked_span( timeval tv ) 
  {
    if ( _imark ==-1 )
      return false;
    (*this)[_imark].span(tv);
    _imark = -1;
    return true;
  }

private:
  stat_list _stat_list;
  int _imark;
};

}}

#endif
