//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_TYPE_LIST_HPP
#define FAS_PATTERNS_TYPE_LIST_HPP

#include <fas/pattern/conversion.hpp>
#include <fas/pattern/static_check.hpp>

namespace fas{ namespace pattern {

struct empty_type;

template< typename L, typename R = empty_type >
struct type_list;

template<typename T, typename L>
struct erase;

template<typename T, typename L>
struct erase_all;

template<typename F, typename S, int C>
struct erase_from_list;

template<typename L, template<typename> class M>
struct for_each;

template<typename L, typename R>
struct intersection;

template<typename L>
struct is_type_list;

template<typename T, typename L>
struct left_cast;

template<typename TL>
struct length;

template<typename L, typename R>
struct merge;

template<typename T>
struct organize;

template<typename T>
struct reverse;

template<typename T, typename L>
struct select;

/** right type list wrapper */
/*template< typename T >
struct rtlw;

template< typename T, bool B>
struct make_rtlw;
*/

template<typename T, typename L>
struct type_count;

template<typename T, typename L>
struct type_list_cast;

template<int i, typename L>
struct type_list_nth_cast;

template<typename T>
struct type_list_traits;

template<typename T, typename L>
struct type_position;

template<typename T>
struct unique;

template<typename T, typename L>
struct unique_from_list;

template< typename T1 = empty_type, typename T2 = empty_type, typename T3 = empty_type,
          typename T4 = empty_type, typename T5 = empty_type, typename T6 = empty_type,
          typename T7 = empty_type, typename T8 = empty_type,
          typename T9 = empty_type, typename T10 = empty_type>
struct type_list_n;

}}



#include <fas/pattern/detail/type_list.hpp>

namespace fas{ namespace pattern {

struct empty_type
{
  typedef empty_type left_type;
  typedef empty_type right_type;
};

template< typename L, typename R >
struct type_list
{
  typedef L left_type;
  typedef R right_type;
  typedef type_list<L, R> self;
};

/*
template<typename T>
struct tlw;

template<typename T, bool>
struct mtlw_helper{ typedef tlw<T> type;};

template<typename T>
struct mtlw_helper<T, false>{ typedef T type;};

template<typename T>
struct mtlw
{
  typedef typename T::left_type left_type;
  struct _rt: T::right_type{};
  typedef typename mtlw_helper<_rt, is_type_list<_rt>::result >::type right_type;
};*/

/*
template<typename L, typename R>
struct mtlw< type_list<L, R> >
{
  typedef L left_type;
  typedef R right_type;
};*/
/*
template<>
struct mtlw< empty_type >
{
  typedef empty_type left_type;
  typedef empty_type right_type;
};
*/
/*
template<typename T>
struct make_tlw
{
  struct type: tlw<T> {};
}
*/
/*
template<typename T>
struct tlw
  : type_list< 
      typename mtlw<T>::left_type,  
      typename mtlw<T>::right_type
  >
{};

template<>
struct tlw<empty_type>: empty_type {};
*/


/*
template<typename L, int P = length<L>::result>
struct tlw
  : type_list<empty_type>
{
  typedef typename type_list_nth_cast< length<L>::result - P, L>::type::left_type left_type;
  typedef tlw<L, P-1> right_type;
};

template<typename L>
struct tlw<L, 1>
  : type_list< empty_type >
{
  typedef typename type_list_nth_cast<length<L>::result - 1, L>::type::left_type left_type;
  typedef empty_type right_type;
};

template<typename L >
struct tlw<L, 0>
{
  typedef empty_type left_type;
  typedef empty_type right_type;
};
*/

/// /////////////////////////////////////////////////////////////////////////

/** Удаляет первое вхождение типа T из списка типов L.
  * Если типа T нет в списке, то возвращается исходный список.
  * Если удалены все элементы то возвращаеться empty_type.
  * Примечание: Если тип T является базовым для типов Х1..XN 
  * из списка L, то срабатывает первое вхождение типов T, Х1..XN.
  * @param T - удаляемый тип
  * @param L - организованный список типов
  * @typedef type - организованный список типов или empty_type
  */
template<typename T, typename L>
struct erase
{
  typedef typename detail::erase_helper<T, L, super_sub_class<T, typename L::left_type>::result >::type type;
};

/** Удаляет все вхождения типа T из списка типов L.
  * Если типа T нет в списке, то возвращается исходный список.
  * Если удалены все элементы то возвращаеться empty_type.
  * Примечание: Если тип T является базовым для типов Х1..XN из 
  * списка L, то срабатывают все вхождения типов T, Х1..XN.
  * @param T - удаляемый тип
  * @param L - организованный список типов
  * @typedef type - организованный список типов или empty_type
  */
template<typename T, typename L>
struct erase_all
{
  typedef detail::erase_all_helper<T, L, super_sub_class<T, typename L::left_type>::result > helper;
  typedef typename helper::removed removed;
  typedef typename helper::type type;

