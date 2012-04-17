#ifndef FAS_MISC_JSON_SPECIALIZATION_JSON_EXCEPT_HPP
#define FAS_MISC_JSON_SPECIALIZATION_JSON_EXCEPT_HPP



namespace fas{ namespace json{

class json_error: public std::runtime_error
{
  std::ptrdiff_t _tail_of;
public:
  json_error(const std::string& msg, size_t tail_of = 0 ): std::runtime_error(msg), _tail_of(tail_of) {}
  size_t tail_of() const { return _tail_of; }

  template<typename P>
  std::string message( P beg, P end ) const
  {
    if (std::distance(beg, end) < _tail_of )
      return this->what();
    std::string msg; 
    msg = this->what();
    msg += std::string(": ") + std::string(beg, end - _tail_of ) + ">>>" + std::string(end - _tail_of, end);
    return msg;
  };
};

class serialize_error: public json_error
{ 
public:
  serialize_error(size_t tail_of = 0)
    : json_error("serialize error", tail_of) {}
};

class deserialize_error: public json_error
{ 
public:
  deserialize_error(size_t tail_of = 0)
    : json_error("deserialize error", tail_of)  {}
};

class not_supported: public json_error
{ 
public:
  not_supported(size_t tail_of = 0)
    : json_error("not supported", tail_of)  {}
};

class invalid_json: public json_error
{ 
public:
  invalid_json(size_t tail_of = 0)
    : json_error("invalid json", tail_of)  {}
};

class invalid_json_null: public json_error
{ 
public:
  invalid_json_null(size_t tail_of = 0)
    : json_error("invalid json null", tail_of)  {}
};


class invalid_json_number: public json_error
{ 
public:
  invalid_json_number(size_t tail_of = 0)
    : json_error("invalid json number", tail_of)  {}
};

class invalid_json_bool: public json_error
{ 
public:
  invalid_json_bool(size_t tail_of = 0)
    : json_error("invalid json bool", tail_of)  {}
};

class invalid_json_string: public json_error
{ 
public:
  invalid_json_string(size_t tail_of = 0)
    : json_error("invalid json string", tail_of)  {}
};

class invalid_json_member: public json_error
{ 
public:
  invalid_json_member(size_t tail_of = 0)
    : json_error("invalid json member", tail_of)  {}
};

class invalid_json_object: public json_error
{ 
public:
  invalid_json_object(size_t tail_of = 0)
    : json_error("invalid json object", tail_of)  {}
};

class invalid_json_array: public json_error
{ 
public:
  invalid_json_array(size_t tail_of = 0)
    : json_error("invalid json array", tail_of)  {}
};

class invalid_conversion
  : public json_error
{
public:
  invalid_conversion(size_t tail_of = 0)
    : json_error( "invalid conversion", tail_of) {}
  invalid_conversion( const std::string& from, const std::string& to, size_t tail_of = 0 )
    : json_error( std::string("invalid conversion from '") + from + std::string("' to '") + to, tail_of ) {}
};

class unexpected_end_fragment
  : public json_error
{
public:
  unexpected_end_fragment(size_t tail_of = 0)
    : json_error( "unexpected end of ragment", tail_of) {}
  unexpected_end_fragment(const std::string& str, size_t tail_of = 0)
   : json_error( std::string("unexpected end of ragment: ") + str, tail_of ) {}
};

class expected_of
  : public json_error
{
public:
  expected_of(const std::string& str, size_t tail_of = 0)
    : json_error( std::string("expected of '") + str + std::string("'"), tail_of) {}
};


}}

#endif
