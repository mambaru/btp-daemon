//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_BEST_MUX_HPP
#define FAS_BEST_MUX_HPP

#include <fas/system/system.hpp>

#ifdef WIN32
#include <fas/mux/selector.hpp>
#elif HAVE_SYS_EPOLL_H
#include <fas/mux/epoller.hpp>
#else
#include <fas/mux/selector.hpp>
#endif

namespace fas { namespace mux {

/** Лучший мультиплексор для даной ОС */
#ifdef WIN32
class best_mux: public selector{};
#elif HAVE_SYS_EPOLL_H
class best_mux: public epoller{};
#else
class best_mux: public selector{};
#endif

}}

#endif
