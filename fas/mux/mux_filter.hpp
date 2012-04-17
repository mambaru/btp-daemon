//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_MUX_MUX_FILTER_HPP
#define FAS_MUX_MUX_FILTER_HPP

#include <iostream>
#include <fas/mux/imux.hpp>
#include <fas/mux/imux_observer.hpp>
#include <fas/aop/aspect.hpp>
#include <fas/aop/advice.hpp>
#include <fas/adv/ad_stub.hpp>
#include <fas/filter/binary_filter.hpp>
#include <fas/pattern/type_list.hpp>
#include <fas/pattern/final.hpp>
#include <fas/adv/io/ad_sock.hpp>

#include <string>
#include <sstream>

namespace fas { namespace mux {

class mux_filter_error:
  public std::logic_error
{
  static std::string _make_messasge( const std::string& str, int d1, int d2 )
  {
    std::stringstream ss;
    ss << str << ": The difference between the descriptors "<< "(" << d1 << " != " << d2 <<")";
    return ss.str();
  }
public:
  mux_filter_error(const std::string& str): std::logic_error(str){}
  mux_filter_error(const std::string& str, int d1, int d2)
    : std::logic_error( _make_messasge( str, d1, d2 ) )
  {
  }
};

namespace aa = ::fas::aop;
namespace ad = ::fas::adv;
namespace adio = ::fas::adv::io;
namespace ap = ::fas::pattern;
namespace af = ::fas::filter;

struct ad_release
{
  template<typename T, typename D>
  void operator()(T& t, D) {  t.release(false); }
};

struct ad_exception
{
  template<typename T>
  void operator()(T& t) 
  {
    t.release(false);
    t.release(false);
    throw;
  }
};

struct _on_ready_read_{};
struct _on_ready_write_{};
struct _on_ready_urgent_{};
struct _on_ready_error_{};

struct _e_ready_read_{};
struct _e_ready_write_{};
struct _e_ready_urgent_{};
struct _e_ready_error_{};

typedef aa::tag_list_n<
          _e_ready_read_,
          _e_ready_write_,
          _e_ready_urgent_,
          _e_ready_error_
        >::type mux_on_error_tags;

/*
typedef aa::tag_list_n<
          _on_ready_read_,
          _on_ready_write_,
          _on_ready_urgent_,
          _on_ready_error_
        >::type mux_on_tags;
struct mux_on_stub: aa::advice< mux_on_tags, ad::ad_stub<> > {};
*/

struct mux_on_stub: aa::advice< aa::tag_list_n<_on_ready_read_, _on_ready_write_>::type, ad::ad_stub<> > {};
struct mux_on_release: aa::advice< aa::tag_list_n<_on_ready_urgent_, _on_ready_error_>::type, ad_release > {};
struct mux_e : aa::advice< mux_on_error_tags, ad_exception > {};

struct recv_advice: aa::advice< aa::tag<af::_read_>, adio::ad_recv > {};
struct send_advice: aa::advice< aa::tag<af::_write_>, adio::ad_send > {};

#ifdef WIN32_1

struct binary_read_advice
  : aa::advice<
      aa::tag<af::_binary_read_>, 
      af::ad_buf_binary_read<af::_on_read_, af::_on_rclosed_, af::_on_rerror_, descriptor_t> 
    >
{};

struct binary_write_advice
  : aa::advice<
      aa::tag<af::_binary_write_>,
      af::ad_buf_binary_write<af::_on_write_, af::_on_wclosed_, af::_on_werror_, descriptor_t> 
    >
{};

#endif

struct mux_filter_advice_list
 : ap::type_list_n<
      mux_on_stub,
      mux_on_release,
      mux_e
/*,
      aa::alias< af::_cleaner_advice_, _on_release_ >*/
#ifdef WIN32
      , recv_advice
      , send_advice
#endif
 >::type
{};

struct mux_filter_aspect: aa::aspect<mux_filter_advice_list>{};

template< typename F, typename A>
struct make_mux_filter_super_class
{
  typedef typename F::template rebind<
      typename aa::aspect_merge<mux_filter_aspect, A>::type
    >::type type;
};


struct d_state_info
{
  size_t ready_read_count;
  size_t ready_write_count;
  size_t ready_urgent_count;
  size_t ready_error_count;
  int swh_count;
  int recursive;
  bool mux_read;
  bool mux_write;
  bool mux_urgent;
  bool released;
  bool releasing;

