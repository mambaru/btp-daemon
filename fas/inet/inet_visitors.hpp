//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_VISITORS_HPP
#define FAS_INET_VISITORS_HPP

#include <fas/inet/server.hpp>
#include <fas/inet/mt_server.hpp>

namespace fas{ namespace inet{

template<typename V>
struct server_visitor:
  public V
{
  typedef V super;
  typedef server_visitor<V> self;
  template<typename T>
  void visit(T* t)
  {
    _visit(t);
  }

private:
  template< typename A, typename SA, net_protocol_t NP,
            transport_protocol_t TP, typename C >
  void _visit( server<A, SA, NP, TP, C>* s)
  {
    typedef server<A, SA, NP, TP, /*X,*/ C> server;
    typedef typename server::mutex_type mutex_type;

    typename mutex_type::scoped_lock sl( s->get_mutex() );

    typename server::iterator beg = s->begin();
    typename server::iterator end = s->end();
    for (;beg!=end; ++beg)
    {
      super::visit( &(*beg));
    }
  }
};


template<typename V >
struct mt_server_visitor:
  public V
{
  typedef V super;
  typedef mt_server_visitor<V> self;
  template<typename T>
  void visit(T* t)
  {
    _visit(t);
  }

private:

  template< typename A, typename SA, net_protocol_t NP,
            transport_protocol_t TP, typename M, typename S >
  void _visit( mt_server<A, SA, NP, TP, M, S>* s)
  {
    typedef mt_server<A, SA, NP, TP, M, S> mt_server;
    typedef typename mt_server::mutex_type mutex;

    typename mutex::scoped_lock sl( s->get_mutex() );

    typename mt_server::iterator beg = s->begin();
    typename mt_server::iterator end = s->end();
    for (;beg!=end; ++beg)
    {
      super::visit( beg->chd );
    }
  }
};

}}

#endif
