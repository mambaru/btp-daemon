#ifndef FAS_MISC_JSON_SPECIALIZATION_JSON_OBJECT_HPP
#define FAS_MISC_JSON_SPECIALIZATION_JSON_OBJECT_HPP

//#include "json_parser.hpp"

// #define USIND_STDLIB

namespace fas{ namespace json{

/// ////////////////////////////////////////////////////////////

template<typename T, typename V, typename M, M V::* m, typename W >
struct serializerT< member_value<T, V, M, m, W> >
{
  template<typename P>
  P operator()( const T& t, P end)
  {
    return typename W::serializer()( static_cast<const V&>(t).*m, end);
  }

  template<typename P>
  P operator()( T& t, P beg, P end)
  {
    return typename W::serializer()( static_cast<V&>(t).*m, beg, end);
  }
};


/// ////////////////////////////////////////////////////////////

template<typename T, typename L>
class serializerT< object<T, L> >
{
  typedef object<T, L> object_type;

public:

  serializerT(){}

  /** serialization:
   * end - back inserter iterator
   */
  template<typename P>
  P operator()( const T& t, P end)
  {
    *(end++)='{';
    end = serialize_members(t, end, L() );
    *(end++)='}';
    return end;
  }

  /** deserialization:
    * beg, end - forward iterator
    */
  template<typename P>
  P operator()( T& t, P beg, P end)
  {

    if ( parser::is_null(beg, end) )
    {
      t = T();
      return parser::parse_null(beg, end);
    }

    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *(beg++) != '{' ) 
      throw expected_of("{", std::distance(beg, end) );

    beg = parser::parse_space(beg, end);
    if ( beg==end ) 
      throw unexpected_end_fragment();

    if ( *beg != '}')
    {
       beg = unserialize_members(t, beg, end, L() );
       if ( beg==end ) throw unexpected_end_fragment();
       beg = parser::parse_space(beg, end);
       if ( beg==end ) throw unexpected_end_fragment();
    }

    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *(beg++) != '}' ) 
      throw expected_of("}", std::distance(beg, end));

    return beg;
  }

private:

  template<typename P, typename C, typename R>
  P serialize_members( const T& t, P end, ap::type_list<C, R> )
  {
    end = serialize_member(t, end, C());
    *(end++)=',';
    return serialize_members(t, end, R() );
  }

  template<typename P, typename C>
  P serialize_members( const T& t, P end, ap::type_list<C, ap::empty_type> )
  {
    return serialize_member(t, end, C());
  }

  template<typename P>
  P serialize_members( const T& t, P end, ap::empty_type )
  {
    return end;
  }

  template<typename P, typename M >
  P serialize_member_name( const T& t, P end, M memb )
  {
    const char* name = memb();
    *(end++)='"';
    for (;*name!='\0'; ++name) *(end++) = *name;
    *(end++)='"';
    *(end++) = ':';
    return end;
  }

  template<typename P, typename N, typename G, typename M, M G::* m, typename W >
  P serialize_member( const T& t, P end, const member<N, G, M, m, W>& memb )
  {
    end = serialize_member_name(t, end, memb);

    typedef typename member<N, G, M, m, W>::serializer serializer;
    return serializer()( memb.ref(t), end );
  }

  template<typename P, typename N, typename G, typename M, typename GT, typename W >
  P serialize_member( const T& t, P end, const member_p<N, G, M, GT, W>& memb )
  {
    end = serialize_member_name(t, end, memb);
    typedef typename member_p<N, G, M, GT, W>::serializer serializer;
    return serializer()( memb.get(t), end );
  }


  template<typename P, typename ML, typename MR, bool RU >
  P serialize_member( const T& t, P end, const member_if<ML, MR, RU>& memb )
  {
    typedef typename ML::type typeL;
    if ( !( _get_value(t, ML()) == typeL() ) )
      return serialize_member( t, end, ML() );
    return serialize_member( t, end, MR() );
  }

private:

  template<typename N, typename G, typename M, M G::* m, typename W >
  M _get_value( const T& t, member<N, G, M, m, W> memb )
  {
    return memb.ref(t);
  }

  template<typename N, typename G, typename M, typename GT, typename W >
  M _get_value( const T& t, member_p<N, G, M, GT, W> memb )
  {
    return memb.get(t);
  }

