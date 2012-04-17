#ifndef FAS_MISC_FAST_JSON2_JSON_STRING_HPP
#define FAS_MISC_FAST_JSON2_JSON_STRING_HPP

namespace fas{ namespace json{

template<typename T>
class serializerS;

template<>
class serializerS<char>
{
public:
  template<typename P1,typename P>
  P serialize(P1 beg, P1 end, P itr)
  {
    *(itr++)='"';
    for (;beg!=end && *beg!='\0';++beg)
    {
      switch (*beg)
      {
        case '"' :
        case '\\':
        case '/' :  *(itr++)='\\'; *(itr++) = *beg; break;
        case '\t':  *(itr++)='\\'; *(itr++) = 't'; break;
        case '\b':  *(itr++)='\\'; *(itr++) = 'b'; break;
        case '\r':  *(itr++)='\\'; *(itr++) = 'r'; break;
        case '\n':  *(itr++)='\\'; *(itr++) = 'n'; break;
        case '\f':  *(itr++)='\\'; *(itr++) = 'f'; break;
        default: *(itr++) = *beg; break;
      }
    }
    *(itr++)='"';
    return itr;
  }

  template<typename P, typename P1>
  P unserialize(P beg, P end, P1 vitr, int n = -1)
  {
    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *(beg++) != '"' ) 
      throw expected_of("\"");

    for ( ;beg!=end && *beg!='"' && n!=0; )
    {
      if (*beg=='\\')
      {
        if (++beg==end) 
          throw unexpected_end_fragment();
        switch (*beg)
        {
          case '"' :
          case '\\':
          case '/' : *(vitr++) = *beg; ++beg; --n; break;
          case 't':  *(vitr++) = '\t'; ++beg; --n; break;
          case 'b':  *(vitr++) = '\b'; ++beg; --n; break;
          case 'r':  *(vitr++) = '\r'; ++beg; --n; break;
          case 'n':  *(vitr++) = '\n'; ++beg; --n; break;
          case 'f':  *(vitr++) = '\f'; ++beg; --n; break;
          case 'u':  beg = _unserialize_uhex(++beg, end, &vitr, n); break;
          default:
            throw invalid_json_string(std::distance(beg, end) );
        }
      }
      else
        beg = _unserialize_symbol(beg, end, &vitr, n);
    };

    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *(beg++) != '"' ) 
      throw expected_of("\"");

    return beg;
    // return ++beg;
  }
private:
  template<typename P, typename P1>
  P _unserialize_symbol(P beg, P end, P1* vitr, int n)
  {
    
    if (beg == end) throw unexpected_end_fragment();

    if ( (*beg & 128)==0 )
    {
      *((*vitr)++) = *(beg++);
      --n;
    }
    else if ( (*beg & 224)==192 )
      beg = _symbol_copy<2>(beg, end, vitr, n);
    else if ( (*beg & 240)==224 )
      beg = _symbol_copy<3>(beg, end, vitr, n);
    else if ( (*beg & 248)==240 )
      beg = _symbol_copy<4>(beg, end, vitr, n);
    else
      throw invalid_json_string(std::distance(beg, end) );
    return beg;
  }

  template<int N, typename P, typename P1>
  P _symbol_copy(P beg, P end, P1* vitr, int n)
  {
    for (register int i = 0; i < N && n!=0; ++i, --n)
    {
      if (beg == end) 
        throw unexpected_end_fragment();
      *((*vitr)++) = *(beg++);
    }
    return beg;
  }

/*
0x00000000 — 0x0000007F 	0xxxxxxx
0x00000080 — 0x000007FF 	110xxxxx 10xxxxxx
0x00000800 — 0x0000FFFF 	1110xxxx 10xxxxxx 10xxxxxx

U+0439	й	d0 b9	CYRILLIC SMALL LETTER SHORT I
00000100 00111001 -> 110_10000 10_111001

// উ - U+0989 - 	e0 a6 89
// 00001001 10001001 -> 11100000 10100110 10001001
*/
  template<typename P, typename P1>
  P _unserialize_uhex(P beg, P end, P1* vitr, int n)
  {
    unsigned short hex = 0;
    if (beg == end ) throw unexpected_end_fragment();
    hex |= _toUShort(*(beg++)) << 12;
    if (beg == end ) throw unexpected_end_fragment();
    hex |= _toUShort(*(beg++)) << 8;
    if (beg == end ) throw unexpected_end_fragment();
    hex |= _toUShort(*(beg++)) << 4;
    if (beg == end ) throw unexpected_end_fragment();
    hex |= _toUShort(*(beg++));

    if ( hex <= 0x007F )
    {
      *((*vitr)++) = static_cast<unsigned char>(hex);
      --n;
    }
    else if ( hex <= 0x007FF )
    {
       *((*vitr)++) = 192 | static_cast<unsigned char>( hex >> 6 );
       --n;
       if ( n==0) throw invalid_json_string(std::distance(beg, end) );
       *((*vitr)++) = 128 | ( static_cast<unsigned char>( hex ) & 63 );
       --n;
    }
    else
    {
       *((*vitr)++) = 224 | static_cast<unsigned char>( hex >> 12 );
       --n;
       if ( n==0) throw invalid_json_string(std::distance(beg, end) );
       *((*vitr)++) = 128 | ( static_cast<unsigned char>( hex >> 6 ) & 63 );
       --n;
       if ( n==0) throw invalid_json_string(std::distance(beg, end) );
       *((*vitr)++) = 128 | ( static_cast<unsigned char>( hex ) & 63 );
       --n;
    }

    return beg;

  }

  unsigned short _toUShort(unsigned char c)
  {
    if ( c >= '0' && c<='9' ) return c - '0';
    if ( c >= 'a' && c<='f' ) return (c - 'a') + 10;
    if ( c >= 'A' && c<='F' ) return (c - 'A') + 10;
    throw invalid_json_string();
  }

};


template<int N>
class serializerT< value< char[N]> >
  : serializerS<char>
{
public:

  typedef char value_type[N];

  template<typename P>
  P operator()( const value_type& v, P end)
  {
    return serialize( v, v+N, end);
  };

  template<typename P>
  P operator() ( value_type& v, P beg, P end )
  {
    for ( register int i =0 ; i < N; ++i) v[i]=0;
    if ( parser::is_null(beg, end) )
      return parser::parse_null(beg, end);

    //P s_end = parser::parse_string(beg, end);
    return unserialize(beg, end, &(v[0]), N);
  }
};


template<>
class serializerT< value<std::string> >
  : serializerS<char>
{
public:
  template<typename P>
  P operator()( const std::string& v, P end)
  {
    return serialize( v.begin(), v.end(), end);
  }

  template<typename P>
  P operator() ( std::string& v, P beg, P end )
  {
    v.clear();
    if ( parser::is_null(beg, end) )
      return parser::parse_null(beg, end);
    return unserialize(beg, end, std::back_inserter(v));
  }
};

template<typename T>
class serializerT< raw_value<T/*std::string*/> >
{
public:
  template<typename P>
  P operator()( const /*std::string*/T& v, P end)
  {
    if ( v.begin() != v.end() )
      return std::copy(v.begin(), v.end(), end );
    else
    {
      *(end++)='"';
      *(end++)='"';
      return end;
    }
  }

  template<typename P>
  P operator() ( /*std::string*/T& v, P beg, P end )
  {
    v.clear();
    P start = beg;
    beg = parser::parse_value(beg, end);
    std::copy( start, beg, std::back_inserter(v) );
    return beg;
  }
};

}}


#endif

