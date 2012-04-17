//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_STATIC_CHECK_HPP
#define FAS_PATTERNS_STATIC_CHECK_HPP

namespace fas{ namespace pattern {

template<int> struct static_check;

template<> struct static_check<true> {
  enum { result = 1 };
};

}}
#endif // FAS_PATTERNS_STATIC_CHECK_H