  bool read_status;
  bool write_status;
  bool write_ready_status;
};


/** Абстрактный фильтр реализующий взаимодействие
  * мультиплексора ввода/вывода с классом-фильтром.
  * @advice on_ready_read(desc_type)
  * @advice on_ready_write(desc_type)
  * @advice on_ready_urgent(desc_type)
  * @advice on_ready_error(desc_type)
  * @advice on_release(desc_type)
  * @param A - пользовательский аспект (по умолчанию aspect<> )
  * @param F - базовый фильтр бинарного ввода/вывода
  *             ( по умолчанию binary_filter<> )
  */
template< typename A = aa::aspect<>,
          typename F = af::binary_filter<>
        >
class mux_filter_base
  : public imux_observer< typename make_mux_filter_super_class<F, A>::type::desc_type >
  , public make_mux_filter_super_class<F, A>::type
{
  size_t _d_ready_read_count;
  size_t _d_ready_write_count;
  size_t _d_ready_urgent_count;
  size_t _d_ready_error_count;

public:

  typedef typename make_mux_filter_super_class<F, A>::type super;
  typedef typename super::desc_type desc_type;

  typedef class imux<desc_type> imux;
  typedef mux_filter_base<A, F> self;

  typedef typename super::aspect aspect;

  /*
    friend class aa::advice_cast<_reader_, aspect>::type; 
    friend class aa::advice_cast<_writer_, aspect>::type; 
  */

  typedef typename super::read_return_type read_return_type;
  typedef typename super::write_return_type write_return_type;
  typedef typename super::read_size_type read_size_type;
  typedef typename super::write_size_type write_size_type;

  typedef typename super::size_type size_type;

  typedef typename super::mutex_type mutex_type;


  template<typename AA = A, typename FF = F>
  struct rebind
  {
    typedef mux_filter_base< AA, FF > type;
  };

  /** конструктор */
  mux_filter_base()
    :
    _d_ready_read_count(0)
    , _d_ready_write_count(0)
    , _d_ready_urgent_count(0)
    , _d_ready_error_count(0)
    , _mux(0)
    , _swh_count(0)
    , _recursive(0)
    , _mux_read(false)
    , _mux_write(false)
    , _mux_urgent(false)
    , _released(true)
    , _releasing(false)
  {}

  /** конструктор копирования */
  mux_filter_base(const mux_filter_base& mf)
    : super( static_cast<const super&>(mf))
    , _d_ready_read_count(0)
    , _d_ready_write_count(0)
    , _d_ready_urgent_count(0)
    , _d_ready_error_count(0)
    , _mux(mf._mux)
    , _swh_count(0)
    , _recursive(0)
    , _mux_read(false)
    , _mux_write(false)
    , _mux_urgent(false)
    , _released(true)
    , _releasing(false)
  {
    /*if (_mux_read)
      abort();*/
  }

  virtual ~mux_filter_base()
  {
  }

  void debug_get_state_info(d_state_info& dsi) const
  {
    dsi.ready_read_count = _d_ready_read_count;
    dsi.ready_write_count = _d_ready_write_count;
    dsi.ready_urgent_count = _d_ready_urgent_count;
    dsi.ready_error_count = _d_ready_error_count;
    dsi.swh_count = _swh_count;
    dsi.recursive = _recursive;
    dsi.mux_read = _mux_read;
    dsi.mux_write = _mux_write ;
    dsi.mux_urgent = _mux_urgent;
    dsi.released = _released;
    dsi.releasing = _releasing;

    dsi.read_status = super::_get_wstatus();
    dsi.write_status = super::_get_rstatus();
    dsi.write_ready_status = super::_get_writer().ready();
  };

  /** Задает мультиплексор ввода/вывода 
    * @param m - указатель на интерфейс мультиплексора */
  void set_mux(imux* m)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    _mux = m;
  }

