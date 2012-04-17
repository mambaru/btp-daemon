//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_BINARY_FILTER_HPP
#define FAS_FILTERS_BINARY_FILTER_HPP

#include <fas/filter/basic_filter.hpp>
#include <fas/adv/io/ad_binary_read.hpp>
#include <fas/adv/io/ad_binary_write.hpp>
#include <fas/adv/io/ad_file.hpp>
#include <fas/adv/ad_cleaner.hpp>
#include <fas/adv/ad_stub.hpp>
#include <fas/aop/aspect.hpp>
#include <fas/aop/advice.hpp>
#include <fas/pattern/type_list.hpp>

namespace fas { namespace filter {

namespace ad = ::fas::adv;
namespace adio = ::fas::adv::io;
namespace aa = ::fas::aop;
namespace af = ::fas::filter;
namespace ap = ::fas::pattern;

struct _on_read_{};
struct _on_writing_{};
struct _on_write_{};
struct _on_assign_{};
struct _on_release_{};
struct _on_rclosed_{};
struct _on_wclosed_{};
struct _on_rerror_{};
struct _on_werror_{};

struct _read_policy_{};
struct _write_policy_{};

struct binary_on_tag_list: 
  aa::tag_list_n< 
    _on_read_,
    _on_writing_,
    _on_write_,
    _on_assign_,
    _on_release_,
    _on_rclosed_,
    _on_wclosed_,
    _on_rerror_,
    _on_werror_
>::type{};

struct on_binary_stub: aa::advice< binary_on_tag_list, ad::ad_stub<> > {};

struct file_read_advice
  : aa::advice<
      aa::tag<_read_>, 
      adio::ad_file_read
    >
{};

struct file_write_advice
  : aa::advice<
      aa::tag<_write_>, 
      adio::ad_file_write
    >
{};

namespace detail
{
  struct ad_binary_read: adio::ad_buf_binary_read< _read_, _on_read_, _on_rclosed_, _on_rerror_> {};
  struct ad_binary_write: adio::ad_buf_binary_write<_write_, _on_write_, _on_wclosed_, _on_werror_> {};
}

struct binary_read_advice
  : aa::advice<
      aa::tag<_read_policy_>,
      detail::ad_binary_read
      //adio::ad_buf_binary_read< _read_, _on_read_, _on_rclosed_, _on_rerror_> 
    >
{};

struct binary_write_advice
  : aa::advice<
      aa::tag<_write_policy_>,
      detail::ad_binary_write
      // adio::ad_buf_binary_write<_write_, _on_writing_, _on_write_, _on_wclosed_, _on_werror_> 
    >
{};

struct _cleaner_advice_{};  // TODO:  периименовать
// struct _cleaner_{};

struct cleaner_advice
  : aa::advice<
      aa::tag<_cleaner_advice_>,
      ad::ad_cleaner<ad::_cleaner_>
    >
{};



typedef ap::type_list_n<
          cleaner_advice,
          binary_read_advice,
          binary_write_advice,
          file_read_advice,
          file_write_advice,
          on_binary_stub
        >::type _binary_filter_advice_list;

struct binary_filter_advice_list : _binary_filter_advice_list{};

struct binary_filter_aspect: aa::aspect< binary_filter_advice_list >{};

// struct default_basic_filter: basic_filter<  binary_filter_aspect, _binary_read_, _binary_write_>{};

template<typename A>
struct make_binary_filter_super_class
{
  typedef typename aa::aspect_merge< 
                     binary_filter_aspect, A
                   >::type merged_aspect;

