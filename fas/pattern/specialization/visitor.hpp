//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_VISITOR_SPECIALIZATION_HPP
#define FAS_PATTERNS_VISITOR_SPECIALIZATION_HPP


#include <fas/pattern/type_list.hpp>
#include <fas/pattern/tags.hpp>
#include <fas/pattern/scatter_hierarchy.hpp>

#include <set>
#include <algorithm>

namespace fas{ namespace pattern {




template<typename V>
class visitor<V, void>
 : public V
{
};



template<typename T>
struct visit_list_traits<visit_list<T> >
{
  typedef visit_list<T> type;
};




template<typename T>
struct visitor_list_traits< visitor_list<T> >
{
  typedef visitor_list<T> type;
};


template< typename TWhen, typename TWho >
class visiting<TWhen, TWho, void>
  : public visiting< typename type2tag<TWhen>::type, TWhen, TWho> 
{
};


}}

#endif