  /** Осуществляет чтение данных в пользовательский 
    * буффер. Если во время операции источник стал
    * закрыт, то вызывает метод release() */
  read_return_type read(char* d, read_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this, d, s);
  }

  /** Осуществляет чтение данных во внутренний буфер
    * буффер. Если во время операции источник стал
    * недоступен, то вызывает метод release() */
  read_return_type read()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_read(*this);
  }

  /** Осуществляет запись данных из пользовательского
    * буфера. Если во время операции получатель стал
    * недоступен, то вызывает метод release() */
  write_return_type write(const char* d, write_size_type s)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this, d, s);
  }

  /** Осуществляет запись данных из внутреннего
    * буфера. Если во время операции получатель стал
    * недоступен, то вызывает метод release() */
  write_return_type write()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_write(*this);
  }

 /** Инициализирует фильтр дескрипторами чтения и записи
    * @advice on_assign(read_desc_type, write_desc_type )
    * @param wd - дескриптор чтения
    * @param rd - дескриптор записи
    */
  void assign( desc_type rd, desc_type wd )
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_assign(*this, rd, wd);
  }

  /** Инициализирует фильтр общим дескрипторам чтения и записи
    * @advice on_assign(desc_type, desc_type )
    * @param d - дескриптор чтения/записи
    */
  void assign( desc_type d)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_assign(*this, d, d);
  }

  

  /** Возвращает true если не находится в состоянии released. 
    * В состоянии releasing вы можете продолжать читать и писать
    * данные при условии, что источник и получатель позволяют 
    * это делать (не были закрыты). */
  bool get_status(/*bool lock = true*/) const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return self::_get_status();

    /*
    if (!lock) return !_released;
    typename mutex_type::scoped_lock sl( self::get_mutex(), lock ); //TODO:
    return !_released;*/
  }

  operator bool () const  { return self::get_status(); }


  /** Завершает работу с мультиплексором. Работает в режимах мягкого
    * и жесткого завершения. При первом вызове сбрасывается ожидание 
    * готовности чтения (т.е. перестает получать данные) и проверяет 
    * наличие данных в выходном буфере. Если данных нет, то сбрасывает
    * ожидание готовности записи и возвращает true (перходит в состояние 
    * released). Если данные в буфере еще остались, то остается в режиме 
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
  virtual bool release(bool lock = false ) = 0;

  /** Переводит фильтр на ожидание готовности для записи. Обычно в этот
    * режим фильтр переводиться автоматически, если в выходном буфере остаются
    * данные для записи. Можно использовать этот метод, чтобы заставить 
    * фильтр вызвать _on_ready_write_ при следующем вызове select
    */
  void set_write_handler()
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    self::_set_write_handler(*this);
  }
  
  int get_recursive_count() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _recursive;
  }

