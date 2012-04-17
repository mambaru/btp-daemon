#ifndef FAS_MISC_JSON_SPECIALIZATION_JSON_ENUM_HPP
#define FAS_MISC_JSON_SPECIALIZATION_JSON_ENUM_HPP

//#include "json_parser.hpp"

namespace fas{ namespace json{

/// /////////////////////////////////////////////////////////////////

template<typename T, typename L>
class serializerT< enumerator<T, L> >
{
  typedef typename enumerator<T, L>::enum_list enum_list;
public:
  template<typename P>
  P operator()( const T& v, P end)
  {
    *(end++)='"';
    end = this->serialize(v, enum_list(), end);
    *(end++)='"';
    return end;
  };

  template<typename LL, typename RR, typename P>
  P serialize( const T& v, ap::type_list<LL, RR>, P end)
  {
    if (LL::value == v)
    {
      const char* val = LL()();
      for ( ; *val!='\0' ; ++val)
        *(end++) = *val;
      return end;
    }
    else
    {
      return this->serialize(v, RR(), end);
    }
  }

  template<typename P>
  P serialize( const T& v, ap::empty_type, P end)
  {
    return end;
  }

  template<typename P>
  P operator() ( T& v, P beg, P end )
  {
    v = T();
    for ( ; beg!=end && *beg<=' '; ++beg);
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!='"') throw expected_of("\"");
    ++beg;
    if (beg==end) throw unexpected_end_fragment();
    P first = beg;
    for ( ; beg!=end && *beg!='"'; ++beg);
    if (beg==end) throw unexpected_end_fragment();
    if (*beg!='"') throw expected_of("\"");
    this->deserialize(v, enum_list(), first, beg);
    ++beg;
    return beg;
  }

  template<typename LL, typename RR, typename P>
  void deserialize( T& v, ap::type_list<LL, RR>, P beg, P end)
  {
    P first = beg;
    const char *pstr = LL()();
    for ( ; beg!=end && *pstr!='\0' && *pstr==*beg; ++beg, ++pstr);
    if ( beg==end && *pstr=='\0')
    {
      v = LL::value;
    }
    else
      deserialize(v, RR(), first, end);
  }

  template<typename P>
  void deserialize( T& , ap::empty_type,P,P)
  {
  }

};

}}

#endif
