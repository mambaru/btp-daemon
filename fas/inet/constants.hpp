//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_CONSTANTS_H
#define FAS_INET_CONSTANTS_H

#include <fas/system/inet.hpp>
#include <fas/inet/types.hpp>

namespace fas{ namespace inet {

const net_protocol_t IPv4 = ::fas::system::inet::IPv4;
const net_protocol_t IPv6 = ::fas::system::inet::IPv6;

const transport_protocol_t TCP = ::fas::system::inet::TCP;
const transport_protocol_t UDP = ::fas::system::inet::UDP;


} }

#endif // FAS_INET_CONSTANTS_H