  // typedef typename detail::erase_helper<T, L, super_sub_class<T, L>::result >::type type;
};

/** 
  * Удаляет те типы (или производные от них) из списка типов S, которые 
  * перечислены в списке F, оставляя максимум C вхождений.
  * @param F - организованный список из которого удаляем 
  * @param S - организованный список удаляемых типов
  * @param C - максимальное числ вхождений, которое будет оставлено для каждого типа
  * 27.02.2008 заменил >= на >
  */
template<typename F, typename S, int C>
struct erase_from_list
{
  enum { result = (type_count<typename F::left_type, S>::result) > C };
  typedef typename detail::erase_from_list_helper< F, S, result, C >::type type;
};

/** Заменяет каждый элемент T списка типов L, на M<T>::type. Как правило используется чтобы 
  * "обернуть" все типы в некоторый wrapper-класс (holder). 
  * @param L - список типов
  * @param M<template> - шаблонный шаблонный параметр-преобразователь одного типа в другой 
  */
template<typename L, template<typename> class M>
struct for_each
{
  typedef typename detail::for_each_helper< typename type_list_traits<L>::type, M >::type type;
};

/** пересечение */
template<typename L, typename R>
struct intersection
{
  typedef typename detail::intersection_helper<L, R, type_count<typename L::left_type, R>::result >::type type1;
  typedef typename organize<type1>::type type;
};


template<typename L>
struct is_type_list
{
  enum { result = detail::is_type_list<L>::result };
};


/** Находит первое вхождение типа T в списке типов и возвращает его.
  * Примечание: Если тип T является базовым для типов Х1..XN из списка L,
  * то срабатывает первое вхождение типов T, Х1..XN.
  * @param T - искомый тип
  * @param L - организованный список типов
  * @typedef type - найденный тип
  */
template<typename T, typename L>
struct left_cast
{
  typedef typename type_list_cast<T, L >::type::left_type type;
};

template<int i, typename L>
struct type_list_nth_cast
{
  typedef typename type_list_nth_cast<i-1, typename L::right_type>::type type;
};

template<typename L>
struct type_list_nth_cast<0, L>
{
  typedef L type;
};


/** Определяет размер списка типов.
  * @param T - организованный список типов
  * @enum result - размер списка типов
  */
template<typename T>
struct length
{
  enum { result = type_position<empty_type, T >::result };
};

/** Объединяет два списка типов в один
  * @param L - организованный список типов
  * @param R - организованный список типов
  * @typedef type - объединенный организованный список типов
  */
template<typename L, typename R>
struct merge
{
  typedef typename organize< type_list<L, R> >::type type;
};

/** Преобразует неорганизованный список типов в организованный
  * @param T - неорганизованный список типов
  * @typedef type - организованный список типов
  */
template<typename T>
struct organize
{
  typedef typename detail::organize_helper< 
    T,
    template_conversion2<T, type_list>::result!=0
  >::type type;
};

template<typename L, typename R>
struct organize< type_list<L,R> >
{
  typedef typename detail::organize_helper< 
    type_list<L,R>, true
  >::type type;
};



template<typename T>
struct reverse
{
  typedef typename detail::reverse_helper< T, length<T>::result - 1 >::type type;
  // typedef typename type_list_nth_cast< length<T>::result - 1, T  >::type last_type;

