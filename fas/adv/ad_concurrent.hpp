//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_CONCURRENT_H
#define FAS_FILTERS_AD_CONCURRENT_H

#include <fas/system/thread.hpp>

namespace fas { namespace adv { 


/** Использует объект синхронизации контекста вызова для
  * доступа к адвайсу N контекста вызова.
  * @param N - тег адвайса к которому будет перенаправлен вызов
  * @param R - тип возвращаемого значения давайса N
  */
template<typename N, typename R = void>
class ad_concurrent
{
public:
  typedef ad_concurrent<N, R> self;

  template<typename T>
  R operator()(T& t)
  {
    typename T::mutex_type::scoped_lock sl( t.get_mutex() );
    return t.get_aspect().template get<N>()(t);
  }

  template<typename T, typename T0>
  R operator()(T& t, T0 t0)
  {
    typename T::mutex_type::scoped_lock sl( t.get_mutex() );
    return t.get_aspect().template get<N>()(t, t0);
  }

  template<typename T, typename T0, typename T1>
  R operator()(T& t, T0 t0, T1 t1)
  {
    typename T::mutex_type::scoped_lock sl( t.get_mutex() );
    return t.get_aspect().template get<N>()(t, t0, t1);
  }

  template<typename T, typename T0, typename T1, typename T2>
  R operator()(T& t, T0 t0, T1 t1, T2 t2)
  {
    typename T::mutex_type::scoped_lock sl( t.get_mutex() );
    return t.get_aspect().template get<N>()(t, t0, t1, t2);
  }
private:
};

}}

#endif
