//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MUX_MUX_TYPES_H
#define FAS_MUX_MUX_TYPES_H

#include <fas/system/types.hpp>

namespace fas { namespace mux {

#ifdef WIN32
typedef fas::system::socket_t descriptor_t;
typedef fas::system::socket_t socket_t;
#else
typedef fas::system::descriptor_t descriptor_t ;
typedef fas::system::socket_t socket_t;
#endif

/*#ifdef WIN32
typedef SOCKET socket_t;
typedef SOCKET descriptor_t;
#else
typedef int socket_t;
typedef int descriptor_t;
#endif
*/

} }

#endif //FAS_MUX_MUX_TYPES_H
