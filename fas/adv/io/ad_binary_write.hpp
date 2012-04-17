//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_BINARY_WRITE_HPP
#define FAS_FILTERS_AD_BINARY_WRITE_HPP

#include <fas/adv/io/desc_holder.hpp>
#include <fas/system/system.hpp>
#include <fas/system/inet.hpp>

#include <vector>

namespace fas { namespace adv { namespace io{



/** Адвайс-класс осуществляет запись данных из пользовательского буфера.
  * @param N - тег адвайса, который вызывается при успешной записи данных
  * @param C - тег адвайса, который вызывается при попытке записи в
  *            закрый источник
  * @param E - тег адвайса, который вызывается если при попытке записи данных
  *            произошла ошибка
  * @param D - дескриптор источник
  *
  * @advice N(char*, size_t)
  * @advice C()
  * @advice E()
  */
template<typename W, typename N, typename C, typename E>
class ad_binary_write
{
public:

  typedef W write_advice_tag;

  /** Пишет данные из пользовательского буфера
    * @param d - указатель на буфер данных.
    * @param s - размер буфера
    * @return число записанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  template<typename T>
  typename T::return_type operator()(T& t, const char* d, typename T::write_size_type s)
  {
    typedef typename T::write_return_type return_type;
    typedef typename T::write_size_type size_type;

    // if ( s > 60 ) s = 60; //!!! убрать это для отладки!

    return_type rs = t.get_aspect().template get<W>()(d, s);

    /*if (rs <= 0)
       throw ::fas::system::inet::socket_error("ad_binary_write: ");*/

/*
    if (rs > 0)
       std::cout<<"WRITE TRACE["<<std::string(d, d + rs)<<"] == "<<rs<<std::endl;
    else 
       std::cout<<"!!!NO WRITE TRACE["<<std::string(d, d + s)<<"] == "<<rs<<std::endl;
*/





    if (rs == 0)
      t.get_aspect().template get<C>()(t/*, super::get_d()*/);
    else if (rs > 0)
      t.get_aspect().template get<N>()(t, d, rs);
    else 
      t.get_aspect().template get<E>()(t/*, super::get_d()*/);
    return rs;
  }


  /** Заглушка. Требуется для binary_filter */
  void clear(){ }
};

/** Адвайс-класс осуществляет запись и буферизацию данных. Используется
  * в тех случаях когда операция записи м.б. выполнена не полностью (напимер
  * при использовании неблокируемого ввода/вывода).
  * Примерный алгоритм использования данного адвайса
  *   buf_binary_write(const char*, size_t);
  *    while (buf_binary_write.ready() )
  *      buf_binary_write();
  * Предупреждение: так как при каждом вызове 
  *                 buf_binary_write(const char*, size_t) все не записанные 
  *                 данные копируются в буфер, при неэффективной работе системы
  *                 ввода/вывода буфер может разрастаться до больших размеров,
                    если несвоевременно производить вызовы buf_binary_write().
  *                 Однако при блокируемом вводе/выводе (в этом случае гарантирована 
  *                 запись всех данных) буферизация не производится. Это относится
  *                 и ко всем случаям когда за одну операцию производится запись всех
  *                 данных.
  * @param N - тег адвайса, который вызывается при успешной записи данных
  * @param C - тег адвайса, который вызывается при попытке записи в
  *            закрый источник
  * @param E - тег адвайса, который вызывается если при попытке записи данных
  *            произошла ошибка
  * @param D - дескриптор источника
  *
  * @advice N(char*, size_t)
  * @advice C(D)
  * @advice E(D)
  */
template<typename W, typename N, typename C, typename E, size_t S = 0, size_t MAX_CAPACITY=16384>
class ad_buf_binary_write:
  public ad_binary_write< W, N, C, E >
{
public:
  typedef ad_binary_write< W, N, C, E> super;
  typedef std::vector<char> buffer_type;

  /** Очищает внутренний буффер */
  void clear()
  {
    _buffer.clear();
    if ( _buffer.capacity() > MAX_CAPACITY )
      buffer_type().swap(_buffer);
  }

  /** Флаг готовности буфера для записи. 
    * В буфере есть данные предназначенные для записи и
    * их можно записать */
//#warning !!!
  bool ready() const { return /*super::get_status() &&*/ !_buffer.empty(); }

  /** Пишет данные из пользовательского буфера. Если не все данные 
    * были записаны за одну операцию, то оставшиеся данные копируются
    * во внутренний буфер.
    * @param d - указатель на буфер данных.
    * @param s - размер буфера
    * @return число записанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  template<typename T>
  typename T::write_return_type operator()(T& t, const char* d, typename T::write_size_type s)
  {
    typedef typename T::write_return_type return_type;
    typedef typename T::write_size_type size_type;


    if ( !_buffer.empty() || s < S )
    {
      _buffer.reserve( _buffer.size() + s > 4096 ? s : 4096 );
      std::copy( d, d + s, std::back_inserter(_buffer) );
    }

    return_type rt = 0; // TODO: сделать по умолчанию -1 
    if ( _buffer.empty() )
    {
      if (_buffer.capacity() > 0 )
        buffer_type(_buffer).swap(_buffer);

      rt = super::operator()(t, d, s );
      if ( rt > 0 && rt < static_cast<return_type>(s) )
        _buffer.assign( d + rt, d + s);
      else if ( rt < 0 )
        _buffer.assign( d, d + s);
    }
    else if ( _buffer.size() > S )
    {
      rt = super::operator()(t, &(_buffer[0]), static_cast<size_type>(_buffer.size()) );
      if ( rt == static_cast<return_type>( _buffer.size() ) )
        _buffer.clear();
      else if ( rt > 0 )
        _buffer.erase( _buffer.begin(), _buffer.begin() + rt );
      else if ( rt == 0)
        _buffer.clear(); // дескриптор закрыт
    }
    else
      rt = S;

    return rt;
  }

  /** Записывает оставшиеся в буфере данные.
    * @return число записанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
   */
  template<typename T>
  typename T::return_type operator()(T& t)
  {
    typedef typename T::write_return_type return_type;
    typedef typename T::write_size_type size_type;

    if ( _buffer.empty() )
    {
      if (_buffer.capacity() > 0 )
        buffer_type(_buffer).swap(_buffer);

      // return 0; // TODO: источник то не закрыт, вернуть -1
      return -1;
    }

    return_type rt = super::operator()(t, &(_buffer[0]), static_cast<size_type>(_buffer.size() ) );
    if ( rt == static_cast<return_type>(_buffer.size()) )
      _buffer.clear();
    else if ( rt > 0 )
      _buffer.erase( _buffer.begin(), _buffer.begin() + rt );

    return rt;
  }

private:
  buffer_type _buffer;
};

}}}

#endif
