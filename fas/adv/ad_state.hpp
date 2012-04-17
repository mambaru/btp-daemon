//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_AD_STATE_HPP
#define FAS_AOP_AD_STATE_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/aop/aspect.hpp>


namespace fas { namespace adv {

namespace ap=::fas::pattern;
namespace aa=::fas::aop;



/** Авайс-класс - состояние. 
  * @param G - группа состояний
  * @param I - начальное состояние 
  * 
  */
template<typename G, typename I, bool a = false>
class ad_state
{
  int _state;
public:
  ad_state() { _state = 0; }

  template<typename T>
  void clear(T& t)
  {
    // _state = 0;
    //typedef typename ap::select<G, typename T::advice_list>::type first_state;
    this->state<I>(t);
  }

  /*
  template<typename TT>
  void state() 
  {
    _state = ap::type_position<TT, L>::result; 
  }*/

  template<typename TT, typename T>
  typename aa::advice_cast<TT, typename T::aspect>::type& state(T& t) 
  {

    _state = ap::type_position<TT, typename ap::select<G, typename T::advice_list>::type >::result; 
    _activate<TT>(t, ap::bool2type<a>() );
    return t.get_aspect().template get<TT>();
  }

  template<typename TT, typename T>
  const typename aa::advice_cast<TT, typename T::aspect>::type& state(T& t) const
  {
//    _state = ap::type_position<TT, typename T::advice_list>::result; 
    _state = ap::type_position<TT, typename ap::select<G, typename T::advice_list>::type >::result; 
    return t.get_aspect().template get<TT>();
  }

  template<typename T>
  void operator()(T& t)
  { 
    //typedef typename T::advice_list advice_list;
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t);
  }

  template<typename T, typename P1>
  void operator()(T& t, P1 p1)
  { 
    //typedef typename T::advice_list advice_list;
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t, p1);
  }

  template<typename T, typename P1, typename P2>
  void operator()(T& t, P1 p1, P2 p2) 
  { 
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t, p1, p2);
  }

  template<typename T, typename P1, typename P2, typename P3>
  void operator()(T& t, P1 p1, P2 p2, P3 p3) 
  { 
    //typedef typename T::advice_list advice_list;
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t, p1, p2, p3);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4>
  void operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4) 
  { 
    //typedef typename T::advice_list advice_list;
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t, p1, p2, p4);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
  void operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
  { 
    //typedef typename T::advice_list advice_list;
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    this->_select(state_list(), t, p1, p2, p4, p5);
  }

private:
  template<typename TT, typename T>
  void _activate(T& t, ap::bool2type<true> )
  {
    t.get_aspect().template get<TT>().activate(t);
  }

  template<typename TT, typename T>
  void _activate(T&, ap::bool2type<false> )
  {
    //t.get_aspect().template get<TT>().activate(t);
  }


  template<typename LL, typename T>
  void _select(LL, T& t)
  { 
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    if ( _state == ap::length<state_list>::result - ap::length<LL>::result )
      static_cast<typename LL::left_type&>(t.get_aspect())(t);
      // t.get_aspect().template get<typename LL::left_type>()(t);
    else
    this->_select(typename LL::right_type(), t);
  }

  template<typename T>
  void _select(ap::empty_type, T&)
  { 
    throw;
  }

  template<typename LL, typename T, typename P1>
  void _select(LL, T& t, P1 p1)
  { 
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    if ( _state == ap::length<state_list>::result - ap::length<LL>::result )
      static_cast<typename LL::left_type&>(t.get_aspect())(t, p1);
      // t.get_aspect().template get<typename LL::left_type>()(t, p1);
    else
      this->_select(typename LL::right_type(), t, p1);
  }

  template<typename T, typename P1>
  void _select(ap::empty_type, T&, P1)
  { 
    throw;
  }

  template<typename LL, typename T, typename P1, typename P2>
  void _select(LL, T& t, P1 p1, P2 p2)
  { 
    typedef typename ap::select<G, typename T::advice_list>::type state_list;
    if ( _state == ap::length<state_list>::result - ap::length<LL>::result )
      static_cast<typename LL::left_type&>(t.get_aspect())(t, p1, p2);
    else
      this->_select(typename LL::right_type(), t, p1, p2);
  }

  template<typename T, typename P1, typename P2>
  void _select(ap::empty_type, T&, P1, P2)
  {
    throw;
  }
};

