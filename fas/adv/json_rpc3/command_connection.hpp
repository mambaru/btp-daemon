#ifndef FAS_ADV_JSON_RPC3_CONNECTION_HPP
#define FAS_ADV_JSON_RPC3_CONNECTION_HPP

#include <fas/inet/connection.hpp>
#include "method.hpp"

/** класс соединения для работы с командами
  * Для использования необходимо переопределить:
  * 
     class my_command_list { ... }

     template< typename A, typename F >
     class my_connection_base:
        public command_connection<A, F, my_command_list>
     {
     public:
       
       // virtual void process(command1& cmd) { super::template _process<_command1_>(*this, cmd); };
       // virtual void process(command2& cmd) {};
       
     };

  */

namespace fas{ namespace adv{ namespace json_rpc3{

namespace detail
{
/// notify
  template<bool>
  struct notify_processor;

  template<>
  struct notify_processor<true>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) 
    {
      if ( c.is_notify() )
        m.notify(t, c);
    }
  };

  template<>
  struct notify_processor<false>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) 
    {
    }
  };

/// request
  template<bool>
  struct request_processor;

  template<>
  struct request_processor<true>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) 
    {
      if ( c.is_request() ) m.request(t, c); 
    }
  };

  template<>
  struct request_processor<false>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) { }
  };

/// response

  template<bool>
  struct response_processor;

  template<>
  struct response_processor<true>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) 
    {
      if ( c.is_response() )
        m.response(t, c, c.id);
    }
  };

  template<>
  struct response_processor<false>
  {
    template<typename M, typename T, typename C >
    void operator()( M& m, T& t, C& c) 
    {

    }
  };

/// helper
  template<bool>
  struct processor_helper;

  template<>
  struct processor_helper<true>
  {
    template<typename M, typename T, typename C>
    void process ( T& t, C& c )
    {
      _process( t.get_aspect().template get<M>(), t, c);
    }

  private:
    template<typename M, typename T, typename C>
    void _process( M& m, T& t, C& c )
    {
      typedef typename M::advice_class method;
      _process2( static_cast<method&>(m), t, c);
    }

    template<typename M, typename T, typename C>
    void _process2( M& m, T& t, C& c )
    {

      typedef typename M::method_class method_class;

      detail::notify_processor< has_call_notify<method_class>::result>()(m, t, c);
      detail::request_processor< has_call_request<method_class>::result>()(m, t, c);
      detail::response_processor< has_invoke_response<method_class>::result>()(m, t, c);
    }
  };

  template<>
  struct processor_helper<false>
  {
    template<typename M, typename T, typename C>
    void process ( T& t, C& c )
    {
      throw std::logic_error( 
         std::string("connection<> -> pprocessor_helper<false>::process( T& t, C& c ): обработчик команды в аспекте не найден ") 
         + c.name() 
      );
    }
  };
}

class command_caller
{
public:
  template<typename M, typename T, typename C>
  void call(T& t, C& c)
  {
    enum { has_method = ap::type_count<M, typename T::aspect::advice_list>::result };
    typedef detail::processor_helper< has_method !=0 > processor_helper;
    processor_helper().template process<M>(t, c);
  }
};


template<typename A, typename F, typename I >
class command_connection
 : public fas::inet::mux_connection_base<A, F>
 , public I
{
  typedef fas::inet::mux_connection_base<A, F> super;

  command_caller _caller;

public:

  typedef typename super::aspect aspect;
  typedef typename aspect::advice_list advice_list;
  typedef typename super::mutex_type mutex_type;

  aspect& get_aspect() { return super::get_aspect();}
  const aspect& get_aspect() const { return super::get_aspect();}

protected:

  /// Обработка исходящих комманд
  template<typename Tg, typename T, typename C>
  void _call(T& t, C& cmd)
  {
    _caller.call<Tg>(t, cmd);
  }

private:
  template<typename I1, typename I2>
  static I1 _pointer_cast(I2 i) { return i;}

  template<typename I1>
  static I1 _pointer_cast(ap::empty_type* i) { return 0;}
public:
  /// Обработка запроса клента
  template<typename T, typename C>
  bool _process_request(T& t, const C& cmd, int id, C& resp)
  {
    
    // typename C::iprocessor* i = &(static_cast<I*>(this)->get_aspect().template get<typename C::iprocessor>());
    // typename C::iprocessor* i = &(I::template get<typename C::iprocessor>());
    typename C::iprocessor* i = _pointer_cast<typename C::iprocessor*> ( &(I::template get<typename C::iprocessor>()) );
    resp = cmd;
    resp.id = id;
    resp.callback = i;
    resp.as_request();
    resp.as_incoming();
    
    t.get_aspect().template get<_binder_>()(t, resp);
    t._process_incoming(t, cmd);
    resp.execute();
    return false; /// ахтунг! ответ только через bind
    
  }

  /// Обработка ответа клиента на запрос сервера
  template<typename T, typename C>
  void _process_response(T& t, C& cmd, int id)
  {
    // typename C::iprocessor* i = &(I::template get<typename C::iprocessor>()); 
    typename C::iprocessor* i = _pointer_cast<typename C::iprocessor*> ( &(I::template get<typename C::iprocessor>()) );
    cmd.id = id;
    cmd.callback = i;
    cmd.as_response();
    cmd.as_incoming();
    t.get_aspect().template get<_binder_>()(t, cmd);
    t._process_incoming(t, cmd);
    cmd.execute();
  }

  /// Обработка уведомления от клиента
  template<typename T, typename C>
  void _process_notify(T& t, C& cmd)
  {
    // typename C::iprocessor* i = &(I::template get<typename C::iprocessor>());
    typename C::iprocessor* i = _pointer_cast<typename C::iprocessor*> ( &(I::template get<typename C::iprocessor>()) );
    cmd.id = -1;
    cmd.callback = i;
    cmd.as_notify();
    cmd.as_incoming();
    
    t.get_aspect().template get<_binder_>()(t, cmd);
    t._process_incoming(t, cmd);
    cmd.execute();
  }

  template<typename T, typename C>
  void _process_incoming(T&, C&){ }


};



}}}

#endif
