//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_SYSTEM_DETAIL_THREAD_HPP
#define FAS_SYSTEM_DETAIL_THREAD_HPP


namespace fas{ namespace system { namespace thread { namespace detail{

template <typename M>
class empty_scoped_lock
{
  empty_scoped_lock(empty_scoped_lock&){}
public:
  typedef M mutex_type;

  explicit empty_scoped_lock(mutex_type&, bool = true) { }

  ~empty_scoped_lock() { }

  void lock() { }

  void unlock() { }

  bool locked() const { return false; }

  operator const void*() const { return 0; }
};

template <typename M>
class empty_scoped_try_lock
{
  empty_scoped_try_lock(empty_scoped_try_lock&){}
public:
  typedef M mutex_type;

  explicit empty_scoped_try_lock(mutex_type& ) { }

  empty_scoped_try_lock(mutex_type& mx, bool) { }

  ~empty_scoped_try_lock() { }

  void lock(){ }

  bool try_lock() { return true; }

  void unlock() { }

  bool locked() const { return false; }

  operator const void*() const { return 0; }
};

template <typename M>
class scoped_lock
{
  scoped_lock(scoped_lock&){}
public:
  typedef M mutex_type;

  explicit scoped_lock(mutex_type& m, bool lock = true):_m(m) { if (lock) _m.lock(); }

  ~scoped_lock() { if ( _m.locked() ) _m.unlock(); }

  void lock() { _m.lock(); }

  void unlock() { _m.unlock();}

  bool locked() const { return _m.locked(); }

  operator const void*() const { return &_m; }

private:
  mutex_type& _m;
};

template <typename M>
class scoped_try_lock
{
  scoped_try_lock(scoped_try_lock&){}
public:
  typedef M mutex_type;

  explicit scoped_try_lock(mutex_type& ) { }

  scoped_try_lock(mutex_type& mx, bool) { }

  ~scoped_try_lock() { }

  void lock(){ }

  bool try_lock() { return true; }

  void unlock() { }

  bool locked() const { return false; }

  operator const void*() const { return 0; }
};


}}}}

#endif
