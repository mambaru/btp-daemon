#ifndef FAS_MISC_JSON_HPP
#define FAS_MISC_JSON_HPP

#include <stdexcept>
#include <vector>
#include <string>
#include <set>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <unordered_map>
#else
#include <map>
#endif


#include <fas/pattern/type_list.hpp>


namespace fas{ namespace json{

namespace ap = ::fas::pattern;


/// /////////////////////////////////////////////////////

template<typename T>
struct value;

template<typename T = std::string>
struct raw_value;

/*
template<typename L, typename R>
struct if_def;
*/

template<typename N, typename T, T v>
struct enum_value;

template< typename T, typename L>
struct enumerator;

template<typename T, typename L>
struct object;

template<typename C>
struct array;

template<typename K, typename V>
struct pair;

/*сериализация одного мембера объекта как значения.  
  * длс a{ bool v };  b: a{}
    member_value<a, a, bool, &a::v>
    member_value<b, a, bool, &a::v>
 * member_value
*/
template<typename T, typename V, typename M, M V::* m, typename W = value<M> >
struct member_value;

template<typename N,
         typename T,
         typename M,
         M T::* m,
         typename W = value<M> >
struct member;

template<typename N,
         typename T,
         typename M,
         typename G, // getter
         typename W = value<M>
        >
struct member_p;

/** Ахтунг! замороченые правила:
  * L и M типа member с одинаковыми N (именами)
  * сериализация:
  *   если значение мембера L не равно значению по умолчанию, то сериализуется L, в противном случае R
  * десериализация:
  *   всегда десереализуется в R, а L игнорируется
  * Используется для чего-чего?:
  *   L мембер типа raw_value (т.е. строка голого json как есть) и если он не пустой, то просто копируем в поток эту строку,
  *   если L пустой, то применяем сериализацию R. Соответственно если попытатся десериализовать, то сразу десериализуется по правилам в R.
  *   но если RU задать как false, то копируем в raw_value значение мембера
  */
template<typename L, typename R, bool RU = true >
struct member_if;

template<typename J>
class serializerT;

/// //////////////////////////////////////////////////////// 

/// /////////////////////////////////////////////////////

template<>
struct value<char>
{
  typedef char target;
  typedef serializerT< value<char> > serializer;
};

template<>
struct value<unsigned char>
{
  typedef unsigned char target;
  typedef serializerT< value<unsigned char> > serializer;
};

template<>
struct value<short>
{
  typedef short target;
  typedef serializerT< value<short> > serializer;
};

template<>
struct value<unsigned short>
{
  typedef unsigned short target;
  typedef serializerT< value<unsigned short> > serializer;
};


template<>
struct value<int>
{
  typedef int target;
  typedef serializerT< value<int> > serializer;
};

template<>
struct value<unsigned int>
{
  typedef unsigned int target;
  typedef serializerT< value<unsigned int> > serializer;
};

template<>
struct value<long>
{
  typedef long target;
  typedef serializerT< value<long> > serializer;
};

template<>
struct value<unsigned long>
{
  typedef unsigned long target;
  typedef serializerT< value<unsigned long> > serializer;
};

template<>
struct value<long long>
{
  typedef long long target;
  typedef serializerT< value<long long> > serializer;
};

template<>
struct value<unsigned long long>
{
  typedef unsigned long long target;
  typedef serializerT< value<unsigned long long> > serializer;
};


template<>
struct value<bool>
{
  typedef bool target;
  typedef serializerT< value<bool> > serializer;
};


template<>
struct value<std::string>
{
  typedef std::string target;
  typedef serializerT< value<std::string> > serializer;
};

template<int N>
struct value< char[N] >
{
  typedef char target[N];
  typedef serializerT< value<char[N]>  > serializer;
};


template<typename T>
struct raw_value
{
  typedef T target;
  typedef serializerT< raw_value< T > > serializer;
};

/*
template<>
struct raw_value<std::string>
{
  typedef std::string target;
  typedef serializerT< raw_value<std::string> > serializer;
};

template<typename T>
struct raw_value< std::vector<T> >
{
  typedef std::string target;
  typedef serializerT< raw_value< std::vector<T> > > serializer;
};
**/

/*
template<typename T>
struct if_def
{
  typedef typename T::target target;
  typedef serializerT< if_def<T> > serializer;
};*/

/// /////////////////////////////////////////
template<typename K, typename V>
struct pair
{
  typedef typename K::target key_target;
  typedef typename V::target value_target;
  typedef std::pair<key_target, value_target> target;
  typedef serializerT< pair<K, V> > serializer;
  typedef typename K::serializer key_serializer;
  typedef typename V::serializer value_serializer;
};

/// //////////////////////////////////////////////////////////////////////////////

template<typename N, typename T, T v>
struct enum_value: N
{
  static const T value = v;
  const char* operator()() const { return N::operator()(); }
};

template< typename T, typename L>
struct enumerator
{
  typedef T target;
  typedef typename ap::type_list_traits<L>::type enum_list;
  typedef serializerT< enumerator<T, enum_list> > serializer;
};

/// //////////////////////////////////////////////////////////////////////////////

template<typename T, typename L>
struct object
{
  typedef T target;
  typedef serializerT< object<T, L> > serializer;
  typedef typename ap::type_list_traits<L>::type member_list;
};

///
template<typename T, typename V, typename M, M V::* m, typename W >
struct member_value
{
  typedef T target;
  /*
  typedef M type;
  typedef W wrapper;
  typedef typename W::target wrapper_target;
  */

  typedef serializerT< member_value<T, V, M, m, W> > serializer;

