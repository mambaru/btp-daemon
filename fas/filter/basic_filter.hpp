//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTER_BASIC_FILTER_HPP
#define FAS_FILTER_BASIC_FILTER_HPP

#include <fas/aop/aspect.hpp>
#include <fas/aop/advice.hpp>
#include <fas/pattern/type_list.hpp>
#include <fas/aop/tag.hpp>
#include <fas/adv/ad_mutex.hpp>
// #include <fas/filter/ad_file.hpp>

namespace fas { namespace filter {

namespace ad = ::fas::adv;
namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;

/** Осуществляет вызов методов _read контекста вызова.
  * Контекст должен предоставлять тип read_return_type.
  */
class ad_reader
{
public:

  /** Осуществляет вызов метода _read контекста вызова без параметров */
  template<typename T>
  typename T::read_return_type operator ()(T& t)
  {
    return t._read(t);
  }

  /** Осуществляет вызов метода _read контекста вызова с одним параметром */
  template<typename T, typename P0>
  typename T::read_return_type operator ()(T& t, P0 p0)
  {
    return t._read(t, p0);
  }

  /** Осуществляет вызов метода _read контекста вызова с двумя параметрами */
  template<typename T, typename P0, typename P1>
  typename T::read_return_type operator ()(T& t, P0 p0, P1 p1)
  {
    return t._read(t, p0, p1);
  }

  /** Осуществляет вызов метода _read контекста вызова с тремя параметрами */
  template<typename T, typename P0, typename P1, typename P2>
  typename T::read_return_type operator ()(T& t, P0 p0, P1 p1, P2 p2)
  {
    return t._read(t, p0, p1, p2);
  }
};

/** Осуществляет вызов методов _write контекста вызова.
  * Контекст должен предоставлять тип write_return_type.
  */
class ad_writer
{
public:
  /** Осуществляет вызов метода _write контекста вызова без параметров */
  template<typename T>
  typename T::write_return_type operator ()(T& t)
  {
    return t._write(t);
  }

  /** Осуществляет вызов метода _write контекста вызова с одним параметром */
  template<typename T, typename P0, typename P1>
  typename T::write_return_type operator ()(T& t, P0 p0)
  {
    return t._write(t, p0);
  }

  /** Осуществляет вызов метода _write контекста вызова с двумя параметрами */
  template<typename T, typename P0, typename P1>
  typename T::write_return_type operator ()(T& t, P0 p0, P1 p1)
  {
    return t._write(t, p0, p1);
  }

