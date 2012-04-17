//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_SYSTEM_THREAD_HPP
#define FAS_SYSTEM_THREAD_HPP

#include <fas/unp.h>
#include <fas/system/detail/thread.hpp>
#include "fas/system/system.hpp"

#ifdef HAVE_BOOST_THREAD_HPP
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#endif


namespace fas{ namespace system { namespace thread {

class empty_mutex
{
  empty_mutex(empty_mutex&){}
public:
  empty_mutex(){}
  typedef detail::empty_scoped_lock<empty_mutex> scoped_lock;
};

struct empty_try_mutex
{
  empty_try_mutex(empty_try_mutex&){}
public:
  empty_try_mutex(){}
  typedef detail::empty_scoped_lock<empty_try_mutex> scoped_lock;
  typedef detail::empty_scoped_try_lock<empty_try_mutex> scoped_try_lock;
};

class empty_recursive_mutex
{
  empty_recursive_mutex(empty_recursive_mutex&){}
public:
  empty_recursive_mutex(){}
  typedef detail::empty_scoped_lock<empty_recursive_mutex> scoped_lock;
};

struct empty_recursive_try_mutex
{
  empty_recursive_try_mutex(empty_recursive_try_mutex&){}
public:
  empty_recursive_try_mutex(){}
  typedef detail::empty_scoped_lock<empty_recursive_try_mutex> scoped_lock;
  typedef detail::empty_scoped_try_lock<empty_recursive_try_mutex> scoped_try_lock;
};

#ifdef HAVE_BOOST_THREAD_HPP

typedef boost::mutex mutex;
typedef boost::try_mutex try_mutex;
typedef boost::recursive_mutex recursive_mutex;
typedef boost::recursive_try_mutex recursive_try_mutex;
typedef boost::thread thread;
typedef boost::thread_group thread_group;

#elif WIN32

class mutex
{
  bool _locked;
  CRITICAL_SECTION _cs;
  mutex(mutex&){}
public:
  mutex():_locked(false) 
	{ 
		::InitializeCriticalSection(&_cs);
	}
	
	~mutex()
	{ 
		::DeleteCriticalSection(&_cs);
	}

	void lock() 
	{ 
		::EnterCriticalSection(&_cs); 
		if (_locked)
			throw std::logic_error("recursive lock");
		_locked = true;
	}

	void unlock() 
	{ 
		if (!_locked)
			throw std::logic_error("unlock non lock");
		_locked = false; 
		::LeaveCriticalSection(&_cs); 
	}
	
	bool locked() const 
	{ 
		return _locked; 
	}
    
	typedef detail::scoped_lock<mutex> scoped_lock;

};

typedef mutex recursive_mutex;

/// ///////// 

class thread
{
  
  HANDLE _hThread;
public:

  template<typename F>
  static void __cdecl thread_func(void* param) 
  {
    F* f = (reinterpret_cast<F*>(param));
    try
    {
      (*f)();
    }
    catch(...)
    {
      delete f;
      throw;
    }
    delete f;
    _endthread( );
  }

  template<typename F>
  explicit thread(F f) : _hThread(NULL)
  {
    uintptr_t ptrThread = _beginthread(thread_func<F>, 0, new F( f ) );
    _hThread = HANDLE(ptrThread);
  }

  void join() 
  {
    ::WaitForSingleObject(_hThread, INFINITE);
  }
};

#endif

#ifndef WIN32

/*
class thread
{
  pthread_t thread;
  static void *thread_func(void *d)
  {
    ((thread *)d)->run();
  }
public:
  Thread(){}

  virtual ~Thread(){}

  virtual void run(){}

  int start()
  {
    return pthread_create(&thread, NULL, Thread::thread_func, (void*)this);
  }

  int wait()
  {
    return pthread_join(thread, NULL);
  }
};*/
 

#else

#endif

}}}

#endif
