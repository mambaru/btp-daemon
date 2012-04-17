//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_DETAIL_VISITOR_HPP
#define FAS_PATTERNS_DETAIL_VISITOR_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/pattern/tags.hpp>
#include <fas/pattern/scatter_hierarchy.hpp>

#include <set>
#include <algorithm>

namespace fas{ namespace pattern { namespace detail {

template<typename V>
class f_accept_visitor
{
  V& _visitor;
public:
  f_accept_visitor(V& visitor): _visitor(visitor)  {};
  template<typename L>
  void operator()(L* obj) {
    obj->accept_visitor( _visitor );
  };
};

template<typename T>
class visit_list_impl
{
  typedef std::set<T*> visit_list_t;
  visit_list_t _list;
public:
  void add( T* t) { _list.insert(t); }

  template<typename TVisitor>
  void do_visit(TVisitor& visitor)
  {
    std::for_each(_list.begin(), _list.end(), detail::f_accept_visitor<TVisitor>(visitor) );
  }
};

template<typename T>
class visit_list_impl< type_list<T, empty_type> > 
{
  typedef std::set< T*> visit_list_t;
  visit_list_t _list;
public:
  void add( T* t) { _list.insert(t); }

  template<typename TVisitor>
  void do_visit(TVisitor& visitor)
  {
    std::for_each(_list.begin(), _list.end(), 
                  detail::f_accept_visitor<TVisitor>(visitor) );
  }

  void do_visit(empty_type){}
};

template<typename L, typename R >
class visit_list_impl< type_list<L, R> > 
  : public visit_list_impl<R>
{
  typedef visit_list_impl<R> super;
  typedef std::set< L*> visit_list_t;
  visit_list_t _list;
public:
  void add( L* t ) { _list.insert(t); }

  template<typename T>
  void add( T* t )  { super::add(t);}

  template<typename TVisitor>
  void do_visit(TVisitor& visitor)
  {
    std::for_each(_list.begin(), _list.end(),
                  detail::f_accept_visitor<TVisitor>(visitor) );
    super::do_visit(visitor);
  }
  void do_visit(empty_type){}
};
} }}

#endif
