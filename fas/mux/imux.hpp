//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MUX_IMUX_HPP
#define FAS_MUX_IMUX_HPP

#include <fas/mux/types.hpp>
#include <fas/mux/imux_observer.hpp>

namespace fas { namespace mux {


//template<typename D>
//class imux_observer;

/**
 * @interface Инерфейс мултиплексора ввода/вывода
 * @param D - тип дескриптора
 */
template<typename D = descriptor_t>
class imux
{
public:
  typedef D desc_type;
  typedef class imux_observer<D> imux_observer;

  virtual ~imux(){}

  /** Установить наблюдателя для всех событий дескриптора  */
  virtual void set_handlers(desc_type d, imux_observer* imo) =0;

  /** Сбросить всех наблюдателей для дескриптора */
  virtual void reset_handlers(desc_type d) =0;

  /** Установить наблюдатель для дескриптора на готовность чтения  */
  virtual imux_observer* set_rhandler(desc_type d, imux_observer* imo) =0;

  /** Установить наблюдатель для дескриптора на готовность записи  */
  virtual imux_observer* set_whandler(desc_type d, imux_observer* imo) =0;

  /** Установить наблюдатель для дескриптора на готовность получения внеполосных данных  */
  virtual imux_observer* set_uhandler(desc_type d, imux_observer* imo) =0;

  /** Сбросить наблюдателя для дескриптора на готовность чтения  */
  virtual imux_observer* reset_rhandler(desc_type d) =0;

  /** Сбросить наблюдателя для дескриптора на готовность записи  */
  virtual imux_observer* reset_whandler(desc_type d) =0;

  /** Сбросить наблюдатель для дескриптора на готовность получения внеполосных данных  */
  virtual imux_observer* reset_uhandler(desc_type d) =0;

  /** Ожидать готовности для наблюдаемых дескрипторов не более timeout милисекунд.
    * @param timeout - время ожидания в милмсекундах (-1 вечно, 0 - неждать)
    * @return true - произошло одно или несколько событий
    *         false - вышел по timeout
    */
  virtual bool select(long timeout) = 0;
};

} }

#endif //FAS_MUX_IMUX_H
