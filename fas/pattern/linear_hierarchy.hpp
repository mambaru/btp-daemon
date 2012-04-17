//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_LINEAR_HIERARCHY_HPP
#define FAS_PATTERNS_LINEAR_HIERARCHY_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/pattern/nth_cast.hpp>

namespace fas{ namespace pattern
{

template<class TL>
class linear_hierarchy;

/** Линейная иерархия - это иерархия классов linear_hierarchy, число которых 
  * равно размеру списку типов, по которому строится эта иерархия. Каждый 
  * такой класс содержит атрибут, тип которого совподает с одним из типов 
  * списка типов и наследуеться от класса, содержащий атрибут следующего по 
  * порядку типа из списка. Класс содержащий атрибут последнего типа, не имеет 
  * базового класса. Доступ к объектам осуществляется либо по индексу (номер 
  * по порядку в списке типов) либо по типу.
  * @param - T организованный список типов
  */
template<class L, class R>
class linear_hierarchy< type_list<L,R> >
  : public linear_hierarchy< R >
{
public:
  /** Список типов линейной иерархии */
  typedef type_list<L,R> type_list_type;
  /** Тип этого класса */
  typedef linear_hierarchy< type_list<L,R> > self;
  /** Тип базового класса */
  typedef linear_hierarchy< R > right_type;
  /** Тип базового класса */
   typedef linear_hierarchy< R > super;
  /** тип хранимого аттрибута (первый тип из списка ) */
  typedef L value_type;

  /** Возвращает ссылку на базовый класс */
  right_type& right() { return static_cast<right_type&>(*this);}
  /** Возвращает костантную ссылку на базовый класс */
  const right_type& right() const { return static_cast<const right_type&>(*this);}

  /** Возвращает ссылку на хранимый аттрибут */
  value_type& left() { return value;}
  /** Возвращает костантную ссылку на хранимый аттрибут */
  const value_type& left() const { return value;}

  /** Возвращает ссылку на объект типа T или производного типа. Если в
    * списке типов, по которому строилась иерархия, такого типа 
    * нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return ссылка на объект типа T или производный
    */
  template <typename T>
  typename left_cast<T, type_list_type>::type&
  get()
  {
    typedef typename type_list_cast<T, type_list_type>::type sublist;
    typedef linear_hierarchy< sublist > T_hierarchy;
    return static_cast< T_hierarchy& >(*this).left();
  }

  /** Возвращает константную ссылку на объект типа T или производного типа.
    * Если в списке типов, по которому строилась иерархия, такого типа 
    * нет, то ошибка компиляции.
    * Примечание: Если тип T является базовым для типов Х1..XN из 
    * списка типов, по которому строилась иерархия, то срабатывает
    * первое вхождения типов T, Х1..XN.
    * @return константная ссылка на объект типа T или производный
    */
  template <typename T>
  const typename left_cast<T, type_list_type>::type& get() const 
  {
    typedef typename type_list_cast<T, type_list_type>::type sublist;
    typedef linear_hierarchy< sublist > T_hierarchy;
    return static_cast< const T_hierarchy& >(*this).left();
  }

  /** Возвращает ссылку на объект i-ого типа из списка типов 
    * по которому строилась иерархия.Если i больше динны списка
    * типов, то ошибка компиляции.
    * @param int - номер типа из списка
    * @return ссылка на объект типа i-ого типа из списка типов
    */
  template< int i>
  typename nth_cast<i, self>::type::value_type& get()
  {
    typedef typename nth_cast<i, self>::type N_type;
    return static_cast< N_type& >(*this).left();
  };

  /** Возвращает константную ссылку на объект i-ого типа из списка типов 
    * по которому строилась иерархия.Если i больше динны списка
    * типов, то ошибка компиляции.
    * @param int - номер типа из списка
    * @return константная ссылка на объект типа i-ого типа из списка типов
    */
  template< int i>
  const typename nth_cast<i, self>::type::value_type& get() const
  {
    typedef typename nth_cast<i, self>::type N_type;
    return static_cast< const N_type& >(*this).left();
  };

private:
  value_type value;
};

}}

#include <fas/pattern/specialization/linear_hierarchy.hpp>

#endif
