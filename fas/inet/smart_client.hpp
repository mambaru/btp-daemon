//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_SMART_CLIENT_HPP
#define FAS_INET_SMART_CLIENT_HPP

#include <fas/adv/io/ad_smart_write.hpp>
#include <fas/inet/client.hpp>


namespace fas { namespace inet {

namespace adio = ::fas::adv::io;
namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace am = ::fas::mux;
namespace asi = ::fas::system::inet;
namespace af = ::fas::filter;

/// не включен в аспект фильтра, но может использоваться в аспекте клиента
struct smart_binary_write_advice
  : aa::advice<
      aa::tag<af::_write_policy_>,
      adio::ad_smart_write<af::_write_, af::_on_write_, af::_on_wclosed_, af::_on_werror_> 
    >
{};

template < typename A, net_protocol_t NP, transport_protocol_t TP, typename C>
struct mux_smart_client_base_super
{
  typedef mux_client_base<
    typename aa::aspect_merge<A, aa::aspect< smart_binary_write_advice > >::type, NP, TP, C> type;
};


template < typename A = aa::aspect<>,
           net_protocol_t NP = IPv4,
           transport_protocol_t TP = TCP,
           typename C = connection<A, am::mux_filter_base<> >
         >
class mux_smart_client_base
  : public mux_smart_client_base_super< A, NP, TP, C>::type
{
public:
  typedef mux_smart_client_base<A, NP, TP, C> self;
  typedef typename mux_smart_client_base_super< A, NP, TP, C>::type super;
  typedef typename super::write_return_type write_return_type;
  typedef typename super::write_size_type write_size_type;
  typedef typename super::mutex_type mutex_type;

  void confirm(bool write_next)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    _confirm(*this, write_next );
  }

  template<typename T>
  void _confirm(T& t, bool write_next)
  {

    t.get_aspect().template get<af::_write_policy_>().confirm(t, write_next );
  }


};

}}

#endif //FAS_INET_CLIENT_HPP
