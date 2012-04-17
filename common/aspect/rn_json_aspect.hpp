//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <fas/filter/binary_filter.hpp>
#include <fas/adv/ad_cleaner.hpp>
#include <fas/adv/io/ad_binary_splitter.hpp>
#include <fas/aop/aspect.hpp>
#include <fas/adv/json_rpc3/aspect.hpp>
#include "json_aspect.hpp"

namespace ap = ::fas::pattern;
namespace ad = ::fas::adv;
namespace adio = ::fas::adv::io;
namespace aa = ::fas::aop;
namespace af = ::fas::filter;
namespace ajr = ::fas::adv::json_rpc3;

namespace common { namespace aspect{

struct _rn_writer_{};

class ad_rn_writer
{
  typedef std::vector<char> data_type;
  data_type data;
public:
  template<typename T>
  void operator () (T&t, const char* d, size_t s)
  {
//    std::cout << "RN WRITE [[[" << std::endl << std::string(d, d + s) << std::endl << "]]]" << std::endl;
    /// Возможно не самое красивое решения, но при s < 1024 (очень примерно)
    /// Это работает быстрее чем запись двумя порциями ( d, а затем \r\n)
    data.reserve(s + 2);
    data.assign(d, d + s);
    data.push_back('\r');
    data.push_back('\n');
    t.get_aspect().template get<af::_writer_>()(t, &(data[0]), data.size());

    if ( data.capacity() > 10240 )
      data_type().swap(data);
  }
};

struct _rn_trace_ {};

struct rn_trace
{
  template<typename T>
  void operator () (T&t, const char* d, size_t s)
  {
    std::cout << "RN READ [[[" << std::endl << std::string(d, d + s) << std::endl << "]]]" << std::endl;
    t.get_aspect().template get<ajr::_input_>()(t, d, s);
  }
};



struct rn_reader_advice:
         aa::advice<
           aa::tag_list_n< af::_on_read_, aa::gtag< ad::_cleaner_> >::type,
           adio::ad_binary_splitter< adio::sep<'\r', '\n'>, ajr::_input_/*_rn_trace_*/ >
         >
{};

struct rn_writer_advice:
  aa::advice<
    aa::tag< _rn_writer_ >,
    ad_rn_writer
  >
{};

typedef ap::type_list_n<
    rn_reader_advice,
    rn_writer_advice,
    aa::advice< _rn_trace_, rn_trace>,
    aa::alias< _rn_writer_, ajr::_output_ >
>::type advice_list;

typedef aa::aspect<advice_list> rn_aspect;
struct rn_json_aspect: aa::aspect_merge< json_aspect, rn_aspect>::type {};

}}