  /*
  wrapper_target* ptr(T* t) { return static_cast<wrapper_target*>(t->*m);};
  const wrapper_target* ptr(const T* t) const { return static_cast< const wrapper_target*>(t->*m);};
  wrapper_target& ref(T& t) { return static_cast<wrapper_target&>(t.*m); };
  const wrapper_target& ref(const T& t) const { return static_cast< const wrapper_target&>(t.*m);};
  */
};

/// //////////////////////////////////////////////////////////////////////////////

template<typename N, typename T, typename M, M T::* m, typename W >
struct member: N
{
  typedef T target;
  typedef M type;
  typedef W wrapper;
  typedef typename W::serializer serializer;
  typedef typename W::target wrapper_target;
  const char* operator()() const { return N::operator()(); }

  wrapper_target* ptr(T* t) { return static_cast<wrapper_target*>(t->*m);};
  const wrapper_target* ptr(const T* t) const { return static_cast< const wrapper_target*>(t->*m);};
  wrapper_target& ref(T& t) { return static_cast<wrapper_target&>(t.*m); };
  const wrapper_target& ref(const T& t) const { return static_cast< const wrapper_target&>(t.*m);};
};


template<typename T, typename M, M T::* m>
struct property
{
  void operator()(T& t, const M& value ) const
  {
    t.*m = value;
  }

  const M& operator()(const T& t) const
  {
    return t.*m;
  }
};

template<typename N,
         typename T,
         typename M,
         typename G, // getter
         typename W
        >
struct member_p: N
{
private:
  G _g;
public:
  typedef M type;
  typedef typename W::serializer serializer;
  typedef typename W::target wrapper_target;
  const char* operator()() const { return N::operator()(); }
  void set(T& t, const wrapper_target& v) const { _g(t, v); }
  wrapper_target get(const T& t) const { return _g(t); }
};


template<typename L, typename R, bool RU >
struct member_if
{
};


/// //////////////////////////////////////////////////////////////////////////////


/// //////////////////////////////////////////////////////////////////////////////

template< typename T, char L, char R >
class serializerA;


template<typename C>
struct array_base;

template<typename J>
struct array_base< std::vector<J> >
{
  typedef std::vector<J> json_container;
  typedef J json_value;
  typedef typename json_value::target target;
  typedef std::vector<target> target_container;
  typedef serializerT< array< json_container > > serializer;
  typedef std::back_insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::back_inserter(t); }
};

template<typename J, int N>
struct array_base< J[N] >
{
  typedef J json_container[N];
  typedef J json_value;
  typedef typename json_value::target target;
  typedef target target_container[N];
  typedef serializerT< array< json_container > > serializer;
};

template<typename J>
struct array_base< std::set<J> >
{
  typedef std::set<J> json_container;
  typedef J json_value;
  typedef typename json_value::target target;
  typedef std::set<target> target_container;
  typedef serializerT< array< json_container > > serializer;

  typedef std::insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::inserter(t, t.begin()); }
};

template<typename J>
struct array_base< std::multiset<J> >
{
  typedef std::multiset<J> json_container;
  typedef J json_value;
  typedef typename json_value::target target;
  typedef std::multiset<target> target_container;
  typedef serializerT< array< json_container > > serializer;
  typedef std::insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::inserter(t, t.begin()); }
};


#ifdef __GXX_EXPERIMENTAL_CXX0X__
template<typename JK, typename JV>
struct array_base< std::unordered_map<JK, JV> >
{
  typedef std::unordered_map<JK, JV> json_container;
  typedef typename JK::target key;
  typedef typename JV::target value;
  typedef std::pair<key, value> pair_type;
  typedef pair< JK , JV > json_value;
  typedef typename json_value::target target;
  typedef std::unordered_map< key, value > target_container;
  typedef serializerA< array< json_container >, '{', '}' > serializer;

  typedef std::insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::inserter(t, t.begin()); }
};
#endif


template<typename JK, typename JV>
struct array_base< std::map<JK, JV> >
{
  typedef std::map<JK, JV> json_container;
  typedef typename JK::target key;
  typedef typename JV::target value;
  typedef std::pair<key, value> pair_type;
  typedef pair< JK , JV > json_value;
  typedef typename json_value::target target;
  typedef std::map< key, value > target_container;
  typedef serializerA< array< json_container >, '{', '}' > serializer;

  typedef std::insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::inserter(t, t.begin()); }
};


struct n_key { const char* operator()() const{ return "key";} };
struct n_value { const char* operator()() const{ return "value";} };

template<typename JK, typename JV>
struct array_base< std::multimap<JK, JV> >
{
  typedef std::multimap<JK, JV> json_container;
  typedef typename JK::target key;
  typedef typename JV::target value;
  typedef std::pair<key, value> pair_type;
  typedef object<
            pair_type,
            typename ap::type_list_n<
              member< n_key, pair_type, key, &pair_type::first, JK >,
              member< n_value, pair_type, value, &pair_type::second, JV >
            >::type
          > json_value;
  typedef typename json_value::target target;
  typedef std::map< key, value > target_container;

  typedef serializerT< array< json_container > > serializer;

  typedef std::insert_iterator<target_container> inserter_iterator;
  static inserter_iterator inserter(target_container& t) { return std::inserter(t, t.begin()); }
};

template<typename C>
struct array: public array_base<C>
{
  typedef array_base<C> base;
  typedef typename base::target_container target;
  typedef /*serializerT< array<C> >*/ typename base::serializer serializer;
};

template< typename T, typename L>
struct array< object<T, L> >
  : array< std::vector< object<T, L> > >
{
};

/// //////////////////////////////////////////////////////////////////////////////

}}

#include "json/json_except.hpp"
#include "json/json_parser.hpp"
#include "json/json_number.hpp"
#include "json/json_string.hpp"
#include "json/json_object.hpp"
#include "json/json_array.hpp"
#include "json/json_enum.hpp"

#endif
