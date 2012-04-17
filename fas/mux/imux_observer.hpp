//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MUX_IMUX_OBSERVER_HPP
#define FAS_MUX_IMUX_OBSERVER_HPP

namespace fas{ namespace mux{

/** @interface Интерфейс обработчика событий мультиплексора.
  * @param D - тип дескриптора
  */
template<typename D>
class imux_observer
{
public:
    typedef D desc_type;
    virtual ~imux_observer(){}
    /** Вызывается мултиплексором (субъектом), когда дескриптор готов для чтения */
    virtual void ready_read(desc_type d) = 0;
    /** Вызывается мултиплексором (субъектом), когда дескриптор готов для записи */
    virtual void ready_write(desc_type d) = 0;
    /** Вызывается мултиплексором (субъектом), когда пришли внеполосные данные */
    virtual void ready_urgent(desc_type d) = 0;
    /** Вызывается мултиплексором (субъектом), когда произошла ошибка */
    virtual void ready_error(desc_type d) = 0;
};

}}

#endif //FAS_MUX_IMUX_OBSERVER_HPP
