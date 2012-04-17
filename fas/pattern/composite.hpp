//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_COMPOSITE_HPP
#define FAS_PATTERNS_COMPOSITE_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/pattern/nth_cast.hpp>
#include <fas/pattern/linear_hierarchy.hpp>

#include <map>
#include <string>


namespace fas{ namespace pattern {

template<typename T>
struct child_traits
{
  typedef T type;
};

template<typename L, typename R>
struct child_traits< type_list<L, R> >
{
  typedef linear_hierarchy< type_list<L, R> > type;
};

template <typename _Key, typename _Tp,
          template<typename, typename, typename, typename> class _TMap = std::map,
          typename _Compare = std::less<_Key>,
          typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
class map_container:
  public _TMap<_Key, typename child_traits<_Tp>::type, _Compare, _Alloc >
{
};

template<typename TLeaf, typename TBase>
class composite:  public TBase
{
public:
  typedef typename child_traits<TLeaf>::type leaf_type;
  typedef composite< TLeaf, TBase > this_type;
  typedef TBase super;
  typedef this_type composite_type;
public:

  composite(){}

  composite(const this_type& v )
    : super( static_cast<const super&>(v) )
  {}

  template<typename T>
  inline this_type& operator = ( const T& value)
  {
    _leaf = value;
    return *this;
  }

  inline leaf_type& leaf(){ return _leaf; }

  inline const leaf_type& leaf() const { return _leaf; }

  template< int i>
  inline typename nth_cast<i, leaf_type>::type::value_type& get()
  {
    return _leaf.get<i>();
  };

  template< int i>
  inline const typename nth_cast<i, leaf_type>::type::value_type& get() const
  {
    return _leaf.get<i>();
  };

  template <typename T>
  inline T& get()
  {
    return _leaf.get<T>();
  }


  template <typename T>
  inline const T& get() const
  {
    return _leaf.get<T>();
  }

  inline leaf_type& get() { return _leaf;}

  inline const leaf_type& get() const { return _leaf;}

private:
  leaf_type _leaf;
};

}}
#endif
