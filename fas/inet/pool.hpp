//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_INET_POOL_HPP
#define FAS_INET_POOL_HPP

#include <algorithm>
#include <set>
#include <stdexcept>

#include <fas/inet/ipool.hpp>

namespace fas{ namespace inet{



template<typename T, typename M >
class pool
  : public ipool<T>
{
  typedef ipool<T> interface_type;

public:

  typedef pool<T, M> self;
  typedef T pooled_item;
  typedef std::set<pooled_item*> item_list;
  typedef typename item_list::iterator iterator;
  typedef typename item_list::const_iterator const_iterator;
  typedef M mutex_type;

  mutex_type& get_mutex() const { return _mutex;}

  pooled_item& get_prototype() 
  {
    return _prototype;
  }

  pool( )
   : _limit(static_cast<size_t>(-1) )
  {
    _prototype.pool(this); 
  }

  pool(const pool& p)
    : _prototype(p._prototype)
    , _limit(p._limit)
  {
    _prototype.pool(this);
    _active.clear();
    _pooled.clear();
  }

  ~pool() {
    _clear_active();
    _clear_pooled();
  }

  void release()
  {
	{
      typename mutex_type::scoped_lock sl( _mutex );
      _release();
    }
    process_free();
  }
  
  void clear_pooled() {
    typename mutex_type::scoped_lock sl( _mutex );
    _clear_pooled();
  }

  pooled_item* create()
  {
    _process_free();
    typename mutex_type::scoped_lock sl( _mutex );

    pooled_item* item = 0;
    if ( _pooled.empty() )
    {
      if ( _active.size() < _limit )
        item = new pooled_item(_prototype);
      if (item!=0)
        _active.insert( item );
    }
    else
    {
      item = *_pooled.begin();
      _active.insert( item );
      _pooled.erase( _pooled.begin() );
    }
    return item;
  }

  virtual void free(pooled_item* item)
  {
    typename mutex_type::scoped_lock sl( _qmutex );
    _queue.insert(item);
  }

  size_t process_free()
  {
    return _process_free();
  }

  size_t _process_free()
  {
    item_list q;
    typename mutex_type::scoped_lock sql( _qmutex );
    if ( _queue.empty() )
    {
      typename mutex_type::scoped_lock sl( _mutex );
      return _active.size();
    }
    _queue.swap(q);
    sql.unlock();

    typename mutex_type::scoped_lock sl( _mutex );

    typename item_list::iterator beg = q.begin();
    for (;beg!=q.end();++beg)
    {
      typename item_list::iterator itr = _active.find( *beg );
      if ( itr == _active.end() )
        throw std::runtime_error("fas::inet::pool::free: item not found");
      _pooled.insert(*itr);
      _active.erase(itr);
    }

    return _active.size();
  }

  // nonsinchronized
  iterator abegin() { return _active.begin(); }

  iterator aend() { return _active.end(); }

  const_iterator abegin() const { return _active.begin(); }

  const_iterator aend() const { return _active.end(); }

  iterator pbegin() { return _pooled.begin(); }

  iterator pend() { return _pooled.end(); }

  const_iterator pbegin() const { return _pooled.begin(); }

  const_iterator pend() const { return _pooled.end(); }
  
  size_t size() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return active_count() + pooled_count();
  }

  size_t active_count() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _active.size();
  }

  size_t pooled_count() const
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    return _pooled.size();
  }

  void set_limit(size_t value)
  {
    typename mutex_type::scoped_lock sl( self::get_mutex() );
    _limit = value;
  }

private:

  item_list _active;
  item_list _pooled;
  item_list _queue; // Очередь для освобожденных
  pooled_item _prototype;
  size_t _limit;
  mutable mutex_type _mutex; // mutex для активных
  mutable mutex_type _qmutex; // mutex 

  struct deleter{
    void operator()(pooled_item* pitem)
    { delete pitem;}
  };

  struct releaser{
    void operator()(pooled_item* pitem)
    { pitem->release(false);}
  };

  void _clear_pooled() {
    std::for_each(_pooled.begin(), _pooled.end(), deleter());
    _pooled.clear();
  }

  void _clear_active() {
    std::for_each(_active.begin(), _active.end(), deleter());
    _active.clear();
  }
  
  void _release()
  {
    std::for_each(_pooled.begin(), _pooled.end(), releaser());
    std::for_each(_active.begin(), _active.end(), releaser());
  }

};

}}

#endif