protected:

  bool _get_status() const { return !_released; }

  template<typename T>
  void _ready_read(T& t, desc_type d)
  {
    try
    {
      ++_d_ready_read_count;
      if (d!=super::_get_rd() )
        throw mux_filter_error("mux_filter::_ready_read", super::_get_rd(), d);

      if (_recursive!=0) 
        throw std::logic_error("recursive call forbidden");

      ++_recursive;

      try
      {
        // TODO: t.get_aspect().template get< _on_ready_read_ >()(t, d); поставить перед self::_read(t);? для статистики
        t.get_aspect().template get< _on_ready_read_ >()(t, d);
        self::_read(t);
      }
      catch(...)
      {
        --_recursive;
        throw;
      }
      --_recursive;
    }
    catch(...)
    {
      t.get_aspect().template get< _e_ready_read_ >()(t);
    }
  }

  template<typename T>
  void _ready_write(T& t, desc_type d) 
  {
    try
    {
      ++_d_ready_write_count;
      if (d!=super::_get_wd() )
        throw mux_filter_error("mux_filter::_ready_write", super::_get_wd(), d);
      // throw std::logic_error("_ready_urgent: invalid descriptor");


      if (_recursive!=0) 
        throw std::logic_error("recursive call forbidden");
      ++_recursive;
      try
      {
        self::_write(t);
        t.get_aspect().template get< _on_ready_write_ >()(t, d);
      }
      catch(...)
      {
        --_recursive;
        throw;
      }
      --_recursive;
    }
    catch(...)
    {
      t.get_aspect().template get< _e_ready_write_ >()(t);
    }
  }

  template<typename T>
  void _ready_urgent(T& t, desc_type d)
  {
    try
    {
      ++_d_ready_urgent_count;

      if (d!=super::_get_wd() || d!=super::_get_rd())
        throw mux_filter_error("mux_filter::_ready_urgent", super::_get_rd(), d);
      // throw std::logic_error("_ready_urgent: invalid descriptor");

      if (_recursive!=0) 
        throw std::logic_error("recursive call forbidden");
      ++_recursive;
      try
      {
        // по умолчанию для _on_ready_error_ определен ad_release
        t.get_aspect().template get< _on_ready_urgent_ >()(t, d);
      }
      catch(...)
      {
        --_recursive;
        throw;
      }
      --_recursive;
    }
    catch(...)
    {
      t.get_aspect().template get< _e_ready_urgent_ >()(t);
    }
  }

  template<typename T>
  void _ready_error(T& t, desc_type d)
  {
    
    try
    {
      ++_d_ready_error_count;
      if (d!=super::_get_wd() || d!=super::_get_rd())
        throw mux_filter_error("mux_filter::_ready_error", super::_get_rd(), d);
      // throw std::logic_error("_ready_urgent: invalid descriptor");

      if (_recursive!=0) 
        throw std::logic_error("recursive call forbidden");
      ++_recursive;
      try
      {
        // TODO: а нужно ли здесь делать release или предоставить пользователю?
        // Если пользователь не реализует, то сервер уйдет в 100% т.к. для сокета будет 
        // всегда срабатывать _ready_error
        /*if (!_released)
          this->release();
         */
        // по умолчанию для _on_ready_error_ определен ad_release
        t.get_aspect().template get< _on_ready_error_ >()(t, d);
      }
      catch(...)
      {
        --_recursive;
        throw;
      }
      --_recursive;
    }
    catch(...)
    {
      t.get_aspect().template get< _e_ready_error_ >()(t);
    }
  }

  template<typename T>
  void _assign( T& t, desc_type rd, desc_type wd )
  {

    if (!_released)
      this->release();


    // Это не ошибка, при втором вызове принудительно закрываем запись, 
    // даже если еще остались данные
    if (!_released)
      this->release();


    _releasing = false; // на всякий случАй
    //_swh_count = 0;
    // Важно: во время _assign может быть вызван release
    _released = false;

    /// ахтунг: во время вызова _assign может быть вызван release() а также функции записи
    /// во время которых может быть установлен или сброшен write_handler
    super::_assign(t, rd, wd);

    if ( _mux!=0 && super::_get_rstatus())
    {
      
      _mux->set_rhandler(rd, &t);
      _set_write_handler_if_ready(t);
      _mux_read = true;
    }
  }

  template<typename T>
  bool _release(T& t)
  {

    if (_released)
      return false;

    /*if (_released)
      return true; // ??!! почему был false
    */

    _released = true;

    if ( _mux_read && _mux!=0 )
    {
      _mux->reset_rhandler( super::_get_rd() );
      _mux_read = false;
    }

    _reset_urgent_handler(t);

    super::_set_rstatus(false);

    if ( _mux_write && _mux!=0 )
    {
      // при повторном вызове происходит принудительное закрытие
      if ( !_releasing && super::_get_writer().ready() )
      {
        _released = false;
        _releasing = true;
      }
      else
      {
        _reset_write_handler(t, true);
        super::_set_wstatus(false);
        _releasing = false;
      }
    }

    if ( _released )
    {
      super::_release(t);
    }

    
    //!? _swh_count = 0;

    return _released;
  }

  template<typename T>
  void _set_write_handler(T& t)
  {

    if (_mux == 0) return;

    if (super::_get_wstatus() )
    {

      if (!_mux_write)
      {

        _mux->set_whandler( super::_get_wd(), &t );
      }
      _mux_write = true;
      _swh_count++;
    }
  }

  template<typename T>
  void _reset_write_handler(T& t, bool all)
  {

    if (_mux == 0) return;

    if ( _swh_count > 0 )
    {
      if ( _swh_count == 1 || all )
      {
        _mux_write = false;
        _swh_count = 0;
        _mux->reset_whandler( super::_get_wd() );
      }
      else
        _swh_count--;
    }
  }

  // urgent используется как except в windows (для connect )  !!! отладить под линухом
  template<typename T>
  void _reset_urgent_handler(T& t)
  {
    if (_mux == 0) return;
    
    if ( _mux_urgent )
    {
      _mux_urgent = false;
      _mux->reset_uhandler( super::_get_rd() );
    }
  }

  template<typename T>
  void _set_urgent_handler(T& t)
  {
    if (_mux == 0) return;

    if ( super::_get_rstatus() )
    {
      _mux_urgent = true;
      _mux->set_uhandler( super::_get_rd(), &t );
    }
  }

