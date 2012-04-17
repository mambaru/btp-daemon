//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_CONNECTION_HPP
#define FAS_INET_CONNECTION_HPP

#include <fas/adv/ad_stub.hpp>
#include <fas/adv/ad_cleaner.hpp>
#include <fas/filter/binary_filter.hpp>
#include <fas/mux/mux_filter.hpp>

#include <fas/system/inet.hpp>
#include <fas/inet/constants.hpp>
#include <fas/inet/types.hpp>
#include <fas/inet/iconnection.hpp>
#include <fas/pattern/ref.hpp>

#include <fas/aop/aspect.hpp>

namespace fas{ namespace inet{

namespace aa = ::fas::aop;
namespace ad = ::fas::adv;
namespace af = ::fas::filter;
namespace am = ::fas::mux;
namespace ap = ::fas::pattern;
namespace asi = ::fas::system::inet;

struct _on_connection_assign_{};
struct _on_connection_close_{};

struct connection_stub_tags: aa::tag_list_n<_on_connection_assign_, _on_connection_close_>::type { };

struct connection_advice_list
  : aa::tag_list_n<
      aa::advice< connection_stub_tags, ad::ad_stub<> >/*,
      aa::advice< aa::tag<_on_connection_assign_>, ad::ad_cleaner<> >*/
    >::type
{};

struct connection_aspect: aa::aspect< connection_advice_list > {};

template<typename F, typename A>
struct make_connection_super_class
{
  typedef typename F::template rebind<
       typename aa::aspect_merge<connection_aspect, A>::type
    >::type type;
};

/** Базовый класс-фильтр для работы с инет-соединениями.
  * Не создает инет-соединение, но может его закрывать.
  * @param A - пользовательский аспект
  * @param F - базовый аспект-класс-фильтр бинарного ввода/вывода
  *             ( по умолчанию binary_filter<> )
  */
template< typename A = aa::aspect<>,
          typename F = af::binary_filter<> >
class connection
  : public make_connection_super_class<F, A>::type
  , public iconnection
{
public:

  typedef connection<A, F> self;
  typedef typename make_connection_super_class<F, A>::type super;
  typedef typename super::aspect aspect;
  typedef typename super::mutex_type mutex_type;
  typedef address_t address_type;

  template<typename AA = A, typename FF = F >
  struct rebind
  {
    typedef connection< AA, FF> type;
  };

  typedef typename super::read_desc_type read_desc_type;
  typedef typename super::write_desc_type write_desc_type;
  typedef typename super::desc_type desc_type;

  connection(): _closed(true) { }

  ~connection() 
  {
     if (!_closed)
     {
       ::fas::system::inet::close( super::_get_rd() );
       if (super::_get_rd() != super::_get_wd())
          ::fas::system::inet::close( super::_get_wd() );
     }
  }

  /** Получить инет-адрес локального хоста*/
  address_t get_local_address() const
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return self::_get_local_address();
  }

  /** Получить инет-адрес удаленного хоста */
  address_t get_remote_address() const
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return self::_get_remote_address();
  }

  bool get_local_ip(std::string& ip) const
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return _get_local_ip(ip);
  }

  bool get_remote_ip(std::string& ip) const
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return _get_remote_ip(ip);
  }

  /** Инициализация соединения.
    * @param rd - дескриптор для чтения (может совпадать с wd )
    * @param wd - дескриптор для записи (может совпадать с rd )
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
   */
  void assign( read_desc_type rd,
               write_desc_type wd,
               const address_t& local_address,
               const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_assign(*this, rd, wd, ap::cref(local_address), ap::cref(remote_address) );
  }

  /** Инициализация соединения.
    * @param d - дескриптор для чтения/записи 
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
   */
  void assign( desc_type d,
               const address_t& local_address,
               const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_assign(*this, d, d, ap::cref(local_address), ap::cref(remote_address) );
  }

  /** Закрыть инет-соединение. Осуществляеться через 
    * соответствующий системный вызов. Все неполученные 
    * (непереданные) данные будут утеряны. */
  void close()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_close(*this);
  }

  void _set_remote_address( const address_t& addr )
  {
    _remote_address = addr;
  }