template<typename G, typename S, typename T>
inline
typename aa::advice_cast<S, typename T::aspect>::type&
state(T& t)
{
  return t.get_aspect().template get<G>().template state<S>(t);
}
/*
template<typename G>
template<typename TT, typename T>
void ad_state<G, true>::_activate<>(T& t)
{
  t.get_aspect().template get<TT>().activate(t);
}*/
/*
template<typename G, bool a>
template<typename TT, typename T>
void ad_state<G, a>::template _activate<TT, T>(T& )
{
  
}*/

  /*
template<typename L, typename U>
class ad_state
{
  int _state;
public:
  ad_state() { _state = ap::type_position<U, L>::result; }

  template<typename TT>
  void state() 
  {
    _state = ap::type_position<TT, L>::result; 
  }

  template<typename TT, typename T>
  typename aa::advice_cast<TT, typename T::aspect>::type& state(T& t) 
  {
    _state = ap::type_position<TT, L>::result; 
    return t.get_aspect().template get<TT>();
  }

  
  template<typename TT, typename T>
  const typename aa::advice_cast<TT, typename T::aspect>::type& state(T& t) const
  {
    _state = ap::type_position<TT, L>::result; 
    return t.get_aspect().template get<TT>()
  }

  template<typename T>
  void operator()(T& t)
  { 
    this->_select(L(), t);
  }

  template<typename T, typename P1>
  void operator()(T& t, P1 p1)
  { 
    this->_select(L(), t, p1);
  }

  template<typename T, typename P1, typename P2>
  void operator()(T& t, P1 p1, P2 p2) 
  { 
    this->_select(L(), t, p1, p2);
  }

  template<typename T, typename P1, typename P2, typename P3>
  void operator()(T& t, P1 p1, P2 p2, P3 p3) 
  { 
    this->_select(L(), t, p1, p2, p3);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4>
  void operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4) 
  { 
    this->_select(0, t, p1, p2, p4);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
  void operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
  { 
    this->_select(0, t, p1, p2, p4, p5);
  }

private:
  template<typename LL, typename T>
  void _select(LL, T& t)
  { 
    if ( _state == ap::length<L>::result - ap::length<LL>::result )
      t.get_aspect().template get<typename LL::left_type>()(t);
    else
    this->_select(typename LL::right_type(), t);
  }

  template<typename LL, typename T>
  void _select(ap::empty_type, T&)
  { 
    throw;
  }

    template<typename LL, typename T, typename P1>
  void _select(LL, T& t, P1 p1)
  { 
    if ( _state == ap::length<L>::result - ap::length<LL>::result )
      t.get_aspect().template get<typename LL::left_type>()(t, p1);
    else
      this->_select(typename LL::right_type(), t, p1);
  }

  template<typename T, typename P1>
  void _select(ap::empty_type, T&, P1)
  { 
    throw;
  }

  template<typename LL, typename T, typename P1, typename P2>
  void _select(LL, T& t, P1 p1, P2 p2)
  { 
    if ( _state == ap::length<L>::result - ap::length<LL>::result )
      t.get_aspect().template get<typename LL::left_type>()(t, p1, p2);
    else
      this->_select(typename LL::right_type(), t, p1, p2);
  }

  template<typename LL, typename T, typename P1, typename P2>
  void _select(ap::empty_type, T&, P1, P2)
  { 
    throw;
  }

};

*/
}}

#endif
