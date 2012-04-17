//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_CALL_POINT_H
#define FAS_AOP_CALL_POINT_H

namespace fas{ namespace aop {

template<typename L, typename N, typename T, typename R, typename P1>
class call_point
  : public ap::tag_list_traits<L>::hierarchy
{
  typedef R (T::*mem_fun)(T, P1);
  mem_fun _mf;
public:
  void operator = ( const mem_fun& mf)
  {
    _mf = mf;
  }

  void operator()(T& t, P1 p1)
  {
    (t.*_mf)(t, p1);
    t.template get<N>()(t, p1);
  }
};

}}

#endif
