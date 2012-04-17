//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_REF_HPP
#define FAS_PATTERNS_REF_HPP

namespace fas{ namespace pattern {

//  todo warning C4512

  /*
namespace detail
{
  template<typename T>
  struct const_detect
  {
    typedef T type;
    typedef const T const_type;
  };

  template<typename T>
  struct const_detect<const T>
  {
    typedef T type;
    typedef const T const_type;
  };

}
*/

template<typename T>
class reference_wrapper
{
private:
  T& _ref;
public:
  explicit reference_wrapper(T& t): _ref(t){}
  operator T& () { return _ref;}
  reference_wrapper<T>& operator=( const reference_wrapper<T>& ) { return *this;}
private:

};

/*template<typename T>
class reference_wrapper<const T>
{
  typedef reference_wrapper<T> self;
private:
  const T& _ref;
public:
  explicit reference_wrapper(const T& t): _ref(t){}
  operator T& () { return _ref;}
  operator const T& () const { return _ref;}

  reference_wrapper<T>& operator=( const T& v )  { _ref = v ; return *this;}

  reference_wrapper<T>& operator=( const reference_wrapper<T>& ) { return *this;}
};
*/

template<typename T>
inline reference_wrapper<T> ref(T& t) { return reference_wrapper<T>(t); };

template<typename T>
inline reference_wrapper<const T> cref(const T& t) { return reference_wrapper<const T>(t); };


}}

#endif // FAS_PATTERNS_REF_HPP
