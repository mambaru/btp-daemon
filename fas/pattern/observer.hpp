//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_OBSERVER_HPP
#define FAS_PATTERNS_OBSERVER_HPP

#include <set>
#include <queue>
#include <iostream>

#include <fas/system/thread.hpp>

namespace fas{ namespace pattern {

/*namespace detail
{
  struct no_mutex 
  {
    struct scoped_lock 
    {
      scoped_lock(no_mutex&, bool = true) {} 
      void lock(){}
      void unlock(){}
    };

    struct scoped_try_lock 
    {
      scoped_try_lock(no_mutex&, bool) {}
      bool try_lock() { return false;}
      void lock(){}
      void unlock(){}
    };
  };
}*/

template<typename I, typename T, typename M = fas::system::thread::empty_try_mutex/*detail::no_mutex*/>
class observer;

  class scoped_fire
  {
    bool& _fire;
  public:
    scoped_fire(bool& f): _fire(f) { _fire = true; }
    ~scoped_fire() { _fire = false;}
  };

template<typename I, typename T, typename M>
class observer<I, T*, M>
{
  typedef std::pair< I, T* > observer_pair;
  typedef std::pair< T*, I > reverse_pair;
  typedef std::set< observer_pair > observers_set;
  typedef std::set< reverse_pair > reverse_set;
  typedef std::queue< std::pair<bool, observer_pair> > observers_queue;
  typedef M mutex_type;


public:
  typedef typename observers_set::size_type size_type;

  observer(): _fire(false) {}
  
  size_type size() const
  {
    typename mutex_type::scoped_lock sl(_mutex);
    return _observers.size(); 
  }

  void insert(I i, T* t)
  {
    typename mutex_type::scoped_try_lock sl(_mutex/*, false*/);
    if (!sl.try_lock())
    {
      typename mutex_type::scoped_lock sql(_qmutex);
      _queue.push( std::make_pair(true, std::make_pair(i, t) ) );
    }
    else
    {
      if (_fire)
      {
        typename mutex_type::scoped_lock sql(_qmutex);
        _queue.push( std::make_pair(true, std::make_pair(i, t) ) );
      }
      else
      {
        _observers.insert( std::make_pair(i, t) );
        _reverse_set.insert( std::make_pair(t, i) );
        _qprocess();
      }
    }
  }

  void erase(I i, T* t)
  {
    typename mutex_type::scoped_try_lock sl(_mutex/*, false*/); // TODO:
    if (!sl.try_lock())
    {
      typename mutex_type::scoped_lock qls(_qmutex);
      _queue.push( std::make_pair(false, std::make_pair(i, t) ) );
    }
    else
    {
      _observers.erase( std::make_pair(i, t) );
      _reverse_set.erase( std::make_pair(t, i) );
      _qprocess();
    }
  }

  void erase(T* t)
  {
    typename mutex_type::scoped_try_lock sl(_mutex/*, false*/); // TODO
    if (!sl.try_lock())
    {
      typename mutex_type::scoped_lock qls(_qmutex);
      typename reverse_set::iterator itr = _reverse_set.lower_bound( std::make_pair(t, I() ) );
      for (;itr!=_reverse_set.end() && itr->first==t; ++itr)
         _queue.push( std::make_pair(false, std::make_pair(itr->second, itr->first) ) );
    }
    else
    {
      typename reverse_set::iterator itr =
        _reverse_set.lower_bound( std::make_pair(t, I() ) );

      if (_fire)
      {
        for (;itr!=_reverse_set.end() && itr->first==t; ++itr)
        _queue.push( std::make_pair(false, std::make_pair(itr->second, itr->first) ) );
      }
      else
      for (;itr!=_reverse_set.end() && itr->first==t;)
      {
        _observers.erase( std::make_pair(itr->second, itr->first) );
        _reverse_set.erase( itr++ );
      }
      _qprocess();
    }
  }

  void _qprocess()
  {
    typename mutex_type::scoped_lock qsl(_qmutex);

    while ( !_queue.empty() )
    {
       if (_queue.front().first)
       {
         typename observers_set::iterator itr = _observers.insert( _queue.front().second ).first;
         _reverse_set.insert( std::make_pair(itr->second, itr->first) );
       }
       else
       {
         _observers.erase( _queue.front().second );
         _reverse_set.erase( std::make_pair(_queue.front().second.second, _queue.front().second.first) );
       }
       _queue.pop();
    }
  }

  void erased_fire( I i, void (T::*mem_fun)())
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; )
    {
      (itr->second->*mem_fun)();
      _reverse_set.erase( std::make_pair(itr->second, itr->first) );
      _observers.erase(itr++);
    }
  }


  template<typename P0>
  void erased_fire( I i, void (T::*mem_fun)(P0), P0 p0)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; )
    {
      (itr->second->*mem_fun)(p0);
      _reverse_set.erase( std::make_pair(itr->second, itr->first) );
      _observers.erase(itr++);
    }
  }

  template<typename P0, typename P1>
  void erased_fire( I i, void (T::*mem_fun)(P0, P1), P0 p0, P1 p1)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; )
    {
      (itr->second->*mem_fun)(p0, p1);
      _reverse_set.erase( std::make_pair(itr->second, itr->first) );
      _observers.erase(itr++);
    }
    
  }


  template<typename P0, typename P1, typename P2>
  void erased_fire( I i, void (T::*mem_fun)(P0, P1, P2), P0 p0, P1 p1, P2 p2)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; )
    {
      (itr->second->*mem_fun)(p0, p1, p2);
      _reverse_set.erase( std::make_pair(itr->second, itr->first) );
      _observers.erase(itr++);
    }
    _qprocess();
  }


  void fire( I i, void (T::*mem_fun)())
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; ++itr)
      (itr->second->*mem_fun)();
  }


  template<typename P0>
  void fire( I i, void (T::*mem_fun)(P0), P0 p0)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; ++itr)
      (itr->second->*mem_fun)(p0);
  }

  template<typename P0, typename P1>
  void fire( I i, void (T::*mem_fun)(P0, P1), P0 p0, P1 p1)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; ++itr)
      (itr->second->*mem_fun)(p0, p1);
  }


  template<typename P0, typename P1, typename P2>
  void fire( I i, void (T::*mem_fun)(P0, P1, P2), P0 p0, P1 p1, P2 p2)
  {
    typename mutex_type::scoped_lock sl(_mutex);
    scoped_fire sf(_fire);
    _qprocess();

    typename observers_set::iterator itr = _observers.lower_bound( std::make_pair(i, static_cast<T*>(0)) );
    for ( ; itr!=_observers.end() && itr->first==i; ++itr)
      (itr->second->*mem_fun)(p0, p1, p2);
  }

private:
  observers_set _observers;
  reverse_set _reverse_set;

  observers_queue _queue;
  mutex_type _mutex;
  mutex_type _qmutex;
  bool _fire;

};

}}

#endif
