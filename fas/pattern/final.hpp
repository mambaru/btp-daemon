//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_FINAL_HPP
#define FAS_PATTERNS_FINAL_HPP

#include <fas/pattern/conversion.hpp>

namespace fas{ namespace pattern {

/*
template<typename T>
class final;

template<typename T>
class final_friend: public T {};

template<typename T>
class final_helper
{
  friend class type2type< final<T> >::type;
  friend class type2type< T >::type;
private:
  final_helper(){};
};

template<typename T>
class final: virtual public final_helper< T >{};
*/

}}

#endif
