#ifndef FAS_MISC_JSON_SPECIALIZATION_JSON_ARRAY_HPP
#define FAS_MISC_JSON_SPECIALIZATION_JSON_ARRAY_HPP

//#include "json_parser.hpp"

namespace fas{ namespace json{

template<typename K, typename V>
class serializerT< pair<K, V> >
{
public:
  typedef pair<K, V> pair_type;
  typedef typename pair_type::target target;
  typedef typename pair_type::key_serializer key_serializer;
  typedef typename pair_type::value_serializer value_serializer;

  template<typename P>
  P operator()( const target& t, P end)
  {
    end = key_serializer()(t.first, end );
    *(end++)=':';
    end = value_serializer()(t.second, end );
    return end;
  }


  template<typename P>
  P operator()( target& t,  P beg, P end)
  {
    beg = key_serializer()(t.first, beg, end );
    beg = parser::parse_space(beg, end);
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=':') throw expected_of(":", std::distance(beg, end) );
    ++beg;
    beg = parser::parse_space(beg, end);
    if (beg==end) throw unexpected_end_fragment();
    beg = value_serializer()(t.second, beg, end );
    return beg;
  }
};

/// ////////////////////////////////////////////////////////////////
/*
template< typename T >
class serializerT< array<T> >
{
  typedef array<T> array_type;
  typedef typename array_type::target_container target_container;
  typedef typename array_type::json_value json_value;
  typedef typename json_value::serializer serializer;
  typedef typename json_value::target target;


public:
  template<typename P>
  P operator()( target_container& t,  P beg, P end)
  {
    if ( parser::is_null(beg, end) )
    {
      t = target_container();
      return parser::parse_null(beg, end);
    }

    // std::back_insert_iterator<C> bitr =  std::back_inserter(t);
    // std::insert_iterator<C> bitr = std::inserter(t, t.end());
    typename array_type::inserter_iterator bitr = array_type::inserter(t);

    if (beg==end) throw unexpected_end_fragment();
    if (*beg!='[') throw expected_of("[", std::distance(beg, end) );
    ++beg;
    for (;beg!=end;)
    {
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==']') break;
      target tg;
      beg = serializer()( tg, beg, end);
      *(bitr++) = tg;
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==']') break;
      if (*beg!=',') throw expected_of(",", std::distance(beg, end));
      ++beg;
    }
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=']') throw expected_of("]", std::distance(beg, end));
    ++beg;
    return beg;
  }

  template<typename P>
  P operator()( const target_container& t, P end)
  {
    *(end++)='[';
    typename target_container::const_iterator itr = t.begin();
    for (;itr!=t.end();)
    {
      end = serializer()(*itr, end);
      ++itr;
      if (itr!=t.end()) *(end++)=',';
    }
    *(end++)=']';
    return end;
  }
};
*/

template< typename T, char L, char R >
class serializerA
{
  typedef T array_type;
  typedef typename array_type::target_container target_container;
  typedef typename array_type::json_value json_value;
  typedef typename json_value::serializer serializer;
  typedef typename json_value::target target;


public:
  template<typename P>
  P operator()( target_container& t,  P beg, P end)
  {
    if ( parser::is_null(beg, end) )
    {
      t = target_container();
      return parser::parse_null(beg, end);
    }

    // std::back_insert_iterator<C> bitr =  std::back_inserter(t);
    // std::insert_iterator<C> bitr = std::inserter(t, t.end());
    typename array_type::inserter_iterator bitr = array_type::inserter(t);

    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=L) throw expected_of( std::string(1, L), std::distance(beg, end) );
    ++beg;
    for (;beg!=end;)
    {
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==R) break;
      target tg;
      beg = serializer()( tg, beg, end);
      *(bitr++) = tg;
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==R) break;
      if (*beg!=',') throw expected_of(",", std::distance(beg, end));
      ++beg;
    }
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=R) throw expected_of(std::string(1, R), std::distance(beg, end));
    ++beg;
    return beg;
  }

  template<typename P>
  P operator()( const target_container& t, P end)
  {
    *(end++)=L;
    typename target_container::const_iterator itr = t.begin();
    for (;itr!=t.end();)
    {
      end = serializer()(*itr, end);
      ++itr;
      if (itr!=t.end()) *(end++)=',';
    }
    *(end++)=R;
    return end;
  }
};

template< typename T >
class serializerT< array<T> >
  : public serializerA< array<T>, '[', ']'>
{
};


template< typename J, int N, char L, char R >
class serializerA< array< J[N] >, L, R >
{
  typedef array< J[N] > array_type;
  typedef typename array_type::target_container target_container;
  typedef typename array_type::json_value json_value;
  typedef typename json_value::serializer serializer;
  typedef typename json_value::target target;


public:
  template<typename P>
  P operator()( target_container& t,  P beg, P end)
  {
    if ( parser::is_null(beg, end) )
    {
      for (int i = 0; i < N ; ++i)
        t[i] = target();
      return parser::parse_null(beg, end);
    }

//    typename array_type::inserter_iterator bitr = array_type::inserter(t);
    target* bitr = t;
    target* eitr = bitr + N;

    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=L) throw expected_of( std::string(1, L), std::distance(beg, end) );
    ++beg;
    for (;beg!=end && bitr!=eitr;)
    {
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==R) break;
      target tg;
      beg = serializer()( tg, beg, end);
      *(bitr++) = tg;
      beg = parser::parse_space(beg, end);
      if (beg==end) throw unexpected_end_fragment();
      if (*beg==R) break;
      if (*beg!=',') throw expected_of(",", std::distance(beg, end));
      ++beg;
    }
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!=R) throw expected_of(std::string(1, R), std::distance(beg, end));
    ++beg;
    return beg;
  }

  template<typename P>
  P operator()( const target_container& t, P end)
  {
    *(end++)=L;
    const target* itr = t;
    const target* iend = itr + N;
    for (;itr!=iend;)
    {
      end = serializer()(*itr, end);
      ++itr;
      if (itr!=iend) *(end++)=',';
    }
    *(end++)=R;
    return end;
  }
};


}}

#endif
