//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_IPOOL_HPP
#define FAS_INET_IPOOL_HPP

namespace fas{ namespace inet{

/** @interface */
template<typename T>
class ipool
{
public:
  virtual ~ipool(){}
  virtual void free(T* item) = 0;
};

}}

#endif // FAS_INET_IPOOL_H