protected:

  /** Получить инет-адрес локального хоста*/
  address_t _get_local_address() const 
  {
    return _local_address; 
  }

  /** Получить инет-адрес удаленного хоста */
  address_t _get_remote_address() const 
  {
    return _remote_address; 
  }

  bool _get_local_ip(std::string& ip) const
  {
    return asi::address2string(_local_address, ip);
  }

  bool _get_remote_ip(std::string& ip) const
  {
    return asi::address2string(_local_address, ip);
  }

  template<typename T>
  void _assign(T& t,  read_desc_type rd,
                      write_desc_type wd,
                      const address_t& local_address,
                      const address_t& remote_address)
  {

    _local_address = local_address;
    _remote_address = remote_address;
    _closed = false;
    // Т.к. во время _assign м.б. вызван close
    super::_assign(t, rd, wd);
    typedef typename aa::advice_cast<
       _on_connection_assign_,
       aspect >::advice on_connection_assign;
    static_cast<on_connection_assign&>(t)
    (t, rd, wd, ap::cref(local_address), ap::cref(remote_address) );
    _closed = !super::_get_status();
    if ( _closed )
    {
      typedef typename aa::advice_cast<
         _on_connection_close_,
         aspect
      >::advice on_connection_close;

      static_cast<on_connection_close&>(t)(t);
    }
  }

  template<typename T>
  void _close(T& t)
  {

    if (!_closed)
    {
      ::fas::system::inet::close( super::_get_rd() );
      if (super::_get_rd() != super::_get_wd())
        ::fas::system::inet::close( super::_get_wd() );
      super::_set_rstatus(false);
      super::_set_wstatus(false);
      _closed = true;

      typedef typename aa::advice_cast<
         _on_connection_close_,
         aspect
      >::advice on_connection_close;

      static_cast<on_connection_close&>(t)(t);
    }
  }

  template<typename T>
  void _set_timeout(T& t)
  {
  }

private:
  bool _closed;
  address_t _remote_address;
  address_t _local_address;
};

template<typename F, typename A>
struct make_mux_connection_super_class
{
  typedef typename F::template rebind<
       typename aa::aspect_merge<connection_aspect, A>::type
    >::type type;
};



/** Базовый класс-фильтр для работы с инет-соединениями через
  * мултиплексор ввода/вывода. Не создает инет-соединение,
  * но может его закрывать.
  * @param A - пользовательский аспект
  * @param F - базовый аспект-класс-фильтр бинарного ввода/вывода
  *             ( по умолчанию binary_filter<> )
  */
template< typename A = aa::aspect<>,
          typename F = connection<aa::aspect<>, am::mux_filter_base<> > >
class mux_connection_base
  : public make_mux_connection_super_class<F, A>::type
{

public:
  template<typename AA = A, typename FF = F >
  struct rebind
  {
    typedef mux_connection_base<AA,  FF> type;
  };

  typedef mux_connection_base<A, F> self;
  typedef typename make_mux_connection_super_class<F, A>::type super;
  typedef typename super::aspect aspect;
  typedef typename super::read_desc_type read_desc_type;
  typedef typename super::write_desc_type write_desc_type;
  typedef typename super::desc_type desc_type;
  typedef typename super::mutex_type mutex_type;

  virtual ~mux_connection_base()
  {
    // this->release(true);
  }

  /** Инициализация соединения.
    * @param rd - дескриптор для чтения (может совпадать с wd )
    * @param wd - дескриптор для записи (может совпадать с wd )
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
    */
  void assign(read_desc_type rd,
              write_desc_type wd,
              const address_t& local_address,
              const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_assign(*this, rd, wd, ap::cref(local_address), ap::cref(remote_address) );
  }

  /** Инициализация соединения.
    * @param d - дескриптор для чтения/записи 
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
   */
  void assign(desc_type d,
              const address_t& local_address,
              const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    self::_assign(*this, d, d, local_address, remote_address );
  }

  /** Завершает работу с мультиплексором и закрывает соединение.
    * Работает в режимах мягкого и жесткого завершения. При первом вызове 
    * сбрасывается ожидание готовности чтения (т.е. перестает получать 
    * данные) и проверяет наличие данных в выходном буфере. Если данных 
    * нет, то сбрасывает ожидание готовности записи и возвращает true 
    * (перходит в состояние released). Если данные в буфере еще остались, 
    * то остается в режиме 
    * готовности записи ( при условии, что получатель также остается 
    * доступен) и возвращает false (переходит в состояние releasing).
    * В этом состоянии он остается до тех пор, пока не будут переданы
    * все оставшиеся данные, после чего объект автоматически перейдет в 
    * состояние released. Для жесткого завершения (принудительного
    * перехода в состояние released) необходимо повторно вызвать этот 
    * метод, если при первом вызове было возвращено false, но в этом случае
    * все непереданные данные будут утеряны.
    * @return true - работа с мультиплексором завершена
    *         false - перешел в состояние releasing
    */
    /*
    virtual bool release(bool lock = false)
    {
      if ( !lock ) return self::_release(*this);
      typename mutex_type::scoped_lock sl( super::get_mutex() );
      return self::_release(*this);
    }*/


  /** Переводит фильтр на ожидание готовности для записи. Обычно в этот
    * режим фильтр переводиться автоматически, если в выходном буфере остаются
    * данные для записи. Можно использовать этот метод, чтобы заставить 
    * фильтр вызвать _on_ready_write_ при следующем вызове select
    */
  void set_write_handler()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_set_write_handler(*this);
  }