  typedef basic_filter<  merged_aspect,
                        _read_policy_,
                        _write_policy_> type;
};

typedef aa::aspect<> binary_filter_default_user_aspect;

/** Фильтр ввода/вывода двоичных данных. Предоставляет адвайсы чтения/записи
  * двоичных данных, доступ к которым осуществляется по тегам
  * _binary_read_ и _binary_write_.
  * @advice binary_read < ad_buf_binary_read > (const char*, size_t)
  * @advice binary_read < ad_buf_binary_read > ()
  * @advice binary_write < ad_buf_binary_write > (const char*, size_t)
  * @advice binary_write < ad_buf_binary_write >()
  * @advice on_assign(DR, DW) - вызывается при инициализации фильтра 
  *         (здесь и далее DR - дескриптор чтения, DW - дескриптор записи)
  * @advice on_read(const char*, size_t) - вызывается из адвайса binary_read
  *          при получении очередной порции данных.
  * @advice on_rclosed(DR) - вызывается из адвайса binary_read
  *          при попытке прочитать данные из закрытого дескриптора.
  * @advice on_rerror(DR) - вызывается из адвайса binary_read, если
  *          при попытке прочитать данные произошла ошибка
  * @advice on_write(const char*, size_t)- вызывается из адвайса binary_write
  *          при записи очередной порции данных.
  * @advice on_wclosed(DW) - вызывается из адвайса binary_write
  *          при попытке записать данные в закрытый дескриптор.
  * @advice on_werror(DW) - вызывается из адвайса binary_write, если
  *          при попытке записать данные произошла ошибка
  * @param A - пользовательский аспект (по умолчанию aspect<> )
  * @see basic_filter basic_filter
  * @see ad_buf_binary_read ad_buf_binary_read
  * @see ad_buf_binary_write ad_buf_binary_write
  * 
 */
template< typename A = binary_filter_default_user_aspect >
class binary_filter
  : public make_binary_filter_super_class<A>::type
{
public:
  typedef binary_filter<A> self;
  typedef typename make_binary_filter_super_class< A>::type super;
  typedef typename super::aspect aspect;

  template<typename AA>
  struct rebind
  {
    typedef binary_filter< AA > type;
  };

  /*
    friend class aa::advice_cast<_reader_, aspect>::type; 
    friend class aa::advice_cast<_writer_, aspect>::type; 
  */

  typedef typename super::read_return_type read_return_type;
  typedef typename super::write_return_type write_return_type;

  typedef typename super::read_size_type read_size_type;
  typedef typename super::write_size_type write_size_type;

  typedef read_size_type size_type;
  typedef read_return_type return_type;

  typedef typename super::read_advice read_advice;
  typedef typename super::write_advice write_advice;

  typedef typename read_advice::desc_type read_desc_type;
  typedef typename write_advice::desc_type write_desc_type;

  typedef typename read_advice::desc_type desc_type;

  typedef typename super::mutex_type mutex_type;
  /** Возвращает дескриптор чтения адвайса binary_read */
  read_desc_type get_rd() const 
  { 
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_rd();

   //    return static_cast<read_advice>(*this).get_d();
    // return super::_get_reader().get_d(); 
  }
  /** Возвращает дескриптор записи адвайса binary_write */
  write_desc_type get_wd() const 
  { 
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_wd();

    //return static_cast<write_advice>(*this).get_d();
    // return super::_get_writer().get_d(); 
  }


  /** Возвращает статус источника */
  bool get_rstatus() const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_rstatus();
    //return super::_get_reader().get_status();
  }

  /** Возвращает статус получателя */
  bool get_wstatus() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_wstatus();
    //return super::_get_writer().get_status();
  }

  /** Возвращает общий статус */
  bool get_status() const 
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_status();
    //return super::_get_reader().get_status()
    //       && super::_get_writer().get_status();
  }

  operator bool () const { return self::get_status(); }

  /** Инициализирует фильтр дескрипторами чтения и записи
    * @advice on_assign(read_desc_type, write_desc_type )
    * @param wd - дескриптор чтения
    * @param rd - дескриптор записи
    */
  void assign(read_desc_type rd, write_desc_type wd )
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_assign(*this, rd, wd);
  }

  /** Инициализирует фильтр общим дескрипторам чтения и записи
    * @advice on_assign(desc_type, desc_type )
    * @param d - дескриптор чтения/записи
    */
  void assign(desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_assign(*this, d, d);
  }

  /** Читает данние из источника
    * @advice on_read(char*, size_t)
    * @advice on_close(read_desc_type)
    * @advice on_error(read_desc_type)
    * @return число прочитанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  read_return_type read()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this);
  }

  /** Читает данние из источника в пользовательский буфер
    * @advice on_read(char*, size_t) 
    * @advice on_close(read_desc_type)
    * @advice on_error(read_desc_type)
    * @param d - указатель на буффер данных, куда будут записаны 
    *            полученные данные.
    * @param s - размер буфера
    * @return число прочитанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  read_return_type read(char *d, read_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this, d, s);
  }

  /** Пишет данные из пользовательского буфера. Если не все данные 
    * были записаны за одну операцию, то оставшиеся данные копируются
    * во внутренний буфер.
    * @advice on_write(char*, size_t) 
    * @advice on_close(read_desc_type)
    * @advice on_error(read_desc_type)
    * @param d - указатель на буфер данных.
    * @param s - размер буфера
    * @return число записанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  write_return_type write(const char *d, write_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this, d, s);
  }


  /** Пишет данные из внутренний буфера.
    * @advice on_write(char*, size_t) 
    * @advice on_close(read_desc_type)
    * @advice on_error(read_desc_type)
    * @return число записанных байт данных, 0 - если источник 
    *         закрыт и -1 в случае ошибки
    */
  write_return_type write()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this);
  }
  
  /** Флаг готовности буфера для записи. 
    * В буфере есть данные предназначенные для записи и
    * их можно записать методом write() (дескриптор открыт 
    * для записи)*/
  bool ready() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return super::_get_writer().ready();
  }

