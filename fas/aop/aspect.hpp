//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_ASPECT_HPP
#define FAS_AOP_ASPECT_HPP

#include <fas/aop/advice.hpp>
#include <fas/pattern/scatter_hierarchy.hpp>

namespace fas{ namespace aop {

namespace ap = ::fas::pattern;


template< typename TL = ap::empty_type>
struct aspect;


template<typename L, typename R>
struct aspect_merge;

template<typename A, typename B = void>
class aspect_class;

template<typename T, typename A >
struct advice_cast;

template<typename T, typename A>
typename advice_cast<T, typename A::aspect >::type& get_advice(A& a);

template<typename T, typename A>
const typename advice_cast<T, typename A::aspect >::type& get_advice( const A& a);

}}

#include <fas/aop/detail/aspect.hpp>

namespace fas{ namespace aop {

/** Создает аспект на базе списка адвайсов (создает распределенную 
  * иерархию и наследует ее). Но сначала у всех адвайсов из списка
  * удаляются дубликаты тегов и удаляются из списка адвайсы с пустым
  * списком тегов.
  * @param T - адвайс или список типов адвайсов
  * @typedef advice_list - список типов адвайсов
  */
template< typename T >
struct aspect
  : ap::scatter_hierarchy< typename advice_list_traits<T>::type >
{
  typedef typename advice_list_traits<T>::common_list common_list;
  typedef typename advice_list_traits<T>::type advice_list;

  template<typename F>
  F for_each(F f)
  {
    return _aspect_for_each(*this, f);
  }

  template<typename F>
  F for_each(F f) const
  {
    return _aspect_for_each(*this, f);
  }

  template<typename Tg>
  struct select { typedef typename ap::select<Tg, advice_list>::type type; };

  template<typename TT>
  struct has
  {
		enum { result = ap::type_count<TT, advice_list>::result };
  };

private:

  template< template<typename> class H, typename C, typename F >
  F _aspect_for_each(H<C>& item, F f)
  {
    f(item.left());
    return _aspect_for_each( item.right(), f);
  }

  template< template<typename> class H, typename C, typename F >
  F _aspect_for_each(const H<C>& item, F f) const
  {
    f(item.left());
    return _aspect_for_each( item.right(), f);
  }

  template< template<typename> class H, typename F >
  F _aspect_for_each(H<ap::empty_type>&, F f) { return f;}

  template< template<typename> class H, typename F >
  F _aspect_for_each(const H<ap::empty_type>&, F f) const { return f;}
};


/** Объединяет два аспекта. Создает новый аспект с объединенным 
  * списком адвайсов.
  * @param L - первый аспект для объединения
  * @param R - второй аспект для объединения
  * @param type - аспект - результат объединения
  */
template<typename L, typename R>
struct aspect_merge
{
private:

  ///  косяк на самом деле. объединяются аспекты без применения алиасов
  typedef typename L::common_list left_common_list;
  typedef typename R::common_list right_common_list;

  typedef typename ap::merge< left_common_list,
                              right_common_list >::type merge_common_list;


public:
  typedef aspect< merge_common_list > type;
};


/** Осуществляет поиск в аспекте A адвайса с тегом T. Если адвайса с тегом
  * T в аспекте нет, то ошибка компиляции
  * @param T - тег искомого адвайса
  * @param A - аспект, в которм осуществляем поиск
  * @typedef type - тип найденного адвайса
  */
template<typename T, typename A >
struct advice_cast
{
  // typedef typename ap::left_cast<T, typename A::aspect::advice_list>::type::advice_class type;
  typedef typename ap::left_cast<T, typename A::advice_list>::type advice;
  typedef typename advice::advice_class type;
};



/** Аспектный класс. Используеться в качестве базового для классов 
  * поддерживающих аспекты.
  * @param A - аспект предоставляемый порожденным классом по умолчанию
  * @param B - аспект определенный пользователем
  * @typedef aspect - аспект аспектного класса
  */
template<typename A, typename B>
class aspect_class
  : protected aspect_merge<A, B>::type
{
public:
  typedef typename aspect_merge<A, B>::type aspect;
  typedef typename aspect::advice_list advice_list;

  /** Возвращает ссылку на аспект */
  aspect& get_aspect() { return *this;}

  /** Возвращает константную ссылку на аспект */
  const aspect& get_aspect() const { return *this;}

  /** Перестраивает текущий класс объеденяя аспекты 
  * @param AA - дополнительный аспект 
  * @param BD - дополнительный аспект 
  * @typedef type новый аспектный класс с объединенными аспектами
   */
  template<typename AA = A, typename BB = B>
  struct rebind
  {
    typedef aspect_class< aspect, typename aspect_merge<AA, BB>::type > type;
  };

};

/** Возвращает ссылку на адвайс помеченного тегом T аспект-класса.
  * Если адвайса с тегом T в аспекте класса нет, то ошибка компиляции.
  * @param T - ссылка на аспектный объект
  * @param a - ссылка на аспектный объект
  * @return ссылка на найденный адвайс аспектного объект
  */
template<typename T, typename A>
inline typename advice_cast<T, typename A::aspect>::type& get_advice(A& a)
{
  return a.get_aspect().template get<T>();
}

template<typename T, typename A>
inline const typename advice_cast<T, typename A::aspect>::type& get_advice(const A& a)
{
  return a.get_aspect().template get<T>();
}


template<typename A>
struct extract_aspect_tags
{
  typedef typename A::advice_list advice_list;
  typedef typename extract_advice_tags<advice_list>::tag_list tag_list;
  typedef typename extract_advice_tags<advice_list>::gtag_list gtag_list;
  typedef typename extract_advice_tags<advice_list>::all_tag_list all_tag_list;

  /*typedef typename A::advice_list advice_list;
  typedef detail::extract_aspect_tags_helper< advice_list > helper;

  typedef typename ap::organize<typename helper::tag_list>::type tag_list;
  typedef typename ap::organize<typename helper::gtag_list>::type gtag_list;
  typedef typename ap::organize<typename helper::all_tag_list>::type all_tag_list;
  */
};

}}


#include <fas/aop/specialization/aspect.hpp>

#endif