protected:

  template<typename T>
  bool _release(T& t)
  {

    if ( super::_release(t) )
      super::_close(t);
    else
      return false;
    return true;
  }

  template<typename T>
  void _assign(T& t,
                      read_desc_type rd,
                      write_desc_type wd,
                      const address_t& local_address,
                      const address_t& remote_address)
  {
    super::_assign(t, rd, wd, ap::cref(local_address), ap::cref(remote_address) );
  }

};

/*
class iconnection
{
public:
  virtual ~iconnection(){}
  virtual bool release(bool lock = true) = 0;
};
*/
/** Базовый класс-фильтр для работы с инет-соединениями через
  * мултиплексор ввода/вывода. Не создает инет-соединение, 
  * но может его закрывать.
  * @param A - пользовательский аспект
  * @param F - базовый аспект-класс-фильтр бинарного ввода/вывода
  *             ( по умолчанию binary_filter<> )
  */
template< typename A = aa::aspect<>,
          template<typename, typename> class C = mux_connection_base,
          typename F = connection<aa::aspect<>, am::mux_filter_base<> >
        >
class mux_connection
  : public C<A, F>// mux_connection_base<A, F>
  //, public iconnection
{

public:

/*
  template<typename AA = A, typename FF = F >
  struct rebind
  {
    typedef mux_connection<AA,  FF> type;
  };
*/
  template<typename AA = A, /*template<typename, typename> class CC = C,*/ typename FF = F >
  struct rebind
  {
    typedef mux_connection<AA, C, FF > type;
  };


  typedef C<A, F> super;
  // typedef mux_connection_base<A, F> super;
  typedef typename super::desc_type desc_type;
  typedef typename super::mutex_type mutex_type;

  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для чтения. 
    * @param d - дескриптор, готовый для чтения, должен совподать
    *            с базовым дескриптором источника
    */
  virtual void ready_read(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_read(*this, d);
  }
  
  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для записи. 
    * @param d - дескриптор, готовый для записи, должен совпадать
    *            с базовым дескриптором получателя
    */
  virtual void ready_write(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_write(*this, d);
  }

  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для чтения внеполосных данных.
    * @param d - дескриптор, готовый для чтения, должен совподать
    *            с базовым дескриптором источника
    */
  virtual void ready_urgent(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_urgent(*this, d);
  }

  /** Вызывается мультиплексором когда на дескрипторе d произошла ошибка
    * @param d - дескриптор, должен совподать
    *            с базовым дескриптором источника или получателя
    */
  virtual void ready_error(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_error(*this, d);
  }

  /** Инициализация соединения.
    * @param rd - дескриптор для чтения (может совпадать с wd )
    * @param wd - дескриптор для записи (может совпадать с wd )
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
    */
  /*void assign(read_desc_type rd,
              write_desc_type wd,
              const address_t& local_address,
              const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_assign(*this, rd, wd, ap::cref(local_address), ap::cref(remote_address) );
  }*/

  /** Инициализация соединения.
    * @param d - дескриптор для чтения/записи 
    * @param local_address - инет-адрес локального хоста
    * @param remote_address - инет-адрес удаленного хоста
   */
  void assign(desc_type d,
              const address_t& local_address,
              const address_t& remote_address)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_assign(*this, d, d, local_address, remote_address );
  }

  /** Завершает работу с мультиплексором и закрывает соединение.
    * Работает в режимах мягкого и жесткого завершения. При первом вызове 
    * сбрасывается ожидание готовности чтения (т.е. перестает получать 
    * данные) и проверяет наличие данных в выходном буфере. Если данных 
    * нет, то сбрасывает ожидание готовности записи и возвращает true 
    * (перходит в состояние released). Если данные в буфере еще остались, 
    * то остается в режиме 
    * готовности записи ( при условии, что получатель также остается 
    * доступен) и возвращает false (переходит в состояние releasing).
    * В этом состоянии он остается до тех пор, пока не будут переданы
    * все оставшиеся данные, после чего объект автоматически перейдет в 
    * состояние released. Для жесткого завершения (принудительного
    * перехода в состояние released) необходимо повторно вызвать этот 
    * метод, если при первом вызове было возвращено false, но в этом случае
    * все непереданные данные будут утеряны.
    * @return true - работа с мультиплексором завершена
    *         false - перешел в состояние releasing
    */
  virtual bool release(bool lock = false)
  {
    if ( !lock ) return super::_release(*this);
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    return super::_release(*this);

  }

  /** Переводит фильтр на ожидание готовности для записи. Обычно в этот
    * режим фильтр переводиться автоматически, если в выходном буфере остаются
    * данные для записи. Можно использовать этот метод, чтобы заставить 
    * фильтр вызвать _on_ready_write_ при следующем вызове select
    */
  void set_write_handler()
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_set_write_handler(*this);
  }

protected:

};

}}

#endif //FAS_INET_CONNECTION_H
