//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_AD_STUB_HPP
#define FAS_AOP_AD_STUB_HPP

namespace fas { namespace adv {

/** Авайс-класс - заглушка. 
  * @param R - тип возвращаемого значения
  */
template<typename R = void>
class ad_stub
{
public:
  template<typename T>
  R operator()(T&) { return R(); }

  template<typename T, typename T1>
  R operator()(T&, T1) { return R(); }

  template<typename T, typename T1, typename T2>
  R operator()(T&, T1, T2) { return R(); }

  template<typename T, typename T1, typename T2, typename T3>
  R operator()(T&, T1, T2, T3) { return R(); }

  template<typename T, typename T1, typename T2, typename T3, typename T4>
  R operator()(T&, T1, T2, T3, T4) { return R(); }
};


}}

#endif
