#ifndef FAS_ADV_JSON_RPC3_AD_INVOKE_HPP
#define FAS_ADV_JSON_RPC3_AD_INVOKE_HPP

#include <set>
#include <fas/pattern/type_list.hpp>
#include <fas/adv/json_rpc3/tags.hpp>
#include <fas/adv/json_rpc3/json_rpc3_types.hpp>
#include <fas/adv/ad_clock.hpp>
#include <fas/misc/json.hpp>

namespace fas{ namespace adv{ namespace json_rpc3{

namespace ap = ::fas::pattern;
namespace ad = ::fas::adv;


class ad_invoke
{
  typedef std::multiset<int> ids_set;
  ids_set _in_ids;
  int _id_count;

public:

  ad_invoke(): _id_count(0){}

  template<typename T>
  void clear(T&)
  {
    _id_count = 0;
    _in_ids.clear();
  }

  template<typename T>
  void operator() (T& t, const char* d, size_t s)
  {
    if ( d == 0 || s == 0 ) 
      return;

    
    const char *beg = d;
    const char *end = d + s;
    t.get_aspect().template get<_begin_invoke_>()(t, d, s );

// #warning искуственное ограничение на слишком большие пакеты запросов, чтобы не тормозить сервак
// TODO: сделать аттрибутом
    for ( int i=0 ; i < 10 ; ++i )
    {
      const char *next = _invoke_1( t, beg, std::distance(beg, end) );
      
      if (next == 0 || next == beg )
        break;
      if (next != 0 ) 
        for (;next!=end && *next <= 32; ++next);
      if (next == end )
        break;

      beg = next;

    }
    
    t.get_aspect().template get<_end_invoke_>()(t);
    
  }

  template<typename T>
  const char* _invoke_1 (T& t, const char* d, size_t s)
  {
    const char* next = 0;

    const char *method = 0;
    const char *id = 0;
    const char *result = 0;

    _search_(d, d + s, &method, &result, &id);

    if ( method==0 && result==0) 
    {
      
      t.get_aspect().template get<_invalid_json_>()(t, d, s ); 
      
      return next; 
    }
    if ( result!=0 && id==0) 
    {
      t.get_aspect().template get<_invalid_json_>()(t, d, s ); 
      return next;
    }

    int rid = -1;
    if (id!=0)
      rid = std::atoi(id);

    typedef typename T::aspect::advice_list advice_list;
    typedef typename ap::select<_gmethod_, advice_list>::type invoke_list;

    if ( method!=0 && id!=0 && result==0)
    {
      ids_set::iterator itr = _in_ids.lower_bound(rid);
      _in_ids.insert(itr, rid);
      next = _invoke(t, invoke_list(), method, d, s, -1);  // request
    }
    else if ( method!=0 && id==0 && result==0)
      next = _invoke(t, invoke_list(), method, d, s, -2);  // notify
    else if (result!=0 && id!=0 && method==0)
      next = _invoke(t, invoke_list(), result, d, s, rid); // response
    else
    {
      t.get_aspect().template get<_invalid_json_>()(t, d, s );
      return next;
    }

    return next;
  }


  int create_id() { return ++_id_count; }

  template<typename T>
  void request(T& t, int id, const char* d, size_t s)
  {
    t.get_aspect().template get<_writer_>()(t, d, s );
  }

  template<typename T>
  void response(T& t, int id, const char* d, size_t s)
  {

    ids_set::iterator itr = _in_ids.lower_bound(id);
    //if ( itr==_in_ids.end() || *itr != id )
    //   throw invalid_id();
    if ( itr != _in_ids.end() && (*itr) == id )
      _in_ids.erase(itr);
    t.get_aspect().template get<_writer_>()(t, d, s );
  }

  template<typename T>
  void notify(T& t, const char* d, size_t s)
  {
    t.get_aspect().template get<_writer_>()(t, d, s );
  }

  template<typename T>
  void error(T& t, int id, const char* d, size_t s)
  {
    
    ids_set::iterator itr = _in_ids.lower_bound(id);
    
    if ( itr != _in_ids.end() && (*itr) == id )
    {
    
      _in_ids.erase(itr);
    }
    
    t.get_aspect().template get<_writer_>()(t, d, s );
    
  }

private:

  // search section
  const char *_search_tovalue_( const char *beg, const char *end)
  {
    for (;beg!=end && *beg <= 32; ++beg);
    if ( *beg!=':') 
      beg = end; 

    if (beg!=end)
      ++beg;
    for (;beg!=end && *beg <= 32; ++beg);

    if ( beg!=end && *beg=='"')
      beg++;

    if ( beg==end ) 
      return 0;
      //throw fas::json::invalid_json();

    return beg;
  }