protected:

  /** Возвращает статус источника */
  bool _get_rstatus() const 
  {
    return static_cast<const read_advice&>(*this).get_status();
    //return super::_get_reader().get_status();
  }

  void _set_rstatus(bool status)
  {
    static_cast<read_advice&>(*this).set_status(status);
    //return super::_get_reader().get_status();
  }

  /** Возвращает статус получателя */
  bool _get_wstatus() const
  {
    return static_cast<const write_advice&>(*this).get_status();
    // return super::_get_writer().get_status();
  }

    /** Возвращает статус получателя */
  void _set_wstatus(bool status) 
  {
    static_cast<write_advice&>(*this).set_status(status);
    // return super::_get_writer().get_status();
  }

  /** Возвращает общий статус */
  bool _get_status() const 
  {
    return _get_rstatus() && _get_wstatus();
  }

    /** Возвращает общий статус */
  void _set_status(bool status)  
  {
    _set_rstatus(status);
    _set_wstatus(status);
  }

  /** Возвращает дескриптор чтения адвайса binary_read */
  read_desc_type _get_rd() const 
  { 
    return static_cast<const read_advice&>(*this).get_d();
    // return super::_get_reader().get_d(); 
  }

  /** Возвращает дескриптор чтения адвайса binary_read */
  void _set_rd(read_desc_type d) 
  { 
    static_cast<read_advice&>(*this).set_d(d);
    // return super::_get_reader().get_d(); 
  }

  /** Возвращает дескриптор записи адвайса binary_write */
  write_desc_type _get_wd() const 
  { 
    return static_cast<const write_advice&>(*this).get_d();
    // return super::_get_writer().get_d(); 
  }

    /** Возвращает дескриптор чтения адвайса binary_read */
  void _set_wd(write_desc_type d) 
  { 
    static_cast<write_advice&>(*this).set_d(d);
    // return super::_get_reader().get_d(); 
  }

  template<typename T>
  void _assign(T& t, read_desc_type rd, write_desc_type wd )
  {
    self::_set_rd(rd);
    self::_set_wd(wd);
    super::_get_reader().clear();
    super::_get_writer().clear();

    typedef typename aa::advice_cast<
	  _cleaner_advice_,
      aspect >::advice cleaner;
    static_cast<cleaner&>(t).clear(t);

    typedef typename aa::advice_cast<
         _on_assign_,
         aspect >::advice on_assign;
    static_cast<on_assign&>(t)(t, rd, wd);
  }

  template<typename T>
  void _release(T& t)
  {
    // self::_set_rd( read_desc_type() );
    //self::_set_wd( write_desc_type() );

    super::_get_reader().clear();
    super::_get_writer().clear();

    typedef typename aa::advice_cast< _cleaner_advice_, aspect >::advice cleaner;
    static_cast<cleaner&>(t).clear(t);

    typedef typename aa::advice_cast< _on_release_, aspect >::advice on_release;
    static_cast<on_release&>(t)(t);
  }

public:
  template<typename T>
  read_return_type _read(T& t, char *d, read_size_type s)
  {
    return super::_read(t, d, s);
  }

  template<typename T>
  write_return_type _write(T& t, const char *d, write_size_type s)
  {
    typedef typename aa::advice_cast< _on_writing_, aspect >::advice on_writing;
    static_cast<on_writing&>(t)(t, d, s);
    return super::_write(t, d, s);
  }

  template<typename T>
  read_return_type _read(T& t)
  {
    return super::_read(t);
  }

  template<typename T>
  write_return_type _write(T& t)
  {
    return super::_write(t);
  }


};

}}

#endif // FAS_FILTERS_BINARY_FILTER_HPP
