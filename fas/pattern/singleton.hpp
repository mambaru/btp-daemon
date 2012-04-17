//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_SINGLETON_HPP
#define FAS_PATTERNS_SINGLETON_HPP

namespace fas{ namespace pattern {

template<typename T>
class singleton
{
public:
  T& instance()
  {
    static T obj;
    return obj;
  }
protected:
  singleton(){}
  explicit singleton(const singleton&){}
};

} }

#endif
