//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_AD_PROXY_HPP
#define FAS_AOP_AD_PROXY_HPP

namespace fas { namespace adv {

/** 
  * @param R - тип возвращаемого значения
  */
template<typename N, typename R = void>
class ad_proxy
{
public:
  template<typename T>
  R operator()(T& t)
  {
    return t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename P1>
  R operator()(T& t, P1 p1)
  {
    return t.get_aspect().template get<N>()(t, p1);
  }

  template<typename T, typename P1, typename P2>
  R operator()(T& t, P1 p1, P2 p2) 
  {
    return t.get_aspect().template get<N>()(t, p1, p2);
  }

  template<typename T, typename P1, typename P2, typename P3>
  R operator()(T& t, P1 p1, P2 p2, P3 p3) 
  {
    return t.get_aspect().template get<N>()(t, p1, p2, p3);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4>
  R operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4) 
  {
    return t.get_aspect().template get<N>()(t, p1, p2, p3, p4);
  }
};

template<typename N>
class ad_proxy<N, void>
{
public:
  template<typename T>
  void operator()(T& t)
  {
    t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename P1>
  void operator()(T& t, P1 p1)
  {
    t.get_aspect().template get<N>()(t, p1);
  }

  template<typename T, typename P1, typename P2>
  void operator()(T& t, P1 p1, P2 p2) 
  {
    t.get_aspect().template get<N>()(t, p1, p2);
  }

  template<typename T, typename P1, typename P2, typename P3>
  void operator()(T& t, P1 p1, P2 p2, P3 p3) 
  {
    t.get_aspect().template get<N>()(t, p1, p2, p3);
  }

  template<typename T, typename P1, typename P2, typename P3, typename P4>
  void operator()(T& t, P1 p1, P2 p2, P3 p3, P4 p4) 
  {
    t.get_aspect().template get<N>()(t, p1, p2, p3, p4);
  }
};

}}

#endif
