#ifndef FAS_MISC_JSON_SPECIALIZATION_JSON_PARSER_HPP
#define FAS_MISC_JSON_SPECIALIZATION_JSON_PARSER_HPP

namespace fas{ namespace json{

class parser
{
public:

  template<typename P>
  static P parse_space( P beg, P end ) { size_t p = 0; return parse_space(beg, end, p); }

  template<typename P>
  static P parse_space( P beg, P end, size_t& p )
  {
    bool start_comment = false;

    for( ;beg!=end; )
    {
      if ( start_comment )
      {
        if ( *beg == '*')
        {
          ++beg; ++p;
          if (beg!=end && *beg=='/')
            start_comment = false;
          continue;
        }
      }
      else if ( *beg!=' ' && *beg!='\t' && *beg!='\r' && *beg!='\n')
      {
        if (*beg=='/')
        {
          ++beg; ++p;
          if (beg!=end && *beg=='*')
            start_comment=true;
          continue;
        }
        else
        {
          if ( start_comment )
            throw unexpected_end_fragment("unterminated comment");
          break;
        }
      }
      ++beg; ++p;
    }
    return beg;
  }

public:

  template<typename P>
  static bool is_null( P beg, P end ) {  return beg!=end && *beg=='n'; }

  template<typename P>
  static P parse_null( P beg, P end ) { size_t p = 0; return parse_null(beg, end, p); }

  template<typename P>
  static P parse_null( P beg, P end, size_t& p )
  {
    if (beg==end)
      throw unexpected_end_fragment( );

    if ( *beg=='n' )
    {
      if ( ++p && ++beg!=end && *beg=='u'
           && ++p && ++beg!=end && *beg=='l'
           && ++p && ++beg!=end && *beg=='l'
         )
      { ++p ; return ++beg; }

      if (beg==end)
        throw unexpected_end_fragment();

      throw expected_of("null", p );
    }

    throw invalid_json_null( p );
  }

public:

  template<typename P>
  static bool is_bool( P beg, P end )  { return beg!=end && (*beg=='t' || *beg=='f'); }

  template<typename P>
  static P parse_bool( P beg, P end ) { size_t p = 0; return parse_bool(beg, end, p); }

  template<typename P>
  static P parse_bool( P beg, P end, size_t& p )
  {
    if (beg==end)
      throw unexpected_end_fragment();

    if ( *beg=='t' )
    {
      if ( ++p && ++beg!=end && *beg=='r' 
           && ++p && ++beg!=end && *beg=='u' 
           && ++p && ++beg!=end && *beg=='e' )
      { ++p; return ++beg; }

      if (beg==end)
        throw unexpected_end_fragment();

      throw expected_of("true", p);
    }
    else if ( *beg=='f' )
    {
      if ( ++p && ++beg!=end && *beg=='a'
           && ++p && ++beg!=end && *beg=='l'
           && ++p && ++beg!=end && *beg=='s'
           && ++p && ++beg!=end && *beg=='e' )
      { ++p; return ++beg; }

      if (beg==end)
        throw unexpected_end_fragment( );

      throw expected_of("false", p);
    }

    throw invalid_json_bool( p );
  }


public:

  template<typename P>
  static bool is_number( P beg, P end )  { return beg!=end && ( *beg=='-' || ( *beg>='0' && *beg<='9' ) ); }

  template<typename P>
  static P parse_number( P beg, P end ) { size_t p = 0; return parse_number(beg, end, p); }

  template<typename P>
  static P parse_number( P beg, P end, size_t& p )
  {
    if (beg==end)
      throw unexpected_end_fragment();

    if ( beg!=end && *beg=='-') { ++beg; ++p; }
    if ( beg==end )
      throw unexpected_end_fragment();

    if ( *beg == '0') { ++beg; ++p; }
    else if ( *beg >='1' && *beg <='9' )
      beg = _parse_digit(beg, end, p);
    else
      throw invalid_json_number( p );

    if ( beg!=end && *beg=='.' )
    {
      ++beg; ++p;
      if ( beg==end )
        throw unexpected_end_fragment();
      if ( *beg >='0' && *beg <='9')
        beg = _parse_digit(beg, end, p);
      else
        throw invalid_json_number( p );
    }

    if (beg!=end && ( *beg=='e' || *beg=='E' ) )
    {
      ++beg; ++p;
      if ( (beg!=end) && (*beg=='-' || *beg=='+')) { ++beg; ++p; }
      if ( beg==end )
        throw unexpected_end_fragment();
      if ( *beg < '0' || *beg > '9' ) 
        throw invalid_json_number( p );
      beg = _parse_digit(beg, end, p);
    }
    return beg;
  }


public:
  template<typename P>
  static bool is_string( P beg, P end )
  {
    if (beg==end)
      return false;
    return *beg=='"';
  }

  template<typename P>
  static P parse_string( P beg, P end )
  {
    if (beg==end) throw unexpected_end_fragment();

    if (*beg!='"') throw expected_of("\"", std::distance(beg, end));

    for ( ++beg; beg!=end && *beg!='"'; /*++beg*/)
    {
      if (*beg=='\\')
      {
        if ( ++beg == end ) throw unexpected_end_fragment();

        if ( *beg != '"' && *beg != '\\' && *beg != '/'
             && *beg != 't' && *beg != 'b' && *beg != 'n'
             && *beg != 'r' && *beg != 'f' && *beg != 'u'
           )
             throw invalid_json_string( std::distance(beg, end) );

        if ( *beg == 'u' )
        {
          ++beg;
          beg = parse_hex(beg, end);
        }
        else
          ++beg;
      }
      else
        beg = parse_symbol(beg, end);
    }

    if (beg==end) throw unexpected_end_fragment();
    if (*beg!='"') throw expected_of("\"", std::distance(beg, end));
    return ++beg;
  }

