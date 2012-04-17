//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_POOLED_ITEM_HPP
#define FAS_INET_POOLED_ITEM_HPP

#include <fas/inet/ipool.hpp>

namespace fas{ namespace inet {

template<typename T>
class pooled_item: public T
{
public:
  typedef T super;
  typedef pooled_item<T> self;
  typedef ipool<self> pool_interface;

  pooled_item()
    : _pool(0) {}

  void pool( pool_interface* value)
  {
    _pool= value;
  }

  virtual bool release(bool lock = false)
  {
    if ( super::release(lock) )
      _pool->free(this);
    else
      return false;
    return true;
  }

private:

  pool_interface* _pool;

  /**
   *@link dependency
   *@associates connection
   * @stereotype bind
   */
  /*# int lnkconnection; */
};

}}
#endif //POOLED_ITEM_H
