//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_INTRODUCTION_H
#define FAS_AOP_INTRODUCTION_H

#include <fas/aop/aspect.hpp>

namespace fas{ namespace aop {

template<typename S, typename A = aspect<> , typename F = void>
struct introduction:
  public S,
  public F::template rebind<A>::type
{
  typedef introduction<S, A, F> self;
  typedef typename F::template rebind<A>::type super;
  typedef typename super::aspect aspect;

  template<typename SS = S, typename AA = A, typename FF = F>
  struct rebind
  {
    typedef introduction<SS, AA, FF> type;
  };
};

template<typename S, typename A>
struct introduction<S, A, void>
  : public S
  , public aspect_class<A>
{
  typedef introduction<S, A, void> self;
  typedef aspect_class<A> super;
  typedef typename super::aspect aspect;

  template<typename SS = S, typename AA = A, typename FF = void>
  struct rebind
  {
    typedef introduction<SS, AA, FF> type;
  };
};

}}

#endif