public:

  template<typename T>
  read_return_type _read( T& t, char* d, read_size_type s )
  {
    read_return_type rt = super::_read( t, d, s );

    if ( !super::_get_rstatus() && !_releasing && !_released )
      this->release();

    return rt;
  }

  template<typename T>
  read_return_type _read(T& t)
  {
    read_return_type rt = super::_read(t);

    if ( !super::_get_rstatus() && !_releasing && !_released )
      this->release();

    return rt;
  }

  template<typename T>
  write_return_type _write(T& t, const char* d, write_size_type s)
  {

    if (_releasing)
      return 0;

    if ( !super::_get_wstatus() )
    {
      //! Добавлено (сервер уходил в 100%)
      _reset_write_handler(t, true);
      return 0;
    }

    write_return_type rt = super::_write(t, d, s);

    _set_write_handler_if_ready(t);

// #warning добавлено 17.06.2010
    if ( !super::_get_wstatus() && !_releasing && !_released )
      this->release();

    return rt;
  }

  template<typename T>
  write_return_type _write(T& t)
  {
    if ( !super::_get_wstatus() )
    {
      //!  25 янв 2010 Добавлено (сервер уходил в 100%)
      _reset_write_handler(t, true);

      // _set_write_handler_if_ready(t);
      return 0;
    }


    write_return_type rt = 0;

    // если в буфере есть данные, то скидываем их
    if ( super::_get_writer().ready() )
      rt = super::_write(t);

    _set_write_handler_if_ready(t);

//#warning добавлено 17.06.2010
    if ( !super::_get_wstatus() && !_releasing && !_released )
      this->release();

    return rt;
  }

  // bool _is_releasing() const { return _releasing; }

private:

  template<typename T>
  void _set_write_handler_if_ready(T& t)
  {

    if ( _released ) return;

    if ( super::_get_wstatus()  )
    {
      if ( super::_get_writer().ready() )
      {
        _set_write_handler(t);
      }
      else
      {
        _reset_write_handler(t, false);
        if (_releasing)
        {
          this->release(false);
        }
      }
    }
    else
    {
      this->release( false );
    }
   /*
    if ( super::_get_wstatus()  )
    {
      if ( super::_get_writer().ready() )
      {
        _set_write_handler(t);
      }
      else
        _reset_write_handler(t, false);
    }
    else
    {
      _reset_write_handler(t, false);
      /// здесь нужен release
      if (!_released)
        this->release();    }
    */

    /** - не убивать, новый не испытан (были заморочки при вызове релиз при ассигне)

    if ( super::_get_wstatus()  )
    {

      if ( _mux != 0 && !_mux_write && ( super::_get_writer().ready() || _swh_count!=0) )
      {

        _mux->set_whandler( super::_get_wd(), &t );
        _mux_write = true;
        if (_swh_count!=0) --_swh_count;
      }
      else if ( _mux != 0 && _mux_write && !super::_get_writer().ready() )
      {

        _mux->reset_whandler( super::_get_wd());
        _mux_write = false;
        _swh_count = 0;
      }
    }
    else
    {

      if ( _mux!=0 && _mux_write  )
        _mux->reset_whandler( super::_get_wd() );
      _mux_write = false;
      _swh_count = 0;
      if (!_released)
        this->release();
    }
    */
  }

  imux* _mux;
  int _swh_count;    // Пользователь может сам установить write _handler ( счетчик чтобы не сбрасывать после записи буфера) - заменить на флаг
  int _recursive;    // счетчик рекурсии
  bool _mux_read;    // Задали mux на чтение
  bool _mux_write;   // Задали mux на запись
  bool _mux_urgent;   // Задали mux 
  double _tmp;
  bool _released;    // Ресурсы свободны
  bool _releasing;   // Начат процесс освобождения ресурсов
};