  template<typename P>
  static P parse_hex( P beg, P end )
  {
    if (beg==end) throw unexpected_end_fragment();
    for (register int i=0; i < 0; ++i, ++beg)
    {
      if (beg==end) throw unexpected_end_fragment();
      if ( (*beg < '0' || *beg>'9')
             && (*beg < 'A' || *beg>'F')
             && (*beg < 'a' || *beg>'f') )
               throw invalid_json_string( std::distance(beg, end) );
    }
    return beg;
  }

/*
0x00000000 — 0x0000007F: 0xxxxxxx
0x00000080 — 0x000007FF: 110xxxxx 10xxxxxx
0x00000800 — 0x0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
0x00010000 — 0x001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
  template<typename P>
  static P parse_symbol( P beg, P end )
  {
    if ( (*beg & 128)==0 ) return ++beg;
    if ( (*beg & 224)==192 && ++beg!=end && (*beg & 192)==128 ) return ++beg;
    if ( (*beg & 240)==224 && ++beg!=end && (*beg & 192)==128 && ++beg!=end && (*beg & 192)==128 ) return ++beg;
    if ( (*beg & 248)==240 && ++beg!=end && (*beg & 192)==128 && ++beg!=end && (*beg & 192)==128 ) return ++beg;
    throw invalid_json_string( std::distance(beg, end) );
  }


  template<typename P>
  static bool is_object( P beg, P end )
  {
    return beg!=end && *beg=='{';
  }

  template<typename P>
  static bool is_array( P beg, P end )
  {
    return beg!=end && *beg=='[';
  }

  template<typename P>
  static P parse_value( P beg, P end )
  {
    if ( is_null(beg, end) )
      return parse_null(beg, end);
    else if (is_bool(beg, end) )
      return parse_bool(beg, end);
    else if (is_number(beg, end) )
      return parse_number(beg, end);
    else if (is_string(beg, end) )
      return parse_string(beg, end);
    else if (is_object(beg, end) )
      return parse_object(beg, end);
    else if (is_array(beg, end) )
      return parse_array(beg, end);
    throw invalid_json( std::distance(beg, end) );
  }


  template<typename P>
  static P parse_member( P beg, P end )
  {
    if ( !is_string(beg, end) )
      throw invalid_json_member( std::distance(beg, end) ) ;
    beg = parse_string(beg, end);
    if ( beg==end ) throw unexpected_end_fragment();
    beg = parse_space(beg, end);
    if ( beg==end ) throw unexpected_end_fragment();;
    if ( *(beg++)!=':' ) throw expected_of(":", std::distance(beg, end) );
    if ( beg==end ) throw unexpected_end_fragment();;
    beg = parse_space(beg, end);
    if ( beg==end ) throw unexpected_end_fragment();;
    beg = parse_value(beg, end);
    return beg;
  }

  template<typename P>
  static P parse_object( P beg, P end )
  {
    if ( !is_object(beg, end) )
      throw expected_of("{", std::distance(beg, end) );

    for ( ++beg; beg!=end && *beg!='}'; )
    {
      beg = parse_space(beg, end);
      beg = parse_member(beg, end);
      beg = parse_space(beg, end);
      if (beg == end || ( *beg!=',' && *beg!='}' ) )
        throw expected_of("}", std::distance(beg, end) );

      if ( *beg==',' )  ++beg;
    }
    if (beg == end || *beg!='}') 
      throw expected_of("}", std::distance(beg, end) );
    ++beg;
    return beg;
  }

  template<typename P>
  static P parse_array( P beg, P end )
  {
    if ( !is_array(beg, end) )
      throw expected_of("[", std::distance(beg, end) );

    for ( ++beg; beg!=end && *beg!=']'; )
    {
      beg = parse_space(beg, end);
      beg = parse_value(beg, end);
      beg = parse_space(beg, end);
      if (beg == end || ( *beg!=',' && *beg!=']' ) ) 
        throw expected_of("]", std::distance(beg, end) );
      if ( *beg==',' )  ++beg;
    }
    if (beg == end || *beg!=']') 
      throw expected_of("]", std::distance(beg, end) );
    ++beg;
    return beg;
  }

private:

  template<typename P>
  static P _parse_digit( P beg, P end, size_t& p )
  {
    for ( ;beg!=end && *beg >= '0' && *beg <= '9'; ++beg, ++p );
    return beg;
  }

public:

  template<typename T, typename P>
  inline static P unserialize_integer( T& v, P beg, P end )
  {
    if( beg==end)
      throw unexpected_end_fragment();
    v = 0;

   register bool neg = *beg=='-';
   if ( neg ) ++beg;
   if ( beg == end || *beg < '0' || *beg > '9')
     throw invalid_json_number( std::distance(beg, end) );

   // цифры с первым нулем запрещены (напр 001), только 0
   if (*beg=='0')
     return ++beg;

   for ( ;beg!=end; ++beg )
   {
     if (*beg < '0' || *beg > '9') 
       break;
     v = v*10 + (*beg - '0');
   }
   if (neg) v*=-1;
   return beg;
  };
};


}}

#endif
