//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_NTH_CAST_HPP
#define FAS_PATTERNS_NTH_CAST_HPP

namespace fas{ namespace pattern
{

template<int i, typename T>
struct nth_cast;

template<typename T>
struct nth_cast<0, T>
{
  typedef T type;
};

template< int i, typename T>
struct nth_cast
{
  typedef typename nth_cast<i-1, typename T::super >::type type;
};

}}
#endif