  const char* _search_helper_( const char *beg, const char *end, const char **method, const char **result, const char **id)
  {
//    const char *start = beg;
    static const char s_method[] = "\"method\"";
    static const char s_result[] = "\"result\"";
    static const char s_id[] = "\"id\"";
    *method = 0;
    *result = 0;
    *id = 0;

    try
    {
    beg = ::fas::json::parser::parse_space( beg, end);
    if ( beg==end || *beg!='{') 
    {
      return 0;
    }
    ++beg;
    beg = ::fas::json::parser::parse_space( beg, end);
    while ( beg!=end )
    {
      int i = 0;
      for ( ; (beg + i)!=end && *(beg+i)==s_method[i]; ++i);
      if ( i == sizeof(s_method)-1 )
        *method = _search_tovalue_(beg+i, end);
      for ( i = 0; (beg + i)!=end && *(beg+i)==s_result[i]; ++i);
      if ( i == sizeof(s_result)-1 )
        *result = _search_tovalue_(beg+i, end);

      for ( i = 0; (beg + i)!=end && *(beg+i)==s_id[i]; ++i);
      if ( i == sizeof(s_id)-1 )
        *id = _search_tovalue_(beg+i, end);
      beg = ::fas::json::parser::parse_member( beg, end);
      beg = ::fas::json::parser::parse_space( beg, end);
      if ( beg==end ) return 0;
      if (*beg=='}') break;
      if (*beg==',') ++beg;
      else
      {
        return 0;
      }
      beg = ::fas::json::parser::parse_space( beg, end);
    }
    if ( beg==end || *beg!='}') 
    {
      return 0;
    }
    ++beg;
    beg = ::fas::json::parser::parse_space( beg, end);
    return beg;
    }
    catch(const ::fas::json::json_error& e)
    {
      
      return 0;
    }
    catch(...)
    {
      return 0;
    }



    bool slash = false;
    int brace_count = 0;

    int method_count = *method!=0 ? 8 : 0;
    int result_count = *result!=0 ? 8 : 0;
    int id_count = *id!=0 ? 4 : 0;

    for ( ; beg!=end && (method_count!=8 || result_count!=8 || id_count!=4); ++beg)
    {
      if ( !slash && *beg=='\\' ) { slash = true; continue;}
      if ( slash ) { slash = false; continue;}
      if ( *beg == '{') { ++brace_count; continue; }
      if ( *beg == '}') { --brace_count; continue; }
      if ( brace_count!=1) continue;

      if ( method_count != 8)
      {
        if ( *beg == s_method[method_count] ) method_count++;
        else method_count = 0;

        if ( method_count == 8)
          *method = _search_tovalue_(beg+1, end);
      }

      if ( result_count != 8)
      {
        if ( *beg == s_result[result_count] ) result_count++;
        else result_count = 0;

        if ( result_count == 8)
          *result = _search_tovalue_(beg+1, end);
      }

      if ( id_count != 4)
      {
        if ( *beg == s_id[id_count] ) id_count++;
        else id_count = 0;

        if ( id_count == 4)
          *id = _search_tovalue_(beg+1, end);
      }
    }
    return beg;
  }

  void _search_( const char *beg, const char *end, const char **method, const char **result, const char **id)
  {
    beg = _search_helper_(beg, end, method, result, id);
  }

  static bool _raw_equal( const char* name_nz, const char* raw_name)
  {
    for ( ; *name_nz!='\0' && *raw_name!='"'; ++name_nz, ++raw_name )
      if (*name_nz!=*raw_name)
        return false;
    return *name_nz=='\0' && *raw_name=='"';
  }

  // invoke section
  template<typename L, typename T>
  const char* _invoke(T& t, L, const char* name, const char* d, size_t s, int code)
  {
    const char* next = 0;
    typedef typename T::aspect::advice_list advice_list;
    typedef typename ap::select<_gmethod_, advice_list>::type invoke_list;
    typedef typename L::left_type current_type;
    enum { method_position = ap::type_position<current_type, invoke_list >::result };

    current_type& current = t.get_aspect().template get<typename L::left_type>();
    bool flag = false;
    if ( code > 0 || _raw_equal( current.name(), name ) )
    {
      flag = true;
      if (code==-1)
      {
        t.get_aspect().template get<ad::_clock_start_>()(t);
        next = current.invoke_request(t, d, s);
        t.get_aspect().template get<ad::_span_reg_>().mark( method_position );
        t.get_aspect().template get<ad::_clock_finish_>()(t);
      }
      else if (code==-2)
      {
        t.get_aspect().template get<ad::_clock_start_>()(t);
        next = current.invoke_notify(t, d, s);
        t.get_aspect().template get<ad::_span_reg_>().mark( method_position );
        t.get_aspect().template get<ad::_clock_finish_>()(t);
      }
      else
      {
        if ( current.has_id(code) )
        {
          next = current.invoke_response(t, d, s);
          // t.get_aspect().template get<ad::_span_reg_>().mark( method_position );
        }
        else
          flag = false;
      }
    }

    if (!flag)
      return _invoke(t, typename L::right_type(), name, d, s, code);

    return next;

  }

  template< typename T>
  const char* _invoke(T& t, ap::empty_type, const char* name, const char* d, size_t s, int code)
  {
    if ( code < 0 )
      t.get_aspect().template get<_unknown_method_>()(t, d, s);
    else 
      t.get_aspect().template get<_lost_result_>()(t, d, s);
    return 0;
  }
};

}}}

#endif
