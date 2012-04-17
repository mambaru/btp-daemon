//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_ADVICE_DETAIL_HPP
#define FAS_AOP_ADVICE_DETAIL_HPP

namespace fas{ namespace aop { namespace detail {

template< typename LL, typename AA , bool BB >
struct rebind_advice
{
  typedef advice<LL, AA, BB> type;
};

template<typename AA>
struct rebind_advice<ap::empty_type, AA, false>
{
  typedef ap::empty_type type;
};


/// //////////////////////////////////////////////

template< typename L >
struct extract_tags_helper
{
  typedef ap::type_list< 
    typename L::left_type::tag_list, 
    typename extract_tags_helper<
      typename L::right_type
    >::tag_list 
  > tag_list;

  typedef ap::type_list< 
    typename L::left_type::gtag_list, 
    typename extract_tags_helper<
      typename L::right_type
    >::gtag_list 
  > gtag_list;

  typedef ap::type_list< 
    typename L::left_type::all_tag_list, 
    typename extract_tags_helper<
      typename L::right_type
    >::all_tag_list 
  > all_tag_list;
};

template<>
struct extract_tags_helper<ap::empty_type>
{
  typedef ap::empty_type tag_list;
  typedef ap::empty_type gtag_list;
  typedef ap::empty_type all_tag_list;
};


/// //////////////////////////////////////////////////////////////
/// для make_alias

template<typename T>
struct is_alias
{
  enum { result = ap::template_conversion2<T, alias>::result != 0 };
};

/// //////////////////////////////////////////////////////////////
/// extract_alias_list

template< typename T, bool B>
struct extract_alias_list_helper;

template< typename T>
struct extract_alias_list
{
  typedef extract_alias_list_helper< T, is_alias<typename T::left_type>::result > helper;
  typedef typename helper::alias_list alias_list;
  typedef typename helper::advice_list advice_list;
  // теги tag_list всех alias
  typedef typename helper::alias_tags alias_tags;
};

template<>
struct extract_alias_list<ap::empty_type>
{
  typedef ap::empty_type alias_list;
  typedef ap::empty_type advice_list;
  typedef ap::empty_type alias_tags;
};

template< typename T>
struct extract_alias_list_helper<T, true >
{
  enum { check = ap::static_check< is_alias<typename T::left_type>::result >::result };

  typedef extract_alias_list< typename T::right_type > exractor;
  typedef ap::type_list< typename T::left_type, typename exractor::alias_list > alias_list;
  typedef typename exractor::advice_list advice_list;

//-
  typedef ap::type_list< typename T::left_type::tag_list, typename exractor::alias_tags > alias_tags;
};


template< typename T>
struct extract_alias_list_helper<T, false >
{
  typedef extract_alias_list< typename T::right_type > exractor;
  typedef ap::type_list< typename T::left_type, typename exractor::advice_list > advice_list;
  typedef typename exractor::alias_list alias_list;

  typedef typename exractor::alias_tags alias_tags;
};



/// /////////////////////////////////////////////////////////

/** @param L1 advice list
  * @param L2 alias list
  */
template<typename L1, typename L2>
struct apply_alias_helper;

template<typename L1, typename L2, bool B1, bool B2>
struct apply_alias_helper2;

template<typename T, typename L>
struct apply_alias_for_advice;


template<typename L1, typename L2>
struct apply_alias_helper
{
  typedef typename apply_alias_helper2<
    L1, L2, 
    ap::is_type_list<L1>::result, 
    ap::is_type_list<L2>::result
  >::type type;
};

template<typename L1, typename L2>
struct apply_alias_helper2<L1, L2, true, true>
{
  typedef typename apply_alias_helper2<
    typename L1::left_type, L2,
    false,
    true 
  >::type left;

  typedef typename apply_alias_helper2<
    typename L1::right_type, L2,
    ap::is_type_list<typename L1::right_type>::result,
    true
  >::type right;