/** Аспект-класс-фильтр реализующий интерфейс взамодействия
  * с мультиплексорором ввода/вывода. Это финальный класс - 
  * вы не можете наследовать этот класс.
  * @param A - пользовательский аспект (по умолчанию aspect<> )
  * @param F - базовый аспект-класс-фильтр бинарного ввода/вывода
  *             ( по умолчанию binary_filter<> )
  */

template< typename A = aa::aspect<>,
          template<typename, typename> class MFB = mux_filter_base,
          typename F = af::binary_filter<>
        >
class mux_filter
  : public MFB<A, F>
{
  typedef MFB<A, F> super;

public:
  typedef typename super::desc_type desc_type;
  typedef typename super::mutex_type mutex_type;
  template<typename AA = A, template<typename, typename> class MFBMFB = MFB, typename FF = F>
  struct rebind
  {
    typedef mux_filter< AA, MFBMFB, FF > type;
  };

  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для чтения. 
    * @param d - дескриптор, готовый для чтения, должен совподать
    *            с базовым дескриптором источника
    */
  virtual void ready_read( desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_read( *this, d );
  }

  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для записи. 
    * @param d - дескриптор, готовый для записи, должен совпадать
    *            с базовым дескриптором получателя
    */
  virtual void ready_write( desc_type d) 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_write( *this, d );
  }

  /** Вызывается мультиплексором когда дескриптор d становится
    * готовым для чтения внеполосных данных (только для сокетов). 
    * @param d - дескриптор, готовый для чтения, должен совподать
    *            с базовым дескриптором источника
    */
  virtual void ready_urgent(desc_type d) 
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_urgent( *this, d );
  }

  /** Вызывается мультиплексором когда на дескрипторе d произошла ошибка
    * @param d - дескриптор, должен совподать
    *            с базовым дескриптором источника или получателя
    */
  virtual void ready_error(desc_type d)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex() );
    super::_ready_error( *this, d );
  }

  /** Завершает работу с мультиплексором. Работает в режимах мягкого
    * и жесткого завершения. При первом вызове сбрасывается ожидание 
    * готовности чтения (т.е. перестает получать данные) и проверяет 
    * наличие данных в выходном буфере. Если данных нет, то сбрасывает
    * ожидание готовности записи и возвращает true (перходит в состояние 
    * released). Если данные в буфере еще остались, то остается в режиме 
    * готовности записи ( при условии, что получатель также остается 
    * доступен) и возвращает false (переходит в состояние releasing).
    * В этом состоянии он остается до тех пор, пока не будут переданы
    * все оставшиеся данные, после чего объект автоматически перейдет в 
    * состояние released. Для жесткого завершения (принудительного
    * перехода в состояние released) необходимо повторно вызвать этот 
    * метод, если при первом вызове было возвращено false, но в этом случае
    * все непереданные данные будут утеряны.
    * @return true - работа с мультиплексором завершена
    *         false - перешел в состояние releasing, либо это повторный вызов
    *
    * Итак при нескольких вызовах release(
    *   false->false->false ... (небыл сделан assign)
    *   true->false->false ... (режим мягкого завершения)
    *   false->true->false ... (режим жесткого завершения)
    */
  virtual bool release(bool lock = false)
  {
    typename mutex_type::scoped_lock sl( super::get_mutex(), lock );
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

};

}}
#endif
