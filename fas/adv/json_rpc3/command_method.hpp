#ifndef FAS_ADV_JSON_RPC3_COMMAND_METHOD_HPP
#define FAS_ADV_JSON_RPC3_COMMAND_METHOD_HPP

namespace fas{ namespace adv{ namespace json_rpc3{

template<typename N, typename J1, typename J2 = J1, bool NoNotyfy = false>
class command_method;

template<typename J1, typename J2, bool NoNotyfy>
class command_method_base;

template<typename J1, typename J2>
class command_method_base<J1, J2, false>
{
public:
  typedef J1 invoke_notify;
  typedef J1 invoke_request;
  typedef J2 invoke_response;

  typedef J2 call_notify;
  typedef J1 call_request;
  typedef J2 call_response;

  typedef typename J1::target request_type;
  typedef typename J2::target response_type;
};

template<typename N, typename J1, typename J2, bool NoNotyfy>
class command_method
  : command_method_base<J1, J2, NoNotyfy>
{
  N _n;
  typedef command_method_base<J1, J2, NoNotyfy> super;
public:
  const char* name() const { return _n(); }

  template<typename T>
  void clear(T&) { }

  typedef typename super::invoke_notify invoke_notify;
  typedef typename super::invoke_request invoke_request;
  typedef typename super::invoke_response invoke_response;

  typedef typename super::call_notify call_notify;
  typedef typename super::call_request call_request;
  typedef typename super::call_response call_response;

  typedef typename super::request_type request_type;
  typedef typename super::response_type response_type;

  template<typename T>
  bool request(T& t, const request_type& cmd, int id, response_type& resp )
  {
    return t._process_request(t, cmd, id, resp);
  }

  template<typename T>
  void response(T& t, request_type& cmd, int id )
  {
    t._process_response(t, cmd, id);
  }

  template<typename T>
  void notify(T& t,request_type& cmd )
  {
    t._process_notify(t, cmd);
  }
};


template<typename N, typename J>
class notify_receiver_method
  /*: public J::target::iprocessor*/
{
  N _n;
public:
  typedef J invoke_notify;
  typedef typename J::target request_type;

  const char* name() const { return _n(); }

  template<typename T>
  void clear(T&) { }

  template<typename T>
  void notify(T& t,request_type& cmd )
  {
    t._process_notify(t, cmd);
  }

  // virtual void process(request_type& cmd){}
};

template<typename N, typename J>
class notify_sender_method
{
  N _n;
public:
  typedef J call_notify;

  const char* name() const 
  {

    return _n();
  }

  template<typename T>
  void clear(T&) { }
};


}}}

#endif
