//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_VISITOR_HPP
#define FAS_PATTERNS_VISITOR_HPP

namespace fas{ namespace pattern {

template<typename T>
class visitable;

template<typename T, typename V = void>
class visitor;

template<typename L>
class visitor_list;

template<typename T>
struct visitor_list_traits;

template<typename T>
class visit_list;

template<typename T>
struct visit_list_traits;

template< typename TTag, typename TWhen, typename TWho = void>
class visiting;

template<typename TL>
class visiting_list;

}}


#include <fas/pattern/detail/visitor.hpp>

namespace fas{ namespace pattern {

template<typename T>
class visitable: public T
{
public:
  typedef T super;
  typedef visitable<super> self;

  visitable(){}

  visitable(const self& v )
    : super(static_cast<const super&>(v))
  {}

  template<typename P> self& operator=(const P& p)
  { static_cast<super&>(*this) = p; return *this;}

  template<typename V>
  void accept_visitor(V& visitor)
  {
    visitor.visit(this);
  }
};


template<typename T, typename V>
class visitor
 : public tag_list_traits< T >::hierarchy
 , public V
{
};

template<typename L>
class visitor_list
  : public scatter_hierarchy<typename type_list_traits<L>::type >
{
public:
  template<typename T>
  void visit(T* t)
  {
    _visit(t, *this);
  }
private:

  template<typename V, typename T>
  void _visit(V* v, T& t)
  {
    t.left().visit(v);
    _visit(v, t.right());
  }

  template<typename V>
  void _visit(V* v, empty_type)
  {
  }
};

template<typename T>
struct visitor_list_traits
{
  typedef visitor_list<T> type;
};



template<typename T>
class visit_list
: public detail::visit_list_impl< typename type_list_traits<T>::type >
{
};

template<typename T>
struct visit_list_traits
{
  typedef visit_list<T> type;
};

template< typename TTag, typename TWhen, typename TWho >
class visiting:
  public TTag,
  public visit_list_traits<TWhen>::type
{

public:

  typedef typename visit_list_traits<TWhen>::type super;
  typedef super visitables_type;
  typedef typename visitor_list_traits<TWho>::type visitor_list_type;

  const visitor_list_type& get_visitors() const {return _visitors;}

  visitor_list_type& get_visitors() {return _visitors;}

  void do_visit() { _do_visit( _visitors); }

private:

  visitor_list_type _visitors;

  template<typename T>
  void _do_visit(T& t)
  {
    super::do_visit(t.left());
    _do_visit(t.right());
  }

  void _do_visit(empty_type right) { }
};


// T = пара { кого посещают, список кем посещают }
template<typename TL>
class visiting_list
  : public /*tagable<*/ scatter_hierarchy<typename type_list_traits<TL>::type> /*>*/
{
public:
  typedef typename type_list_traits<TL>::type super;
  void do_visit()
  {
    _do_visit( *this);
  }

private:

  template<typename T>
  void _do_visit(T& t)
  {
    t.left().do_visit();
    _do_visit(t.right());
  }

  void _do_visit(empty_type right) { }
};

} }

#include <fas/pattern/specialization/visitor.hpp>

#endif