  /** Осуществляет вызов метода _write контекста вызова с тремя параметрами */
  template<typename T, typename P0, typename P1, typename P2>
  typename T::write_return_type operator ()(T& t, P0 p0, P1 p1, P2 p2)
  {
    return t._write(t, p0, p1, p2);
  }
};

struct _read_{};
struct _write_{};

struct _reader_{};
struct _writer_{};
struct _mutex_{};



struct mutex_advice: aa::advice< aa::tag<_mutex_>, ad::ad_mutex<> >{};
struct reader_advice: aa::advice< aa::tag<_reader_>, ad_reader > {};
struct writer_advice: aa::advice< aa::tag<_writer_>, ad_writer > {};

struct basic_filter_advice_list
  : ap::type_list_n<
      reader_advice,
      writer_advice,
      mutex_advice
    >::type
{};

struct basic_filter_aspect: aa::aspect< basic_filter_advice_list >{};

/*template<typename A>
class friend_class : public A {};*/

/** Предоставляет обобщенный механизм чтения/записи посредством поставляемых
  * пользователем соответствующих адвайсов. Доступ к методам чтения и записи
  * класса возможен через дружественные адвайсы _reader_ и _writer_.
  * @advice reader - обеспечивает доступ к механизму чтения
  * @advice writer - обеспечивает доступ к механизму записи
  * @advice mutex - предоставляет механизм синхронизации (по умолчанию
  *         предоставляется мьютекс no_mutex, т.е. без синхронизации).
  *         Обязан определить тип mutex_type.
  * @param A - пользовательский аспект
  * @param R - тег адвайса пользовательского аспекта реализующий механизм чтения.
  * @param W - тег адвайса пользовательского аспекта реализующий механизм записи
  * @typedef aspect - объединеный аспект класса basic_filter 
  * @typedef read_advice - тип адвайса пользовательского аспекта реализующий механизм чтения
  * @typedef write_advice - тип адвайса пользовательского аспекта реализующий механизм записи
  * @typedef mutex_advice - тип адвайса пользовательского аспекта предоставляющий механизм синхронизации
  * @typedef mutex_type - тип объекта синхронизации предоставляемый адвайсом синхронизации
  * @typedef read_return_type - тип, возвращаемый пользовательским адвайсом чтения
  * @typedef write_return_type - тип, возвращаемый пользовательским адвайсом записи
  * @see ad_reader ad_reader - предоставляемый адвайс доступа к механизму чтения
  * @see ad_writer ad_writer - предоставляемый адвайс доступа к механизму записи
  * @see ad_mutex ad_mutex - предоставляемый адвайс предосталяющий механизм синхронизации
  * @see no_mutex no_mutex - предоставляемый по умолчанию объект синхронизации
  */
template<typename A, typename R, typename W>
class basic_filter
  : public aa::aspect_class<basic_filter_aspect, A>
{
public:
  typedef basic_filter<A, R, W> self;
  typedef aa::aspect_class<basic_filter_aspect, A> super;
  typedef typename super::aspect aspect;

private:
  typedef typename aa::advice_cast<_mutex_, aspect>::advice mutex_advice_type;
public:

  /*
    friend class aa::advice_cast<_reader_, aspect>::type; 
    friend class aa::advice_cast<_writer_, aspect>::type; 
  */

  typedef typename aa::advice_cast<R, aspect>::type reader_advice;
  typedef typename aa::advice_cast<W, aspect>::type writer_advice;

  typedef typename aa::advice_cast<typename reader_advice::read_advice_tag, aspect>::type read_advice;
  typedef typename aa::advice_cast<typename writer_advice::write_advice_tag, aspect>::type write_advice;

  typedef typename mutex_advice_type::mutex_type mutex_type;

  typedef typename read_advice::return_type read_return_type;
  typedef typename write_advice::return_type write_return_type;

  typedef typename read_advice::size_type read_size_type;
  typedef typename write_advice::size_type write_size_type;

  template<typename AA = A, typename RR = R, typename WW = W>
  struct rebind
  {
    typedef basic_filter<AA, RR, WW> type;
  };

  /** Возвращает ссылку на объект синхронизации */
  mutex_type& get_mutex() const 
  {
    return static_cast<const mutex_advice_type&>(*this).get_mutex();
    // VC8: internal compiler error (((
    // return super::get_aspect().template get<_mutex_>().get_mutex(); 
  }

protected:

  /** Возвращает ссылку на объект пользовательского адвайса чтения */
  reader_advice& _get_reader() { return super::get_aspect().template get<R>(); };
  const reader_advice& _get_reader() const { return super::get_aspect().template get<R>(); };

  /** Возвращает ссылку на объект пользовательского адвайса записи */
  writer_advice& _get_writer() { return super::get_aspect().template get<W>(); };
  const writer_advice& _get_writer() const { return super::get_aspect().template get<W>(); };

public:
  /** Вызывает пользовательский адвайс чтения без параметров */
  template<typename T>
  read_return_type _read(T& t)
  {
    return static_cast<reader_advice&>(*this)(t);
  }

  /** Вызывает пользовательский адвайс чтения с одним параметром */
  template<typename T, typename P0>
  read_return_type _read(T& t, P0 p0)
  {
    return static_cast<reader_advice&>(*this)(t, p0);
  }

  /** Вызывает пользовательский адвайс чтения с двумя параметрами */
  template<typename T, typename P0, typename P1>
  read_return_type _read(T& t, P0 p0, P1 p1)
  {
    return static_cast<reader_advice&>(*this)(t, p0, p1);
  }

  /** Вызывает пользовательский адвайс чтения с тремя параметрами */
  template<typename T, typename P0, typename P1, typename P2>
  read_return_type _read(T& t, P0 p0, P1 p1, P2 p2)
  {
    return static_cast<reader_advice&>(*this)(t, p0, p1, p2);
  }

  /** Вызывает пользовательский адвайс записи без параметров */
  template<typename T>
  write_return_type _write(T& t)
  {
    return static_cast<writer_advice&>(*this)(t);
  }

  /** Вызывает пользовательский адвайс записи с одним параметром */
  template<typename T, typename P0>
  write_return_type _write(T& t, P0 p0)
  {
    return static_cast<const writer_advice&>(*this)(t, p0);
  }

  /** Вызывает пользовательский адвайс записи с двумя параметрами */
  template<typename T, typename P0, typename P1>
  write_return_type _write(T& t, P0 p0, P1 p1)
  {
    return static_cast<writer_advice&>(*this)(t, p0, p1);
  }

  /** Вызывает пользовательский адвайс записи с тремя параметрами */
  template<typename T, typename P0, typename P1, typename P2>
  write_return_type _write(T& t, P0 p0, P1 p1, P2 p2)
  {
    return static_cast<writer_advice&>(*this)(t, p0, p1, p2);
  }
};

}}

#endif // FAS_FILTER_BASIC_FILTER_H