  /*typedef typename T::left_type left_type;
  typedef typename T::rigth_type rigth_type;
  typedef type_list< >
  */
  /*typedef type_list<
    typename type_list_nth_cast< length<T>::result - 1, T  >::type,
    
  >*/
};






template<typename T, typename L>
struct select
{
  typedef typename detail::select_helper<
    T,
    typename L::left_type,
    typename L::right_type,
    super_sub_class<T, typename L::left_type >::result 
  >::type type;
};

template<template<typename> class T, typename L>
struct select1
{
  typedef typename detail::select1_helper<
    T,
    typename L::left_type,
    typename L::right_type,
    template_conversion1<typename L::left_type, T >::result 
  >::type type;
};

/**
  * Подсчитывает вхождения типа T в список типов L
  * Примечание: Если тип T является базовым для типов Х1..XN
  * из списка L, то срабатывают все вхождения типов T, Х1..XN.
  * @param T - искомый тип
  * @param L - организованный список типов 
  * @enum result - число вхождений
  */
template<typename T, typename L>
struct type_count
{
  enum {
    result =  detail::type_count_helper<
                T, L,
                super_sub_class<T, typename L::left_type >::result 
              >::result
  };
};

/** Возвращает посписок типов списка L, у которого в левой
  * части находиться тип T
  * Примечание: Если тип T является базовым для типов Х1..XN
  * из списка L, то срабатывает первое вхождение типов T, Х1..XN.
  * @param T - искомый тип
  * @param L - организованный список типов
  * @typedef type - организованный подсписок типов
  */
template<typename T, typename L>
struct type_list_cast
{
  typedef typename detail::type_list_cast_helper<
    T, typename L::left_type, L, 
    super_sub_class<
      T, 
      typename L::left_type
    >::result 
  >::type type;
};

/** Преобразует тип T в список типов. Если T - неорганизованный 
  * список типов, то организует его, если type_list_n то извлекает
  * список типов оттуда, в остальных случаях создает список 
  * type_list < T, empty_type >
  * @param T - список типов, type_list_n или другой тип
  * @typedef type организованный список типов
  * @typedef left_type - первый элемент списока типов
  * @typedef right_type - списока типов без первого элемента
  */
template<typename T>
struct type_list_traits
{
  typedef typename detail::type_list_traits_helper<T>::type raw_list;
  typedef typename organize<raw_list>::type type;
  //struct type: organize<raw_list>::type {};
};

/**
  * Определяет позицию первого вхождения типа T
  * в организованном списке типов L. Если типа
  * в списке нет, то ошибка компиляции
  * Примечание: Если тип T является базовым для типов Х1..XN, то 
  * срабатывает первое вхождение типов T, Х1..XN.
  * @param T - искомый тип
  * @param L - организованный список типов
  * @enum result число вхождений
  */
template<typename T, typename L>
struct type_position
{
  enum { 
    result =  detail::type_position_helper<
        T, 
        typename L::right_type, 
        super_sub_class<
          T, 
          typename L::left_type
        >::result 
    >::result  
  } ;
};

/** Удаленяет все дубликаты из списка типов, оставляя последнее вхождение
  * Примечание: Если тип X является базовым для типов Х1..XN из списка T, 
  * то срабатывают все вхождения типов X, Х1..XN.
  * @param T - организованный список типов
  * @typedef type организованный список типов 
  */

/*
template<typename L>
struct unique
{
    typedef typename detail::unique_helper<
      L, type_count<
        typename L::left_type, L 
      >::result - 1
    >::type type;

};
*/

template<typename L, typename R>
struct unique< type_list<L, R> >
{
  typedef typename detail::unique_helper<L, R, type_count<L, R>::result!=0 >::type type;
};


/** Удаленяет дубликаты заданных типов из списка типов, 
  * оставляя последнее вхождение.
  * Примечание: Если типы T1..TN из списка Т является базовым для типов Х1..XN, то 
  * срабатывают все вхождения типов T1..TN, Х1..XN.
  * @param T - организованный список типов,
  *            которые мы хотим сделать уникальными
  * @param L - организованный список типов, в которм будет 
  *            производиться поиск и удаление
  * @typedef type организованный список типов
  */
template<typename T, typename L>
struct unique_from_list
{
  typedef typename detail::unique_from_list_helper<
            typename T::left_type,
            L,
            type_count<
              typename T::left_type, 
              L
            >::result >= 2
          >::type L1_unique;

  typedef typename unique_from_list< typename T::right_type, L1_unique>::type type;
};


/** Создает организованный список типов из заданных параметров.
  * В качестве параметров могут быть неогрганизованные списки 
  * типов и type_list_n.
  * @param T1..T10 - любые типы или списки типов
  * @typedef type - организованный список типов
  */
template< typename T1, typename T2, typename T3,
          typename T4, typename T5, typename T6,
          typename T7, typename T8,
          typename T9, typename T10 >
struct type_list_n
{
  typedef typename detail::type_list_n_helper<T1>::type t1_type;
  typedef typename detail::type_list_n_helper<T2>::type t2_type;
  typedef typename detail::type_list_n_helper<T3>::type t3_type;
  typedef typename detail::type_list_n_helper<T4>::type t4_type;
  typedef typename detail::type_list_n_helper<T5>::type t5_type;
  typedef typename detail::type_list_n_helper<T6>::type t6_type;
  typedef typename detail::type_list_n_helper<T7>::type t7_type;
  typedef typename detail::type_list_n_helper<T8>::type t8_type;
  typedef typename detail::type_list_n_helper<T9>::type t9_type;
  typedef typename detail::type_list_n_helper<T10>::type t10_type;

  typedef typename organize<
            type_list<t1_type,
            type_list<t2_type,
            type_list<t3_type,
            type_list<t4_type,
            type_list<t5_type,
            type_list<t6_type,
            type_list<t7_type,
            type_list<t8_type,
            type_list<t9_type,
            type_list<t10_type>
            > > > > > > > > >
          >::type type;
};



}}

#include <fas/pattern/specialization/type_list.hpp>

#endif
