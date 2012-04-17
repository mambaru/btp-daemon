//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_SCATTER_HIERARCHY_HPP
#define FAS_PATTERNS_SCATTER_HIERARCHY_HPP

#include <string>

#include <fas/pattern/type_list.hpp>

namespace fas{ namespace pattern {

template< typename T >
struct right_wrapper;

template<typename T>
struct scatter_hierarchy;

template<typename T>
struct virtual_scatter_hierarchy;

}}


namespace fas{ namespace pattern {

/** right type list wrapper */
/*template< typename T >
struct rtlw: T::right_type {};


template< typename T, bool B>
struct make_rtlw_hleper;

template< typename T>
struct make_rtlw_hleper<T, true>
{
  typedef rtlw<T> type;
};

template< typename T>
struct make_rtlw_hleper<T, false>
{
  typedef empty_type type;
};

template< typename T>
struct make_rtlw
{
  typedef typename make_rtlw_hleper< T, is_type_list<T>::result >::type type;
};


template<typename T>
struct shw
{
  typedef shw_helper<T, is_type_list<T>::result> helper;
  typedef typename 
};
*/

/** Класс для генерации распределенных иерархий. Распределенная иерархия, в
  * отличие от линейной, использует не агрегацию, а наследование от элементов
  * из списка типов. Если необходимо виртуальное наследование, то используйте
  * virtual_scatter_hierarchy. <b>Если в списке типов находяться типы одной 
  * иерархии, то базовые типы должны быть указанны раньше дочерних, т.к. 
  * доступ к дочерним элементам возможен по базовому типу.
  * @param T - организованный список типов
  * @see virtual_scatter_hierarchy virtual_scatter_hierarchy
  * @see linear_hierarchy linear_hierarchy
  */
template<typename L>
struct scatter_hierarchy
  : public L::left_type
  , public scatter_hierarchy< typename L::right_type >
{
  /** Список типов, на основе которого построена иерархия */
  typedef L type_list_type;

  /** Первый (левый) тип из списка типов */
  typedef typename L::left_type left_type;
  /** Тип распределенной иерархии построенной на основе
    * списка без первого типа */
  typedef scatter_hierarchy< typename L::right_type > right_type;

  typedef right_type super;

  /** Преобразует объект к правому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *          то возвращается ссылка на объект scatter_hierarchy<R>.
    */
  right_type& right(){ return static_cast<right_type&>(*this); }
  const right_type& right() const { return static_cast<const right_type&>(*this); }

  /** Преобразует объект к левому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *         то возвращается ссылка на объект L.
    */
  left_type& left() { return static_cast<left_type&>(*this); }
  const left_type& left() const { return static_cast<const left_type&>(*this); }

  /** Возвращает ссылку на объект типа T. Если в списке типов, по которому
    * строилась иерархия, такого типа нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return сыылка на объект типа T или производный
    */
  template <typename T>
  typename left_cast<T, type_list_type>::type& 
  get()
  {
    typedef typename type_list_cast< T, type_list_type >::type sublist;
    typedef scatter_hierarchy< sublist > T_hierarchy;
    return static_cast<typename T_hierarchy::left_type& >(*this);
  }

  /** Возвращает константную ссылку на объект типа T. Если в 
    * списке типов, по которому строилась иерархия, такого типа 
    * нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return константная сыылка на объект типа T или производный
    */
  template <typename T>
  const typename left_cast<T, type_list_type>::type& 
  get() const
  {
    typedef typename type_list_cast< T, type_list_type >::type sublist;
    typedef scatter_hierarchy< sublist > T_hierarchy;
    return static_cast< const typename T_hierarchy::left_type& >(*this);
  }
};


/** Класс для генерации виртуальных распределенных иерархий. 
  * Распределенная виртуальных иерархия использует виртуальное 
  * наследование от элементов из списка типов. 
  * <b>Если в списке типов находяться типы одной 
  * иерархии, то базовые типы должны быть указанны раньше дочерних, т.к. 
  * доступ к дочерним элементам возможен по базовому типу.
  * @param T - организованный список типов
  * @see scatter_hierarchy scatter_hierarchy
  */
template<typename L, typename R>
struct virtual_scatter_hierarchy< type_list<L, R> >
  : public virtual L
  , public virtual virtual_scatter_hierarchy< R >
{
  /** Список типов, на основе которого построена иерархия */
  typedef type_list<L, R> type_list_type;
  /** Первый (левый) тип из списка типов */
  typedef L left_type;
  /** Тип распределенной иерархии построенной на основе
    * списка без первого типа */
  typedef virtual_scatter_hierarchy< R > right_type;

  virtual ~virtual_scatter_hierarchy(){}

  /** Преобразует объект к правому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *          то возвращается ссылка на объект scatter_hierarchy<R>.
    */
  right_type& right(){ return static_cast<right_type&>(*this); }
  const right_type& right() const { return static_cast<const right_type&>(*this); }

  /** Преобразует объект к левому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *          то возвращается ссылка на объект L.
    */
  left_type& left() { return static_cast<left_type&>(*this); }
  const left_type& left() const { return static_cast<const left_type&>(*this); }

  /** Возвращает ссылку на объект типа T. Если в списке типов, по которому
    * строилась иерархия, такого типа нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return сыылка на объект типа T или производный
    */
  template <typename T>
  typename left_cast<T, type_list_type>::type& 
  get()
  {
    typedef typename type_list_cast< T, type_list_type >::type sublist;
    typedef virtual_scatter_hierarchy<sublist> T_hierarchy;
    return static_cast<T_hierarchy& >(*this);
  }

  /** Возвращает константную ссылку на объект типа T. Если в 
    * списке типов, по которому строилась иерархия, такого типа 
    * нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return константная сыылка на объект типа T или производный
    */
  template <typename T>
  const typename left_cast<T, type_list_type>::type& 
  get() const
  {
    typedef typename type_list_cast< T, type_list_type >::type sublist;
    typedef virtual_scatter_hierarchy<sublist> T_hierarchy;
    return static_cast< const T_hierarchy& >(*this);
  }
};

}}

#include <fas/pattern/specialization/scatter_hierarchy.hpp>

#endif
