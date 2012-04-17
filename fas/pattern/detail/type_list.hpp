//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_TYPE_LIST_DETAIL_HPP
#define FAS_PATTERNS_TYPE_LIST_DETAIL_HPP

#include <fas/pattern/conversion.hpp>
namespace fas{ namespace pattern { namespace detail {

template<typename T, typename L, bool>
struct type_count_helper;

template<typename T, typename L>
struct type_count_helper<T, L, false>
{
  enum { result = type_count<T, typename L::right_type>::result };
};

template<typename T, typename L>
struct type_count_helper<T, L, true>
{
  enum { result = 1 + type_count<T, typename L::right_type>::result };
};

template<typename T>
struct is_type_list
{
  enum { result = template_conversion2<T, type_list>::result != 0 };
};
/// ///////////////////////////////////////////////////

template<typename T, typename R, bool>
struct type_position_helper
{
  enum { result = type_position<T, R>::result + 1 } ;
};

template<typename T, typename R>
struct type_position_helper<T, R, true>
{
  enum { result = 0 } ;
};

/// ///////////////////////////////////////////////////

template<typename T, typename TList, bool>
struct erase_all_helper;

template<typename L, bool B>
struct erase_all_helper<empty_type, L, B>
{
  typedef L type;
  typedef empty_type removed;
};

template<typename T, bool B>
struct erase_all_helper<T, empty_type, B>
{
  typedef empty_type type;
  typedef empty_type removed;
};

template<typename T, typename L>
struct erase_all_helper<T, L, false >
{
  typedef type_list<
    typename L::left_type,
    typename erase_all<T, typename L::right_type>::type
  > type;

  typedef typename erase_all<T, typename L::right_type>::removed removed;
  
  /*typedef erase_all_helper<T, typename L::right_type, super_sub_class< T, typename R::left_type>::result > helper;
  typedef typename helper::removed removed;
  typedef type_list<L, typename helper::result > result;*/
};

template<typename T, typename L>
struct erase_all_helper<T, L, true >
{
  typedef typename erase_all<T, typename L::right_type>::type type;
  typedef type_list<
    typename L::left_type,
    typename erase_all<T, typename L::right_type>::removed
  > removed;
  /*typedef erase_all_helper<T, R, super_sub_class< T, typename R::left_type>::result > helper;
  typedef type_list<L, typename helper::removed> removed;
  typedef typename helper::result result;*/
};

/*
template<typename T, typename L, typename R>
struct erase_all_helper<T, type_list<L, R>, false >
{
  typedef erase_all_helper<T, R, super_sub_class< T, typename R::left_type>::result > helper;
  typedef typename helper::removed removed;
  typedef type_list<L, typename helper::result > result;
};

template<typename T, typename L, typename R>
struct erase_all_helper<T, type_list<L, R>, true >
{
  typedef erase_all_helper<T, R, super_sub_class< T, typename R::left_type>::result > helper;
  typedef type_list<L, typename helper::removed> removed;
  typedef typename helper::result result;
};

template<typename T, typename L>
struct erase_all_helper<T, type_list<L, empty_type>, false >
{
  typedef empty_type removed;
  typedef type_list<L, empty_type > result;
};

template<typename T, typename L>
struct erase_all_helper<T, type_list<L, empty_type>, true >
{
  typedef type_list<L, empty_type > removed;
  typedef empty_type result;
};
*/

/// ///////////////////////////////////////////////////

template<typename T, typename L, bool>
struct erase_helper;

template<typename L, bool B>
struct erase_helper<empty_type, L, B>
{
  typedef L type;
};

template<typename T, bool B>
struct erase_helper<T, empty_type, B>
{
  typedef empty_type type;
};

template<typename T, typename L>
struct erase_helper<T, L, true>
{
  typedef typename L::right_type type;
};


template<typename T, typename L>
struct erase_helper<T, L, false>
{
private:
  typedef typename L::left_type left_type;
  typedef typename L::right_type right_type;
  typedef typename erase<T, right_type>::type right_erased_type;
public:
  typedef type_list<left_type, right_erased_type > type ;
};



/// ///////////////////////////////////////////

template<typename F, typename S, bool, int>
struct erase_from_list_helper;

template<typename F, typename S, int C>
struct erase_from_list_helper<F, S, true, C>
{
  typedef typename erase_from_list<typename F::right_type, S, C>::type type;
};

template<typename F, typename S, int C>
struct erase_from_list_helper<F, S, false, C>
{
  typedef type_list<
            typename F::left_type,
            typename erase_from_list<typename F::right_type, S, C>::type
         > type;
};

/// ///////////////////////////////////////////////////

template<typename L, typename R, bool B>
struct intersection_helper;

template<typename L, typename R>
struct intersection_helper<L, R, true>
{
  typedef type_list<
    typename L::left_type,
    typename intersection< typename L::right_type, R>::type
  > type;
};

template<typename L, typename R>
struct intersection_helper<L, R, false>
{
  typedef typename intersection< typename L::right_type, R>::type type;
};

template<typename R>
struct intersection_helper<empty_type, R, false>
{
  typedef empty_type type;
};

template<typename R>
struct intersection_helper<empty_type, R, true>
{
  typedef empty_type type;
};

/// //////////////////////////////////////////////////

template<typename L, template<typename> class M>
struct for_each_helper;

template<typename L, typename R,  template<typename> class M>
struct for_each_helper< type_list<L, R>, M >
{
  typedef type_list< typename M<L>::type, typename for_each_helper<R, M>::type > type;
};

template<template<typename> class M>
struct for_each_helper< empty_type, M >
{
  typedef empty_type type;
};

/// //////////////////////////////////////////////////

template<typename T, bool B>
struct organize_helper;

template<typename T1, typename T2, bool B1, bool B2>
struct organize_helper2;

template<typename T>
struct organize_helper3
{
  typedef typename organize_helper2<
    typename T::left_type,
    typename T::right_type,
    is_type_list<typename T::left_type>::result!=0,
    is_type_list<typename T::right_type>::result!=0
  >::type type; 
};


template<bool B>
struct organize_helper<empty_type, B>
{
  typedef empty_type type;
};

template<typename T>
struct organize_helper<T, true>
{
  typedef typename organize_helper3<T>::type type;
};

template<typename T>
struct organize_helper<T, false>
{
  typedef type_list<T> type;
};


//-------- ������������� ���� ���� �� ����� empty_type


template<>
struct organize_helper2<empty_type, empty_type, false, false> { typedef empty_type type; };

template<typename T1>
struct organize_helper2<T1, empty_type, false, false>
{
  typedef type_list<T1> type;
};

template<typename T1>
struct organize_helper2<T1, empty_type, true, false>
{
  typedef typename organize_helper3<T1>::type type;
};

template<typename T2>
struct organize_helper2< empty_type, T2, false, false>
{
  typedef type_list<T2> type;
};

template<typename T2>
struct organize_helper2< empty_type, T2, false, true>
{
   typedef typename organize_helper3<T2>::type type;
};

//--- ��������� �������������
template<typename T1, typename T2>
struct organize_helper2< T1, T2, false, false>
{
  typedef type_list< T1, type_list<T2> > type;
};

template<typename T1, typename T2>
struct organize_helper2< T1, T2, true, false>
{
  typedef typename organize_helper2<
    typename T1::right_type, T2, 
    is_type_list<typename T1::right_type>::result!=0, false
  >::type right_organize;

