#ifndef FAS_ADV_JSON_RPC3_ASPECT_HPP
#define FAS_ADV_JSON_RPC3_ASPECT_HPP

#include <fas/pattern/type_list.hpp>
#include <fas/aop/aspect.hpp>
#include <fas/adv/ad_stub.hpp>
#include <fas/adv/ad_proxy.hpp>
#include <fas/adv/ad_cleaner.hpp>

#include <fas/adv/ad_clock.hpp>

#include <fas/adv/json_rpc3/tags.hpp>
#include <fas/adv/json_rpc3/json_rpc3_types.hpp>
#include <fas/adv/json_rpc3/ad_invoke.hpp>
#include <fas/adv/json_rpc3/outgoing_data.hpp>

namespace fas{ namespace adv{ namespace json_rpc3{

namespace ap = ::fas::pattern;
namespace aa = ::fas::aop;
namespace ad = ::fas::adv;


struct stubs_advice:
  aa::advice<
    aa::tag_list_n<
      aa::tag<ad::_clock_finish_>,
      aa::tag<ad::_clock_start_>,
      aa::tag<_unknown_method_>,
      aa::tag<_invalid_json_>,
      aa::tag<_lost_result_>,
      aa::tag<_invalid_id_>,
      aa::tag<_output_>,
      aa::tag<_begin_invoke_>,
      aa::tag<_end_invoke_>,
      aa::tag<_binder_>
    >::type,
    ad::ad_stub<>
  >
{};

struct invoke_advice:
  aa::advice<
    aa::tag_list_n<
      aa::gtag<ad::_cleaner_>,
      aa::tag<_input_>,
      aa::tag<_invoke_>
    >::type,
   ad_invoke
>
{};

struct outgoing_data_advice:
  aa::advice< 
    aa::tag_list_n<
      aa::gtag<ad::_cleaner_>,
      aa::tag<_outgoing_data_>
    >::type,
    outgoing_data
  >
{};

struct span_reg_advice:
  aa::advice<
    aa::tag<ad::_span_reg_>,
    ad::ad_span_reg
  >
{};

struct advice_list
  : ap::type_list_n<
      invoke_advice,
      outgoing_data_advice,
      stubs_advice,
      span_reg_advice,
      aa::advice< aa::tag<_writer_>, ad::ad_proxy<_output_> >
    >::type
{};

typedef aa::aspect<advice_list> aspect;

}}}

#endif
