//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_DESC_HOLDER_H
#define FAS_FILTERS_DESC_HOLDER_H

namespace fas { namespace adv { namespace io{

/** Класс-holder для хранения дескриптора и его статуса.
  * @param D - тип дескриптора
  */
template<typename D>
class desc_holder
{
public:
  typedef D desc_type;
  desc_holder():_status(false){}
  desc_holder(const desc_type& d ): _d(d), _status(true){}
  void set_d(const desc_type& d) { _d = d; _status = true; }
  const desc_type& get_d() const { return _d ; }
  bool get_status() const { return _status;} 
  void set_status(bool value) 
  {
    _status = value;
  }
private:
  D _d;
  bool _status;
};

}}}

#endif // FAS_FILTERS_DESC_HOLDER_H
