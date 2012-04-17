//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_AD_NAMED_PROXY_HPP
#define FAS_AOP_AD_NAMED_PROXY_HPP

#include <fas/aop/aspect.hpp>
#include <fas/pattern/type_list.hpp>

namespace fas { namespace adv {

namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;

/** 
  * @param G - групоовой тег (все адвайсы с этим тегом должны иметь метод const char* name() const )
  * @param U - для необработаных запросов
  * @param P - разрешить частичное совпадение и *
  */
template<typename G, typename U, bool P = false>
class ad_named_proxy
{
public:
  template<typename T>
  void operator()(T&, const char* name) {   throw;  }

  template<typename T, typename T1>
  void operator()(T&, const char* name, T1){ throw; }

  template<typename T, typename P0, typename P1>
  void operator()(T& t, const char* name, P0 p0, P1 p1) 
  {
    typedef typename T::aspect::advice_list advice_list;
    typedef typename ap::select<G, advice_list>::type invoke_list;
    // TODO: проверить, что объект invoke_list() гарантировано не создается
    _select(t, invoke_list(), name , p0, p1 );
  }

  template<typename T, typename T1, typename T2, typename T3>
  void operator()(T&, const char* name, T1, T2, T3) { throw;}

  template<typename T, typename T1, typename T2, typename T3, typename T4>
  void operator()(T&, const char* name, T1, T2, T3, T4) { throw;}
private:

  static bool _raw_equal( const char* left, const char* right)
  {
    for ( ; *left!='\0' && *right!='\0' ; ++left, ++right )
      if (*left!=*right)
        return false;
    return P || *left!=*right;
  }

  template<typename T, typename L, typename P0, typename P1>
  void _select(T& t, L, const char* name, P0 p0, P1 p1)
  {
    typedef typename L::left_type current_type;
    current_type& current = t.get_aspect().template get<typename L::left_type>();
    if ( _raw_equal( current.name(), name ) || ( P && current.name()[0]=='*') )
      current(t, p0, p1);
    _select(t, typename L::right_type(), name, p0, p1);
  }

  template< typename T, typename P0, typename P1>
  void _select(T& t, ap::empty_type, const char*, P0 p0, P1 p1)
  {
    t.get_aspect().template get<U>()(t, p0, p1);
  }

};


}}

#endif
