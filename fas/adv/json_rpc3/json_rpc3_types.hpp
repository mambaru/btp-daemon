//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_ADV_JSON_RPC_JSON_RPC3_TYPES_HPP
#define FAS_ADV_JSON_RPC_JSON_RPC3_TYPES_HPP

#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include <fas/pattern/type_list.hpp>
#include <fas/misc/json.hpp>

namespace fas{ namespace adv{ namespace json_rpc3{

namespace amj = ::fas::json;
namespace ap = ::fas::pattern;
/*
--> {"jsonrpc": "2.0", "method": "foobar", "id": 10}
<-- {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Procedure not found."}, "id": 10}

Procedure Call with invalid JSON:

--> {"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz"]
<-- {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}

Procedure Call with invalid JSON-RPC:

--> [1,2,3]
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid JSON-RPC."}, "id": null}

--> {"jsonrpc": "2.0", "method": 1, "params": "bar"}
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid JSON-RPC."}, "id": null}
*/
class procedure_not_found
  : public std::runtime_error
{
public:
  procedure_not_found(): std::runtime_error("Procedure not found"){}
};

class invalid_json_rpc
  : public std::runtime_error
{
public:
  invalid_json_rpc(): std::runtime_error("Invalid JSON-RPC"){}
};

class parse_error
  : public std::runtime_error
{
public:
  parse_error(): std::runtime_error("Parse error"){}
};

class invalid_id
  : public std::runtime_error
{
public:
  invalid_id(): std::runtime_error("Invalid id"){}
};


template<typename P>
class request_object
{
public:
  typedef P params_type;

  char jsonrpc[8];
  char method[64];
  params_type params;
  int id;

  request_object():id(-1)
  {
    std::strcpy(jsonrpc, "2.0");
    std::strncpy(method, "", 64);
  }

  request_object(const char* name)
    : id(-1)
  {
    std::strcpy(jsonrpc, "2.0");
    std::strncpy(method, name, 64);
  }
};


template<typename P>
class response_object
{
public:
  typedef P result_type;

  char jsonrpc[8];
  result_type result;
  int id;

  response_object()
    : id(-1)
  {
    std::strcpy(jsonrpc, "2.0");
  }
};

template<typename P>
class notify_object
{
public:
  typedef P params_type;

  char jsonrpc[8];
  char method[64];
  params_type params;

  notify_object()
  {
    std::strcpy(jsonrpc, "2.0");
    std::strncpy(method, "", 64);
  }

  notify_object(const char* name)
  {
    std::strcpy(jsonrpc, "2.0");
    std::strncpy(method, name, 64);
  }
};


struct n_jsonrpc { const char* operator()() const { return "jsonrpc";}  };
struct n_method  { const char* operator()() const { return "method"; }  };
struct n_params  { const char* operator()() const { return "params"; }  };
struct n_result  { const char* operator()() const { return "result"; }  };
struct n_id      { const char* operator()() const { return "id";     }  };

template<typename J>
struct request_maker
{
  typedef typename J::target target;

  typedef typename amj::object<
    request_object<target>,
    typename ap::type_list_n<
      amj::member<n_jsonrpc, request_object<target>, char[8], &request_object<target>::jsonrpc >,
      amj::member<n_method, request_object<target>, char[64], &request_object<target>::method >,
      amj::member<n_id, request_object<target>, int, &request_object<target>::id >,
      amj::member<
        n_params, request_object<target>,
        typename request_object<target>::params_type,
        &request_object<target>::params, J
      >
    >::type
  > type;
};

template<typename J>
struct notify_maker
{
  typedef typename J::target target;

  typedef typename amj::object<
    notify_object<target>,
    typename ap::type_list_n<
      amj::member<n_jsonrpc, notify_object<target>, char[8], &notify_object<target>::jsonrpc >,
      amj::member<n_method, notify_object<target>, char[64], &notify_object<target>::method >,
      amj::member<
        n_params, notify_object<target>,
        typename notify_object<target>::params_type,
        &notify_object<target>::params, J
      >
    >::type
  > type;
};

template<typename J>
struct response_maker
{
  typedef typename J::target target;

  typedef typename amj::object<
    response_object<target>,
    typename ap::type_list_n<
      amj::member<n_jsonrpc, response_object<target>, char[8], &response_object<target>::jsonrpc >,
      amj::member<n_id, response_object<target>, int, &response_object<target>::id >,
      amj::member<
        n_result, response_object<target>,
        typename response_object<target>::result_type,
        &response_object<target>::result, J
      >
    >::type
  > type;
};


}}}

#endif
