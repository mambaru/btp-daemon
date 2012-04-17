//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_ADVICE_SPECIALIZATION_HPP
#define FAS_AOP_ADVICE_SPECIALIZATION_HPP

namespace fas{ namespace aop {

/** Специализация для обозначения класса-адвайса без тега.
  * Всегда создается уникальный тег
  * @param A - класс-адвайс
  * @typedef tags_list - список тегов
  * @typedef advice_class - класс-адвайс 
  */
template<typename A, bool B>
struct advice<A, ap::empty_type, B>
  : ap::type2type<A>, A
{
  typedef typename tag_list_traits< ap::type2type<A> >::type all_tag_list;
  typedef typename tag_list_traits< ap::type2type<A> >::tag_list tag_list;
  typedef typename tag_list_traits< ap::type2type<A> >::gtag_list gtag_list;

  typedef A advice_class;
  enum { make_advice_tag = B};

  template< typename LL /*= ap::empty_type*/, typename AA /*= A*/, bool BB /*= B*/>
  struct rebind
  {
    typedef typename detail::rebind_advice<LL, AA, BB>::type type;
  };

  advice_class& get_class() { return static_cast<advice_class&>(*this);}
  const advice_class& get_class() const { return static_cast<const advice_class&>(*this);}

};


template<typename L, typename A >
struct advice<L, A, true>
  : tag_list_traits< ap::type_list< ap::type2type<A> , L > >::hierarchy, A
{
  // ? разобраться почему если подставить вместо L дубликаты type2type появляются в списке тегов
  // А потому что при rebind мы берем all_tag_list в котором уже есть тег ap::type2type<A>
  // и если BB устнавливем в true то генерим еще один 
/*
  typedef ap::type_list< ap::type2type<A> , L > extended_list; 

  typedef typename tag_list_traits< extended_list >::type all_tag_list;
  typedef typename tag_list_traits< extended_list >::tag_list tag_list;
  typedef typename tag_list_traits< extended_list >::gtag_list gtag_list;

#warning !!!
*/

/// Ахтунг!!! при ребинде с all_tag_list и BB=true, но с новым классом тег ap::type2type<A> теряется а генерится новый!!!

  // typedef ap::type_list< ap::type2type<A> , L > extended_list; // ? разобраться почему если подставить вместо L дубликаты type2type появляются в списке тегов

  typedef typename tag_list_traits< L >::type all_tag_list;
  typedef typename tag_list_traits< L >::tag_list tag_list;
  typedef typename tag_list_traits< L >::gtag_list gtag_list;

  typedef A advice_class;
  enum { make_advice_tag = true};

  template< typename LL /*= L*/, typename AA /*= A*/, bool BB /*= true*/>
  struct rebind
  {
    typedef typename detail::rebind_advice<LL, AA, BB>::type type;
  };

  advice_class& get_class() { return static_cast<advice_class&>(*this);}
  const advice_class& get_class() const { return static_cast<const advice_class&>(*this);}

};

/// Не можем создавать адвайс без тегов
template< typename A >
struct advice<ap::empty_type, A, false>;


template<>
struct advice_list_traits<ap::empty_type>
{
  typedef ap::empty_type type;
  typedef ap::empty_type common_list;
};

}}

#endif
