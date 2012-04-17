#ifndef FAS_PATTERN_DYNAMIC_INTERFACE_HPP
#define FAS_PATTERN_DYNAMIC_INTERFACE_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/pattern/scatter_hierarchy.hpp>

namespace fas{ namespace pattern {

template<typename T>
struct interface_pointer;

template<typename L, template<typename> class PM >
struct dynamic_pointer;

template<typename I>
struct interface_pointer_maker;

template<typename H, typename I>
inline H interface_cast(I* i);

template<typename H, typename I>
inline H& interface_cast(H& h, I* i);


template<typename L, template<typename> class IPM = interface_pointer_maker >
struct dynamic_interface;

}}

/// ///////////////////////////////////////////////
#include <fas/pattern/detail/dynamic_interface.hpp>

namespace fas{ namespace pattern {

template<typename T>
struct interface_pointer: type2type<T>
{
  typedef T interface_type;
  interface_pointer(): _ptr(0) {};
  interface_pointer(interface_type* ptr): _ptr(ptr) {};
  interface_pointer<T>& operator = (T* p) 
  {
    _ptr = p;
    return *this;
  }
  operator T*() { return _ptr; }
  T* operator -> () { return _ptr; }
  const T* operator -> () const { return _ptr; }

  T* ptr () { return _ptr; }
  const T* ptr () const { return _ptr; }

private:
  T* _ptr;
};

template<typename L, template<typename> class PM >
struct dynamic_pointer
  : private scatter_hierarchy< typename for_each<L, PM>::type >
{
  typedef scatter_hierarchy< typename for_each<L, PM>::type > super;
  typedef typename super::type_list_type pointer_list;
  typedef L interface_list;

  dynamic_pointer(){}
  // TODO: not testing

  // для инициализации null
  dynamic_pointer(int){}

  /*
  template<typename I>
  dynamic_pointer(I* i)
  {
    interface_cast(*this, i);
  }*/

  template<typename I, typename IL>
  dynamic_pointer(I* i)
  {
    interface_cast<IL>(*this, i);
  }

  template<typename T>
  const typename left_cast< type2type<T>, pointer_list>::type& 
  get() const
  {
    return super::template get< type2type<T> >();
  }

  template<typename T>
  typename left_cast< type2type<T>, pointer_list>::type& 
  get()
  {
    return super::template get< type2type<T> >();
  }

  void reset()
  {
		_set_null(super::left(), super::right());
  }

private:
  template<typename LL, typename RR>
	void _set_null(LL& left, RR& right)
  {
    left = 0;
    _set_null(right.left(), right.right());
  }

  template<typename LL>
	void _set_null(LL& left, scatter_hierarchy<empty_type>)
  {
    left = 0;
  }
};

template<typename I>
struct interface_pointer_maker
{
  typedef interface_pointer<I> type;
};

template<typename H, typename I>
inline H interface_cast(I* i)
{
  typedef typename I::interface_list interface_list;
  H h;
  return detail::interface_cast_helper(h, i, interface_list() );
}

template<typename H, typename L, typename I>
inline H interface_cast(I* i)
{
  typedef typename type_list_traits<L>::type interface_list;
  H h;
  return detail::interface_cast_helper(h, i, interface_list() );
}

template<typename L, typename H, typename I>
inline H& interface_cast(H& h, I* i)
{
  typedef typename type_list_traits<L>::type interface_list;
  return detail::interface_cast_helper(h, i, interface_list() );
}

template<typename H, typename I>
inline H& interface_cast(H& h, I* i)
{
  typedef typename I::interface_list interface_list;
  return detail::interface_cast_helper(h, i, interface_list() );
}




template<typename L, template<typename> class IPM >
struct dynamic_interface
  : public scatter_hierarchy< typename type_list_traits<L>::type >
{
  typedef typename type_list_traits<L>::type interface_list;
  typedef dynamic_pointer< interface_list, IPM > pointer;
  operator pointer() { return interface_cast<pointer>(this); }
};


}}

#endif
