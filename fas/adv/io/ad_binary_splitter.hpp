//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_BINARY_SPLITTER_HPP
#define FAS_FILTERS_AD_BINARY_SPLITTER_HPP

#include <vector>
#include <iostream>

namespace fas { namespace adv { namespace io{

template< char ch0=0, char ch1=0, char ch2=0, char ch3=0, char ch4=0,
          char ch5=0, char ch6=0, char ch7=0, char ch8=0, char ch9=0>
struct sep;


/** Разбивает бинарный поток на датаграмы по признаку определенному пользователем и
  * передает их на адвайс с указанным тегом. Поток передается адвайсу блоками
  * произвольного размера. Если в блоке обнаруженно несколько датаграм, то каждая
  * и зних передается адвайсу N отдельно. Если в переданном блоке последняя датаграмма
  * не содержит признак-разделитель, то предполагается, что оставшаяся часть датаграммы 
  * придет со следующим блоком, а текущая копируется во внутренний буфер. При использовании
  * объекта на новом потоке, внутренний буфер необходимо очистить.
  * @param S - некоторый тип, для которого определен operator()(const char*) и
               S::size - число байт разделителя
  * @param N - тег адвайса, которому будут передаватся датаграммы
  * @advice N(const char*, size_t)
  * @see sep sep
  */
template<typename S, typename N>
class ad_binary_splitter
{
  static const size_t size_sep = S::size;

  typedef std::vector<char> data_type;
  typedef data_type::size_type size_type;
  S _sep;
  data_type _data;
  bool _clear_flag;
public:

  ad_binary_splitter():_sep(){}

  data_type::size_type size() const { return _data.size(); }

  /** Сбрасывает внутренний буфер */
  void clear()
  {
    _clear_flag = true;
    _data.clear(); 
  };

  /** Сбрасывает внутренний буфер. Для адвайсов типа ad_cleaner.
    * @see ad_cleaner ad_cleaner*/
  template<typename T>
  void clear(T& t)
  {
    this->clear();
  }

  /** Обрабатывает переданный блок и делит его на датаграммы.
    * @param d - указатель на блок данных потока
    * @param s - размер переданного блока данных
    * @return сумарное число байт переданных адвайсу N (м.б. больше s, 
              если во внутреннем буфере аставались данные с предыдущего вызова)
              и -1 если не было выделенно ни одной датаграммы
   */
  template<typename T>
  typename T::return_type operator()(T& t, const char* d, typename T::size_type s)
  {
    _clear_flag = false;
    data_type::size_type offset = 0;
    if ( _data.empty() )
    {
      if (_data.capacity() > 0 )
        data_type(_data).swap(_data);
      offset = _parse_(t, d, s, 0, 0);
      if (!_clear_flag)
      {
        if (offset == static_cast<data_type::size_type>(-1))
          _assign(d, d + s);
        else if ( static_cast<typename T::size_type>(offset) != s)
          _assign(d + offset, d + s);
      }
    }
    else
    {
      const size_t datasize = _data.size();
      if (_data.capacity() < datasize + s) {
    	  _data.reserve( datasize < 16384 ? datasize + s : datasize+(datasize>>1)+s);
      }
      std::copy(d, d + s, std::back_inserter(_data));
      offset = _parse_(t, &(_data[0]), _data.size(), 0, datasize>size_sep ? datasize-size_sep : 0);
      if (!_clear_flag)
      {
        if ( offset == static_cast<typename T::size_type>( _data.size() ) )
          _data.clear();
        else if ( offset!= static_cast<data_type::size_type>(-1) )
          _data.erase( _data.begin(), _data.begin() + offset );
      }
    }
    return static_cast<typename T::return_type>(offset);
  }

private:

  template<typename T>
  size_type _parse_(T& t, const char* buffer, size_type size, size_type offset, size_type offset_start)
  {
    if ( size < size_sep )
      return -1;

    bool flag = false;
    for (size_type i = offset_start, s = size - size_sep + 1 ; i < s ; ++i)
    {
      if ( _sep( buffer + i ) )
      {
        _send_(t,
          buffer + offset,
          static_cast<typename T::size_type>(i + size_sep - offset)
        );
        // Если во время _send_ был вызван clear
        if ( _clear_flag )
          return 0;
        offset = i + size_sep;
        flag = true;
      }
    }
    return flag ? offset: -1;
  }

  template<typename T>
  void _send_(T& t, const char* d, size_type s)
  {
    t.get_aspect().template get<N>()(t, d, s);
  }

  void _assign(const char* beg, const char* end)
  {
    _data.assign( beg, end);
  }

};



template<char ch0>
struct sep<ch0, 0,0,0,0,0,0,0,0,0>
{
  enum { size = 1 };
  bool operator () (const char* d) const 
  { return *d == ch0;}
};

template<char ch0, char ch1>
struct sep<ch0, ch1,0,0,0,0,0,0,0,0>
{
  enum { size = 2 };
  bool operator () (const char* d) const 
  { return d[0] == ch0 && d[1] == ch1;}
};

template<char ch0, char ch1, char ch2>
struct sep<ch0, ch1, ch2, 0,0,0,0,0,0,0>
{
  enum { size = 3 };
  bool operator () (const char* d) const
  { return d[0] == ch0 && d[1] == ch1 && d[2] == ch2;}
};

template<char ch0, char ch1, char ch2, char ch3>
struct sep<ch0, ch1, ch2, ch3,0,0,0,0,0,0>
{
  enum { size = 4 };
  bool operator () (const char* d) const
  { return d[0] == ch0 && d[1] == ch1 && d[2] == ch2 && d[3] == ch3;}
};

template<char ch0, char ch1, char ch2, char ch3, char ch4>
struct sep<ch0, ch1, ch2, ch3, ch4, 0, 0, 0, 0, 0>
{
  enum { size = 5 };
  bool operator () (const char* d) const
  { return d[0] == ch0 && d[1] == ch1 && d[2] == ch2 && d[3] == ch3 && d[4] == ch4;}
};

}}}
#endif
