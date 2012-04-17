//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <fas/adv/ad_cleaner.hpp>
#include <fas/adv/ad_clock.hpp>
#include <fas/adv/json_rpc3/aspect.hpp>
#include <fas/aop/aspect.hpp>
#include <vector>
#include "ad_exception.hpp"

namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace ad = ::fas::adv;
namespace ajr = ::fas::adv::json_rpc3;
namespace aj = ::fas::json;

/**
  * Базовый аспект для json-rpc (объединяется со всеми аспектами использующие json-rpc),
  *  включает адвайсы для замеров времени обработки команды (чтоб работала статистика по командам)
  * и обработчик ошибок по умолчанию, который завершает соединение
  */

namespace common{ namespace aspect{

struct _error_{};

struct clock_start_advice
  : aa::advice<
      aa::tag<ad::_clock_start_>,
      ad::ad_clock_start<void, void>
    >
{};

struct clock_finish_advice
  : aa::advice<
      aa::tag<ad::_clock_finish_>,
      ad::ad_clock_finish<>
    >
{};

      /*
struct json_exception_advice
  : aa::advice<
     aa::tag_list_n< ajr::_invalid_json_, ajr::_unknown_method_ >::type,
     ad_exception
    >
{};
      */

/**
 {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Procedure not found."}, "id": 10}
 {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
 {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid JSON-RPC."}, "id": null}

  message Meaning
-32700  Parse error.  Invalid JSON. An error occurred on the server while parsing the JSON text.
-32600  Invalid Request.  The received JSON is not a valid JSON-RPC Request.
-32601  Method not found. The requested remote-procedure does not exist / is not available.
-32602  Invalid params. Invalid method parameters.
-32603  Internal error. Internal JSON-RPC error.
-32099..-32000  Server error. Reserved for implementation-defined server-errors.
*/
struct ad_json_error
{
  template<typename T>
  void operator()(T& t, int id, int code, const char* text)
  {
    char data[255];
    size_t s = snprintf(data, 255, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %d, \"message\": \"%s.\"}, \"id\": %d}",
                               code, text, id);
    t.get_aspect().template get< ajr::_invoke_>().error(t, id, data, s);
  }

  template<typename T>
  void operator()(T& t, int code, const char* text)
  {
    char data[255];
    size_t s = snprintf(data, 255, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %d, \"message\": \"%s.\"}, \"id\": null}",
                               code, text);
    std::cout << data << std::endl;
    t.get_aspect().template get< ajr::_invoke_>().error(t, 0, data, s);
    try
    {
//      throw;
    }
    catch(const aj::json_error& e)
    {
       std::cout << e.what() << std::endl;
    }
    catch(...)
    {
      std::cout << "catch(...)" << std::endl;
    }
  }
};

//struct _error_{};

struct json_error_advice
  : aa::advice<
     _error_,
     ad_json_error
    >
{};

struct ad_invalid_json
{
  template<typename T>
  void operator()(T& t, const char* d, size_t s)
  {
    t.get_aspect().template get<_error_>()(t, -32600, "Invalid JSON-RPC.");
    t.release(false);
  }
};

struct ad_unknown_method
{
  template<typename T>
  void operator()(T& t, const char* d, size_t s)
  {
    t.get_aspect().template get<_error_>()(t, -32601, "Procedure not found.");
    t.release(false);
  }
};

struct invalid_json_advice
  : aa::advice<
     ajr::_invalid_json_,
     ad_invalid_json
    >
{};

struct unknown_method_advice
  : aa::advice<
     ajr::_unknown_method_,
     ad_unknown_method
    >
{};



     /*
/// Не используется по умолчанию, может включаться в аспект вместо json_exception_advice
struct json_exception_advice_empty
  : aa::advice<
     aa::tag_list_n< ajr::_invalid_json_, ajr::_unknown_method_ >::type,
     ad::ad_stub<>
    >
{};
*/

typedef ap::type_list_n<
  clock_start_advice,
  clock_finish_advice,
  json_error_advice,
  invalid_json_advice,
  unknown_method_advice
      /*,
  json_exception_advice*/
>::type json_advice_list;

typedef aa::aspect<json_advice_list> json_aspect_base;

typedef aa::aspect_merge< ajr::aspect, json_aspect_base>::type json_aspect;

}}

