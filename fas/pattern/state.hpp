//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_STATE_HPP
#define FAS_PATTERNS_STATE_HPP

//#include <fas/pattern/tags.hpp>
#include <fas/pattern/scatter_hierarchy.hpp>
#include <fas/pattern/conversion.hpp>

#include <stdexcept>

namespace fas{ namespace pattern {

namespace detail
{
  struct no_mutex2 
  {
    struct scoped_lock { scoped_lock(no_mutex2&, bool = true) {} };
    struct scoped_try_lock 
    {
      scoped_try_lock(no_mutex2&, bool) {} 
      bool try_lock() { return false;}
    };
  };
}

class istate_event_sink
{
public:
  virtual void cast_event(int ev) = 0;
protected:
  virtual ~istate_event_sink(){};
};

template<typename I, int i>
class state
  : public int2type<i>
  , public I
{
  istate_event_sink* _event_sink;
public:

  state(): _event_sink(0) {}

  state(const state& s): _event_sink(0) {}

  void cast_event(int ev) { if (_event_sink!=0) _event_sink->cast_event(ev); }

  int get_state_id() const { return state_id;}

  enum { state_id = i};

public:
  /// только  для state_context
  void set_event_sink(istate_event_sink* ises) { _event_sink = ises; }

};


/**
  * @param I - интерфейс состояний
  * @param L -  списко типов состояний
  */
template<typename I, typename SL, typename M = detail::no_mutex2>
class state_context
  : public I
  , public istate_event_sink
{
  typedef state_context<I, SL, M> self;
  typedef typename type_list_traits<SL>::type state_list_type;
  typedef scatter_hierarchy< state_list_type > state_set_type;
  
  typedef I state_interface;

  class context_construct
  {
    typedef state_context<I, SL, M> context_type;
  public:
    context_construct(context_type* cnt):_context(cnt){}
    template<typename T>
    void operator()( T& t) { t.set_event_sink(_context); }
  private:
    context_type* _context;
  };

public:
  typedef M mutex_type;

  state_context(): _current_state_id(0)
  {
    self::for_each(context_construct(this));
    _current_state = &(_state_set.left());
    _current_state_id = _state_set.left().get_state_id();
  }

  state_context( const state_context<I, SL, M>& sc)
    : I(sc)
    , _state_set(sc._state_set)
    , _current_state_id( sc.get_state_id() )
  {
    self::for_each(context_construct(this));
    self::cast_event(_current_state_id);
  }

  template<int i>
  typename left_cast< int2type<i>, state_list_type >::type& get()
  {
    return _state_set.get< int2type<i> >();
  }

  template<int i>
  const typename left_cast< int2type<i>, state_list_type >::type& get() const
  {
    return _state_set.get< int2type<i> >();
  }

  state_interface* get_state() 
  {
    typename mutex_type::scoped_lock sl(_mutex);
    return _current_state;
  }

  const state_interface* get_state() const 
  {
    typename mutex_type::scoped_lock sl(_mutex);
    return _current_state;
  }

  virtual void cast_event(int ev)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    self::_change_state(ev, _state_set);
  };

  template<typename F>
  void for_each(F f) 
  {
    typename mutex_type::scoped_lock sl(_mutex);
    self::_for_each(_state_set, f); 
  }

  int get_state_id() const 
  {
    typename mutex_type::scoped_lock sl(_mutex);
    return _current_state_id;
  }

private:

  template< typename L>
  void _change_state(int ev, scatter_hierarchy<L>& tl)
  {
    if (ev == scatter_hierarchy<L>::left_type::state_id )
    {
      _current_state = &tl.left();
      _current_state_id = tl.left().get_state_id();
    }
    else
      self::_change_state( ev, tl.right() );
  }

  void _change_state(int ev, scatter_hierarchy<empty_type>&)
  {
    throw std::runtime_error("state_context: unknown state");
  }
  

  template<typename L, typename F>
  void _for_each( scatter_hierarchy<L>& tl, F f)
  {
    f(tl.left());
    self::_for_each(tl.right(), f );
  }

  template<typename F>
  void _for_each( scatter_hierarchy<empty_type>&, F) {}

private:
  state_set_type _state_set;
  state_interface* _current_state;
  int _current_state_id;
  mutable mutex_type _mutex;
};

}}

#endif
