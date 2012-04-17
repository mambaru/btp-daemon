//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_SCATTER_HIERARCHY_SPECIALIZATION_HPP
#define FAS_PATTERNS_SCATTER_HIERARCHY_SPECIALIZATION_HPP

#include <string>

#include <fas/pattern/type_list.hpp>

namespace fas{ namespace pattern {

template<typename L>
struct scatter_hierarchy1//< type_list<L, empty_type> >
  : public L
{
  /** Список типов, на основе которого построена иерархия */
  typedef type_list<L, empty_type > type_list_type;
  /** Первый (левый) тип из списка типов */
  typedef L left_type;
  /** Тип распределенной иерархии построенной на основе
    * списка без первого типа */
  typedef empty_type right_type;

  /** Преобразует объект к правому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *          то возвращается ссылка на объект scatter_hierarchy<R>.
    */
  const empty_type right() const { return empty_type(); }

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
    typedef scatter_hierarchy< sublist> T_hierarchy;
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
    typedef scatter_hierarchy<sublist> T_hierarchy;
    return static_cast< const T_hierarchy& >(*this);
  }
  
};

/**
  Проблема: если не делать специализацию то нужно наледование от empty_type, что не позволит
  объеденить в одну иерархию несколько scatter_hierarchy, в тоже время get<> от несуществ. типа должен давать empty_type
*/
template<typename L>
struct scatter_hierarchy2///< type_list<L, empty_type> >
  : public L
{
  typedef empty_type right_type;
  empty_type _right;

  /** Список типов, на основе которого построена иерархия */
  typedef type_list<L> type_list_type;

  /** Первый (левый) тип из списка типов */
  typedef L left_type;

  /** Преобразует объект к правому базовому типу и возвращает ссылку на него.
    * @return если иерархия построена на основе списка type_list<L, R>, 
    *          то возвращается ссылка на объект scatter_hierarchy<R>.
    */
  empty_type& right(){ return _right; }
  const right_type& right() const { return _right; }

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


template<>
struct scatter_hierarchy< empty_type >: empty_type  ///!!! не лучшая идея (см. выше)
{
  typedef empty_type type_list_type;
  typedef empty_type left_type;
  typedef empty_type right_type;
};

template<>
struct virtual_scatter_hierarchy<empty_type>
{
  empty_type right() const { return empty_type(); }
  empty_type left() const { return empty_type(); }
};


}}

#endif
