// ага
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_CONVERSION_HPP
#define FAS_PATTERNS_CONVERSION_HPP

namespace fas{ namespace pattern {

template<typename T>
struct type2type
{
  typedef T type;
};

template<int i>
struct int2type
{
  /*struct holder{ enum {value=i}; }; */
};

template<bool i>
struct bool2type{ /*struct holder{ enum {value=i}; };*/ };

namespace detail{

struct conversion_base
{
  typedef char small_type;
  typedef class big_tag { char dummy[2];} big_type;
};

template<typename T, typename U>
struct conversion_helper
	: conversion_base
{
  static small_type test(U);
  static big_type test(...);
  static T makeT();
};

template<typename T, template<typename> class U>
struct conversion_helper_t1
	: conversion_base
{
  template<typename P>
  static small_type test(const volatile U<P>*);
  static big_type test(...);
  static const volatile T* makeT();
};

template<typename T, template<typename, typename> class U>
struct conversion_helper_t2
	: conversion_base
{
  template<typename P1, typename P2>
  static small_type test(const volatile U<P1, P2>*);
  static big_type test(...);
  static const volatile T* makeT();
};

template<typename T, template<typename, template<typename> class > class U>
struct conversion_helper_t2t
	: conversion_base
{
  template<typename P1, template<typename> class P2>
  static small_type test(const volatile U<P1, P2>*);
  static big_type test(...);
  static const volatile T* makeT();
};

}

template<typename T, typename U>
struct conversion
{
  typedef detail::conversion_helper<T, U> helper;
  enum { result = (sizeof( helper::test( helper::makeT() ) )==sizeof( typename helper::small_type)) };
  enum { some_type = 0};
};

template<typename T>
struct conversion<T, T>
{
  enum { result = 1 };
  enum { some_type = 1};
};

template<typename T, typename U>
struct super_sub_class
{
  enum { result = conversion<const volatile U*, const volatile T*>::result &&
                  !conversion<const volatile T*, const volatile void*>::some_type };
};

template<typename T, template<typename> class U>
struct template_conversion1
{
  typedef detail::conversion_helper_t1<T, U> helper;
  enum { result = (sizeof( helper::test( helper::makeT() ) )==sizeof( typename helper::small_type)) };
  enum { some_type = 0};

	/*
  typedef detail::conversion_base super;
  typedef template_conversion1<T, U> self;

private:
  template<typename P1>
  static super::small_type test1(const volatile U<P1>*);
  static super::big_type test1(...);
  static const volatile T* makeT();
public:
  enum { result = (sizeof(self::test1(self::makeT()))==sizeof(super::small_type)) };
  enum { some_type = 0};
  */
};

template<typename P1, template<typename> class U>
struct template_conversion1< U<P1>, U >: detail::conversion_base 
{
  enum { result = 1 };
  enum { some_type = 1};
};


template<typename T, template<typename, typename> class U>
struct template_conversion2 // : detail::conversion_base 
{
  typedef detail::conversion_helper_t2<T, U> helper;
  enum { result = (sizeof( helper::test( helper::makeT() ) )==sizeof( typename helper::small_type)) };
  enum { some_type = 0};

	/*
  typedef detail::conversion_base super;
  typedef template_conversion2<T, U> self;
private:
  template<typename P1, typename P2>
  static super::small_type test1(const volatile U<P1, P2>*);
  static super::big_type test1(...);
  static const volatile T* makeT();
public:
  enum { result = (sizeof(self::test1(self::makeT()))==sizeof(super::small_type)) };
  enum { some_type = 0};
  */
};

template<typename P1, typename P2, template<typename, typename> class U>
struct template_conversion2< U<P1, P2>, U >: detail::conversion_base 
{
  enum { result = 1 };
  enum { some_type = 1};
};


/*

*/
template<typename T, template<typename, template<typename> class > class U>
struct template_conversion2t1 // : detail::conversion_base 
{
  typedef detail::conversion_helper_t2t<T, U> helper;
  enum { result = (sizeof( helper::test( helper::makeT() ) )==sizeof(typename helper::small_type)) };
  enum { some_type = 0};

	
	/*
  typedef detail::conversion_base super;
  typedef template_conversion2<T, U> self;

private:
  template<typename P1, template<typename> class P2>
  static super::small_type test1(const volatile U<P1, P2>*);
  static super::big_type test1(...);
  static const volatile T* makeT();
public:
  enum { result = (sizeof(self::test1(self::makeT()))==sizeof(super::small_type)) };
  enum { some_type = 0};
  */
};

template<typename P1, template<typename> class P2, template<typename, template<typename> class > class U>
struct template_conversion2t1< U<P1, P2>, U >: detail::conversion_base 
{
  enum { result = 1 };
  enum { some_type = 1};
};


}}

#endif