  typedef ap::type_list< left, right > type;
};

/**
T - advice
L - список алиасов
*/
template<typename T, typename L>
struct apply_alias_helper2<T, L, false, true >
{
  typedef typename L::left_type left_type;
  typedef typename L::right_type right_type;
  /// удаляем теги алиасов перед применением
  typedef typename left_type::tag_list alias_tags;

  /*
  typedef typename ap::erase_from_list< 
            typename T::all_tag_list,   // Откуда удаляем
            alias_tags,   // что удаляем
            0
          >::type erased_tag_list;

  typedef typename T::template rebind<
            erased_tag_list,
            typename T::advice_class,
            T::make_advice_tag
          >::type rebinded; // м.б. empty_type, тогда беда, тип то потеряли!!!*/


  typedef typename apply_alias_for_advice< /*rebinded*/T, left_type >::type apply_first;

  typedef typename apply_alias_helper2<
    apply_first, 
    right_type,
    false,
    ap::is_type_list<right_type>::result
  >::type type;

};



template<typename T>
struct apply_alias_helper2<T, ap::empty_type, false, false >
{
  typedef T type;
};

template<typename T>
struct apply_alias_helper2<T, ap::empty_type, true, false >
{
  typedef T type;
};

template<typename L>
struct apply_alias_helper2<ap::empty_type, L, false, true >
{
  typedef ap::empty_type type;
};

template<typename L>
struct apply_alias_helper2<ap::empty_type, L, false, false >
{
  typedef ap::empty_type type;
};

template<>
struct apply_alias_helper2<ap::empty_type, ap::empty_type, false, false >
{
  typedef ap::empty_type type;
};

/// ///////////////////////////

template<typename A, typename L, bool>
struct apply_alias_for_advice_helper;


/**  @param A1 advice
  *  @param A2 alias
  **/
template<typename A1, typename A2>
struct apply_alias_for_advice
{
  typedef A2 alias_type;
  typedef typename A1::all_tag_list advice_tag_list;
  typedef typename alias_type::tag alias_tag;
  typedef typename alias_type::tag_list alias_tag_list;

  // =
  typedef typename ap::erase_from_list< 
            advice_tag_list,   // Откуда удаляем
            alias_tag_list,   // что удаляем
            0
          >::type erased_tag_list;

  typedef typename A1::template rebind<
            erased_tag_list,
            typename A1::advice_class,
            A1::make_advice_tag
          >::type rebinded; // м.б. empty_type, тогда беда, тип то потеряли!!!
  // =

  typedef typename apply_alias_for_advice_helper<
            /*A1*/rebinded,
            alias_tag_list,
            ap::type_count<alias_tag, advice_tag_list>::result!=0
          >::type type;
};

template<typename A2>
struct apply_alias_for_advice< ap::empty_type, A2 >
{
  typedef ap::empty_type type;
};

template<typename A, typename L>
struct apply_alias_for_advice_helper<A, L, true>
{
  typedef typename A::all_tag_list advice_tag_list;
  typedef typename tag_list_traits<L>::type alias_tag_list;
  typedef typename ap::merge<advice_tag_list, alias_tag_list>::type new_tag_list;
  typedef typename A::template rebind<
                        new_tag_list,
                        typename A::advice_class,
                        A::make_advice_tag
                       >::type type;
};


template<typename A, typename L>
struct apply_alias_for_advice_helper<A, L, false>
{
  typedef A type;
};


















/*

/// Применяет алиас к списку адвайсов
template<typename T, typename L>
struct apply_alias;


*/




/*


template<typename T, typename L, typename R>
struct apply_alias< T, alias<L, R> >
{
  typedef ap::type_list<
    typename apply_alias_for_advice< typename T::left_type, alias<L, R> >::type,
    typename apply_alias< typename T::right_type, alias<L, R> >::type
  > type;
};


template<typename L, typename R>
struct apply_alias< ap::empty_type, alias<L, R> >
{
  typedef ap::empty_type type;
};



template<typename T>
struct not_empty{};

template<>
struct not_empty<ap::empty_type>;
*/





}}}

#endif
