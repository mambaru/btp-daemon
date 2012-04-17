#ifndef FAS_PATTERN_DYNAMIC_INTERFACE_DETAIL_HPP
#define FAS_PATTERN_DYNAMIC_INTERFACE_DETAIL_HPP

#include <iostream>

namespace fas{ namespace pattern {

namespace detail
{

template<bool>
struct interface_cast_helper_set
{
  template<typename T, typename I>
  static void set(T& t, I* i)
  {
    t = i;
  }

  template<typename I>
  static void set(empty_type, I*) 
  {
  }

};

template<>
struct interface_cast_helper_set<true>
{
  template<typename T, typename I>
  static void set(T& t, I* i)
  {
    t = i->template get<typename T::interface_type>();
  }

  template<typename I>
  static void set(empty_type, I*) 
  {
    
  }

};


template<typename H, typename I>
inline H& interface_cast_helper(H& h, I*, empty_type) { return h; }

template<typename H, typename I, typename L, typename R>
inline H& interface_cast_helper(H& h, I* i, type_list<L, R>)
{
  interface_cast_helper_set< template_conversion2t1< I, dynamic_pointer>::result >::set( h.template get<L>(), i);
  return interface_cast_helper(h, i, R() );
};

}

}}

#endif
