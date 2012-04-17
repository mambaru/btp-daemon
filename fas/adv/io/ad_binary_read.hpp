//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_BINARY_READ_HPP
#define FAS_FILTERS_AD_BINARY_READ_HPP

/*
#include <fas/filter/desc_holder.hpp>
#include <fas/system/system.hpp>
*/

#include <vector>



namespace fas { namespace adv { namespace io{


/** Адвайс-класс осуществляет чтение данных в пользовательский буффер.
  * @param N - тег адвайса, который вызывается при успешном чтении данных
  * @param C - тег адвайса, который вызывается при попытке чтения из 
  *            закрытого источника
  * @param E - тег адвайса, который вызывается если при попытке чтения данных
  *            произошла ошибка
  * @param D - дескриптор источника
  *
  * @advice N(char*, size_t) - при успешном чтении данных
  * @advice C(D) - при попытке прочитать из закрытого источника
  * @advice E(D) - при ошибке чтения
  */
template<typename R, typename N, typename C, typename E>
class ad_binary_read
{
public:

  typedef R read_advice_tag;

  /** Читает данные в пользовательский буффер
    * @param d - указатель на буффер данных, куда будут записаны 
    *            полученные данные.
    * @param s - размер буфера
    * @return число прочитанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  template<typename T>
  typename T::read_return_type operator()(T& t, char* d, typename T::read_size_type s)
  {
    typename T::read_return_type rs = t.get_aspect().template get<R>()(d, s);

    /*
    std::cout<<"READ TRACE rs="<<rs<<std::endl;
  */
/*
    if (rs > 0)
       std::cout<<"READ TRACE["<<std::string(d, d + rs)<<"]["<<d[0]<<"]"<<std::endl;
*/



    if (rs==0)
      t.get_aspect().template get<C>()(t/* не надо, узнаем из контекста, super::get_d()*/);
    else if (rs > 0)
      t.get_aspect().template get<N>()(t, d, rs);
    else
      t.get_aspect().template get<E>()(t/*, super::get_d()*/);
    // std::cout<<"READ TRACE END"<<std::endl;
    return rs;
  }

  /** Заглушка. Требуется для binary_filter */
  void clear(){}
};

/** Адвайс-класс осуществляет буферизированное чтение данных.
  * @param N - тег адвайса, который вызывается при успешном чтении данных
  * @param C - тег адвайса, который вызывается при попытке чтения из 
  *            закрытого источника
  * @param E - тег адвайса, который вызывается если при попытке чтения данных
  *            произошла ошибка
  * @param D - дескриптор источника
  * @param S - размер внутреннего буффера
  *
  * @advice N(char*, size_t) - при успешном чтении данных
  * @advice C(D) - при попытке прочитать из закрытого источника
  * @advice E(D) - при ошибке чтения
  */
template<typename R, typename N, typename C, typename E, size_t S = 4096/*, size_t MAX_CAPACITY=16384*/>
class ad_buf_binary_read
  : public ad_binary_read<R, N, C, E>
{
public:
  typedef ad_binary_read< R, N, C, E > super;
  typedef std::vector<char> buffer_type;

  ad_buf_binary_read()
    : _buffer_lock(false)
    , _buffer_clear(false) 
  {}

  /** Очищает внутренний буффер */
  void clear()
  {
    if ( !_buffer_lock )
    {
      _buffer.clear();
      _buffer_clear = false;
    }
    else
      _buffer_clear = true;

    /* не имеет смысла, буфер всегда равен S
    if ( _buffer.capacity() > MAX_CAPACITY )
      buffer_type().swap(_buffer);*/
  }

  /** Копирует данные из внутреннего буфера в пользовательский буффер.
    * Если внутренний буффер пуст, то делатся попытка чтения S байт
    * во внутренний буффер. Следует иметь ввиду что адвайс N вызывается
    * только после успешного чтения данных в внутренний буфер, а при
    * копировании данных в пользоваетльский буффер этот  вызов не
    * производится.
    * @param d - указатель на буффер данных, куда будут записаны 
    *            полученные данные.
    * @param s - размер буфера
    * @return число прочитанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    * TODO: Залочить буффер
    */
  template<typename T>
  typename T::read_return_type operator()(T& t, char* d, typename T::read_size_type s)
  {
    typedef typename T::read_return_type return_type;
    typedef typename T::read_size_type size_type;

    if ( _buffer.size() >= s )
    {
      std::copy(_buffer.begin(), _buffer.begin() + s, d );
      _buffer.erase( _buffer.begin(), _buffer.begin() + s );
      return static_cast<return_type>(s);
    }

    if ( _buffer.empty() )
    {
      if ( s > S)
        return super::operator()(t, d, s);
      else
      {
        _buffer.resize(S);
        return_type rs = super::operator()(t, &_buffer[0], static_cast<size_type>(_buffer.size()));
        if ( rs==static_cast<return_type>(-1) )
          _buffer.clear();
        else
        {
          if (rs >= 0 )
            _buffer.resize(rs);
          if (rs > static_cast<return_type>(s) ) 
            rs = static_cast<return_type>(s);
          std::copy(_buffer.begin(), _buffer.begin() + rs, d );
          _buffer.erase( _buffer.begin(), _buffer.begin() + rs );
        }
        return rs;
      }
    }
    else
    {
      return_type rs = 0;
      std::copy(_buffer.begin(), _buffer.end(), d);
      rs = static_cast<return_type>(_buffer.size());
      _buffer.clear();
      return_type rrs = super::operator()(t, d + rs, s - rs);
      if ( rs!=static_cast<return_type>(-1) && rrs >= 0)
        rs += rrs;
      return rs;
    }
//    return -1;
  }

  /** Читает данные во внутренний буффер. Если в буфере на 
    * момент выполнения данной операции были данные, то они теряются.
    * Данная операции имеет смысл при использовании совместно
    * с адвайсом N(const char*, size_t) которому передаются
    * прочитанные данные.
    * @return число прочитанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  template<typename T>
  typename T::return_type operator()(T& t)
  {
    typedef typename T::read_size_type size_type;
    _buffer.resize(S);
    _lock_buffer();

    size_type rt = -1;
    try
    {
      rt =  super::operator()(t, &_buffer[0], static_cast<size_type>(_buffer.size()));
    }
    catch(...)
    {
      _unlock_buffer();
      throw;
    }
    _unlock_buffer();
    return rt;
  }

private:

  void _lock_buffer()
  {
    _buffer_lock = true;
  }

  void _unlock_buffer()
  {
    if ( _buffer_lock && _buffer_clear)
      _buffer.clear();
    _buffer_lock = false;
    _buffer_clear = false;
  }

private:

  buffer_type _buffer;
  bool _buffer_lock;
  bool _buffer_clear;
};

}}}

#endif
