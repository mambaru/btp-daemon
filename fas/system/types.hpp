//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_SYSTEM_TYPES_HPP
#define FAS_SYSTEM_TYPES_HPP

#include <fas/unp.h>

namespace fas{ namespace system { 

typedef socket_t socket_t;
typedef descriptor_t descriptor_t;
  /*
#ifdef WIN32
typedef SOCKET socket_t;
typedef SOCKET descriptor_t;
#else
typedef int socket_t;
typedef int descriptor_t;
#endif

*/
/*typedef int descriptor_t;
typedef int socket_t;
*/

}}

#endif