  typedef typename organize_helper2<
    typename T1::left_type,
    right_organize,
    is_type_list<typename T1::left_type>::result!=0, 
    is_type_list<right_organize>::result!=0 
  >::type type;
};

template<typename T1, typename T2>
struct organize_helper2< T1, T2, false, true>
{
  typedef type_list< T1, typename organize_helper3<T2>::type> type;
};

template<typename T1, typename T2>
struct organize_helper2< T1, T2, true, true>
{
  typedef typename organize_helper3<T2>::type first_type;
  
  typedef typename organize_helper2<
      typename T1::right_type,
      first_type,
      is_type_list< typename T1::right_type >::result!=0,
      is_type_list< first_type >::result!=0
  >::type second_type;

  typedef typename organize_helper2<
      typename T1::left_type,
      second_type,
      is_type_list<typename T1::left_type>::result!=0,
      is_type_list< second_type >::result!=0
  >::type type;


};



/*
template<bool B>
struct organize_helper<empty_type, empty_type, B>
{
  typedef empty_type type;
};

template<typename R, bool B>
struct organize_helper<empty_type, R, B>
{
  typedef typename organize<R>::type type;
};

template<typename L>
struct organize_helper<L, empty_type, true>
{
  typedef typename organize<L>::type type;
};

template<typename L>
struct organize_helper<L, empty_type, false>
{
  typedef L type;
};*/

/*
template<typename L, typename R>
struct organize_helper<L, R, false>
{
  typedef type_list<L, typename organize<R>::type> type;
};

template<typename L, typename R>
struct organize_helper<L, R, true>
{
  typedef typename merge<
    typename organize<L>::type,
    typename organize<R>::type
  >::type type;
  // typedef type_list<L, typedef organize<R>::type> type;
};
*/
/*
  typedef typename detail::select_helper<
    T,
    L,
    super_sub_class<T, typename L::left_type >::result 
  >::type type;*/

/// /////////////////////////////////////////////////

template<typename T, int P>
struct reverse_helper
{
  typedef typename type_list_nth_cast< P, T  >::type::left_type last_type;
  typedef type_list<
    last_type,
    typename reverse_helper<T, P -1>::type
  > type;
};

template<typename T>
struct reverse_helper<T, 0>
{
  typedef typename type_list_nth_cast< 0, T  >::type::left_type last_type;
  typedef type_list<last_type> type;
};

// typedef typename detail::reverse_helper< T, length<T>::result - 1 >::type type;

/// /////////////////////////////////////////////////

template<typename T, typename L, typename R, bool B>
struct select_helper;

template<typename T, typename L, typename R>
struct select_helper<T, L, R, true>
{
  typedef type_list< 
    L,
    typename detail::select_helper<
      T,
      typename R::left_type,
      typename R::right_type,
      super_sub_class<T, typename R::left_type >::result 
    >::type
  > type;
};

template<typename T, typename L, typename R>
struct select_helper<T, L, R, false>
{
  typedef typename detail::select_helper<
    T,
    typename R::left_type,
    typename R::right_type,
    super_sub_class<T, typename R::left_type >::result 
  >::type type;
};

template<typename T, typename L>
struct select_helper<T, L, empty_type, true>
{
  typedef type_list<L> type;
};

template<typename T, typename L>
struct select_helper<T, L, empty_type, false>
{
  typedef empty_type type;
};

template< template<typename> class T, typename L, typename R, bool B>
struct select1_helper;

template< template<typename> class T, typename L, typename R>
struct select1_helper<T, L, R, true>
{
  typedef type_list< 
    L,
    typename detail::select1_helper<
      T,
      typename R::left_type,
      typename R::right_type,
      template_conversion1<typename R::left_type, T >::result
    >::type
  > type;
};

template<template<typename> class T, typename L, typename R>
struct select1_helper<T, L, R, false>
{
  typedef typename detail::select1_helper<
    T,
    typename R::left_type,
    typename R::right_type,
    template_conversion1<typename R::left_type, T >::result
  >::type type;
};

template<template<typename> class T, typename L>
struct select1_helper<T, L, empty_type, true>
{
  typedef type_list<L> type;
};

template<template<typename> class T, typename L>
struct select1_helper<T, L, empty_type, false>
{
  typedef empty_type type;
};



/// ///////////////////////////////////////////////////

template<typename L, typename R, bool B>
struct unique_helper;

template<typename L, typename R>
struct unique_helper<L, R, false>
{
  typedef type_list<
    L,
    typename unique<R>::type
  > type;
};

template<typename L, typename R>
struct unique_helper<L, R, true>
{
  typedef typename unique<R>::type type;
};

/*
template<typename L, int C>
struct unique_helper
{
  typedef typename unique_helper<
    typename L::right_type,
    type_count<
      typename L::right_type::left_type,
      typename L::right_type::right_type
    >::result - 1
  >::type type;
};

template<int C>
struct unique_helper<empty_type, C>
{
  typedef empty_type type;
};

template<typename L>
struct unique_helper<L, 0>
{
  typedef type_list<
    typename L::left_type,
    typename unique_helper<
      typename L::right_type,
      type_count<
        typename L::right_type::left_type,
        typename L::right_type
      >::result - 1
    >::type
  > type;
};
*/

/// ///////////////////////////////////////////////////

template<typename T, typename L, bool>
struct unique_from_list_helper;

template<typename T, typename L>
struct unique_from_list_helper<T, L, true>
{
  typedef typename erase<T, L>::type first_erase;
  typedef typename unique_from_list_helper<
    T, first_erase, type_count<T, first_erase>::result >= 2
  >::type type;
};

template<typename T, typename L>
struct unique_from_list_helper<T, L, false>
{
  typedef L type;
};

/// ///////////////////////////////////////////////////

template<typename T, typename L, typename R, bool>
struct type_list_cast_helper
{
  // typedef typename type_list_cast<L, R>::type type;
  typedef R type;
};

template<typename T, typename L, typename R>
struct type_list_cast_helper<T, L, R, false>
{
  typedef typename type_list_cast<T, typename R::right_type>::type type;
};

/// ///////////////////////////////////////////////////

/*
template<typename T>
struct extract_type_list
{
};*/

template<typename T, bool B>
struct type_list_traits_helper2
{
  typedef type_list< T > type;
};

template<typename T>
struct type_list_traits_helper2<T, true>
{
  typedef T type;
};

template<typename T>
struct type_list_traits_helper
{
  typedef typename type_list_traits_helper2< 
    T, is_type_list<T>::result
  >::type type;
};


template<>
struct type_list_traits_helper<empty_type>
{
  typedef empty_type type;
};

/*template<typename L, typename R>
struct type_list_traits_helper< type_list<L, R> >
{
  typedef type_list< 
            typename type_list_traits<L>::type,
            typename type_list_traits<R>::type
          > type;
};
*/
template< typename T1, typename T2, typename T3,
          typename T4, typename T5, typename T6,
          typename T7, typename T8,
          typename T9, typename T10 >
struct type_list_traits_helper< type_list_n<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> >
{
  typedef typename type_list_n<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>::type type;
};


/// ////////////////////////////////////////////////////
template<typename T>
struct type_list_n_helper
{
  typedef T type;
};

template<typename T1, typename T2, typename T3,
         typename T4, typename T5, typename T6,
         typename T7, typename T8,
         typename T9, typename T10 >
struct type_list_n_helper< type_list_n<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> >
{
  typedef typename type_list_n<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>::type type;
};

}}}

#endif
