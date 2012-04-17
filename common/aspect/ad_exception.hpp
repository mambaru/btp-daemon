#pragma once
//#include "logger.hpp"

namespace common { namespace aspect{


/** Универсальный обработчик исключительных ситуаций сервисов.
  * Может включаться в аспект под различными тегами (например json_error, unknown_method или http_error)
  * По умолчание закрывает соединение и отправляет уведомление об ошибке
  */
struct ad_exception
{
  template<typename T>
  void operator()(T& t)
  {
    //::logger<_common_log_, mlog::error>() << "mamba::service::ad_exception - unknown exception ";
    t.release();
  }

  template<typename T>
  void operator()(T& t, const char* d, size_t s)
  {
    /*
    if ( s == 0 ) 
      ::logger<_common_log_, mlog::error>() << "mamba::service::ad_exception - empty exception ";
    else
      ::logger<_common_log_, mlog::error>() << "mamba::service::ad_exception(string): " << std::string(d, d + s) ;
    */
    t.release();
  }

  template<typename T>
  void operator()(T& t, const std::exception& e)
  {
    //::logger<_common_log_, mlog::error>() << "mamba::service::ad_exception(std::exception): " << e.what() ;
    t.release();
  }
};

}}

