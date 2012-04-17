//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_ICONNECTION_HPP
#define FAS_INET_ICONNECTION_HPP

namespace fas{ namespace inet{

class iconnection
{
public:
  virtual ~iconnection(){}
  /*virtual bool release(bool lock = true) = 0;*/
};

}}

#endif // FAS_INET_IPOOL_H

