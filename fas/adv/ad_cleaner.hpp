//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_CLEANER_HPP
#define FAS_FILTERS_AD_CLEANER_HPP

#include <iostream>
#include <fas/aop/aspect.hpp>
#include <fas/pattern/type_list.hpp>

namespace fas { namespace adv {

namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;

// для использования в качестве группового тега
struct _cleaner_ {};

/** Вызавает метод template<typename T> clear(T&) всех адвайсов
  * теги которых переданы в списке типов L
  * @param L - список тегов адвайсов
  * @advace - все адвайсы из списка тегов должны 
  *           иметь соответствующий метод clear
  */
template<typename G = _cleaner_ >
class ad_cleaner
{
  //typedef typename ap::type_list_traits<L>::type clear_list;
public:

  template<typename T>
  void operator()(T& t) { this->clear(t); }

  template<typename T, typename P0>
  void operator()(T& t, P0) { this->clear(t); }

  template<typename T, typename P0, typename P1>
  void operator()(T& t, P0, P1) { this->clear(t); }

  template<typename T, typename P0, typename P1, typename P2>
  void operator()(T& t, P0, P1, P2) { this->clear(t); }

  template<typename T, typename P0, typename P1, typename P2, typename P3>
  void operator()(T& t, P0, P1, P2, P3) { this->clear(t); }

  template<typename T, typename P0, typename P1, typename P2, typename P3, typename P4>
  void operator()(T& t, P0, P1, P2, P3, P4) { this->clear(t); }

  template<typename T>
  void clear(T& t) 
  {
    typedef typename ap::select<G, typename T::advice_list >::type cleaner_list;
    this->_clear( t, cleaner_list() );
  }

private:

  template<typename T, typename L>
  void _clear(T& t, L )
  {

    t.get_aspect().template get<typename L::left_type>().clear(t);
    this->_clear(t, typename L::right_type() );
  };

  template<typename T>
  void _clear(T& , ap::empty_type ) { }
};

/*
template<typename L>
class ad_cleaner
{
  typedef typename ap::type_list_traits<L>::type clear_list;
public:

  template<typename T>
  void operator()(T& t) { this->clear(t); }

  template<typename T, typename P0>
  void operator()(T& t, P0) { this->clear(t); }

  template<typename T, typename P0, typename P1>
  void operator()(T& t, P0, P1) { this->clear(t); }

  template<typename T>
  void clear(T& t) {  this->_clear( t, clear_list() );  }

private:

  template<typename T, typename F, typename S>
  void _clear(T& t, ap::type_list<F, S> )
  {
    t.get_aspect().template get<F>().clear(t);
    this->_clear(t, S() );
  };

  template<typename T>
  void _clear(T& , ap::empty_type ) { }
};
*/
}}

#endif