  template<typename P, typename C, typename R>
  P unserialize_members( T& t, P beg, P end, ap::type_list<C, R>, bool search = false )
  {
    bool unserialized = false;
    beg = unserialize_member( t, beg, end, C(), unserialized );

    if (!unserialized)
    {
      if ( !search ) // Организуем поиск с начала списка
        beg = unserialize_members( t, beg, end, L(), true );
      else // Продолжаем поиск
        return unserialize_members( t, beg, end, R(), true );
    }
    else if (search)
      return beg;

    beg = parser::parse_space(beg, end);

    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *beg == ',' )
    {
      ++beg;
      beg = parser::parse_space(beg, end);

      if ( unserialized )
        beg = unserialize_members( t, beg, end, R(), false );
      else
        beg = unserialize_members( t, beg, end, ap::type_list<C, R>() , false );
    }

    if (beg==end) 
      throw unexpected_end_fragment();

    if ( *beg != '}' )
      throw expected_of("}", std::distance(beg, end));

    return beg;
  }


  template<typename P>
  P unserialize_members( T& t, P beg, P end, ap::empty_type, bool search = false )
  {
    if ( !search )
    {
      beg = parser::parse_space(beg, end);
      if ( beg==end ) 
        throw unexpected_end_fragment();

      if ( *beg=='}' ) return beg;
      for(;;)
      {
        beg = parser::parse_member(beg, end);
        beg = parser::parse_space(beg, end);
        if ( beg==end ) throw unexpected_end_fragment();
        if ( *beg=='}' ) return beg;
        if ( *beg!=',' ) throw expected_of(",", std::distance(beg, end));
        ++beg;
        beg = parser::parse_space(beg, end);
      }
    }
    else
    {
      // если организован поиск и не нашли то пропускаем

      beg = parser::parse_member(beg, end);
      beg = parser::parse_space(beg, end);
      return beg;
    }
  }

  template<typename P, typename M >
  P unserialize_member_name( T& t, P beg, P end, M memb, bool& unserialized )
  {
    const char* name = memb();
    P start = beg;
    if ( !parser::is_string(beg, end) )
      throw expected_of("\"", std::distance(beg, end));
    ++beg;
    unserialized = true;
    for ( ; beg!=end && *name!='\0' && *beg==*name && *beg!='"'; ++name, ++beg)
    {
      if (*name!=*beg) 
      {
        unserialized = false;
        break;
      }
    }

    if (beg==end) 
      throw unexpected_end_fragment();

    if (*beg!='"' || *name!='\0') 
      unserialized = false;

    if ( !unserialized ) return start;
    ++beg;
    beg = parser::parse_space(beg, end);
    if (beg==end) 
       throw unexpected_end_fragment();

    if (*beg!=':') 
      throw expected_of(":", std::distance(beg, end));
    ++beg;
    beg = parser::parse_space(beg, end);

    if (beg==end) 
       throw unexpected_end_fragment();

    return beg;
  }


  template<typename P, typename N, typename G, typename M, M G::* m, typename W >
  P unserialize_member( T& t, P beg, P end, member<N, G, M, m, W> memb, bool& unserialized )
  {
    beg = unserialize_member_name(t, beg, end, memb, unserialized);
    if ( !unserialized )
      return beg;
    typedef typename member<N, G, M, m, W>::serializer serializer;
    return serializer()( memb.ref(t), beg, end);
  }

  template<typename P, typename N, typename G, typename M, typename GT, typename W >
  P unserialize_member( T& t, P beg, P end, member_p<N, G, M, GT, W> memb, bool& unserialized )
  {
    beg = unserialize_member_name(t, beg, end, memb, unserialized);
    if ( !unserialized )
      return beg;
    typedef typename member_p<N, G, M, GT, W>::serializer serializer;
    M value = M();
    beg = serializer()( value, beg, end);
    memb.set(t, value);
    return beg;
  }

  template<typename P, typename ML, typename MR >
  P unserialize_member( T& t, P beg, P end, member_if<ML, MR, true>, bool& unserialized )
  {
    return unserialize_member(t, beg, end, MR(), unserialized);
  }

  template<typename P, typename ML, typename MR >
  P unserialize_member( T& t, P beg, P end, member_if<ML, MR, false>, bool& unserialized )
  {
    return unserialize_member(t, beg, end, ML(), unserialized);
  }

};

}}

#endif
