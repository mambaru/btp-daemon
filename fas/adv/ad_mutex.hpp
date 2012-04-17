//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_MUTEX_HPP
#define FAS_FILTERS_AD_MUTEX_HPP

#include <fas/aop/aspect.hpp>
#include <fas/system/thread.hpp>

namespace fas { namespace adv { 

/** Адвайс-класс для храниения объекта синхронизации 
  * @param М - тип объекта синхронизации
  * @typedef mutex_type - тип объекта синхронизации
  */
template<typename M = fas::system::thread::empty_mutex>
class ad_mutex
{
  mutable M _mutex;
public:
  
  typedef M mutex_type;
  typedef ad_mutex<M> self;

  /** Коструктор. Создает объект синхронизации.*/
  ad_mutex(){}

  /** Коструктор копирования. Создает новый объект синхронизации.*/
  ad_mutex(const self& ) {}

  /** Возвращает ссылку на объект синхронизации */
  mutex_type& get_mutex() const {  return _mutex; }

  /** Возвращает ссылку на объект синхронизации */
  mutex_type& get_mutex() {  return _mutex; }
  
  /** Деструктор. Удаляет объект синхронизации.*/
  // ~ad_mutex() { delete _mutex;}
};


}}

#endif
