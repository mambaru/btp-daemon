#ifndef FAS_ADV_JSON_RPC3_OUTGOING_DATA_HPP
#define FAS_ADV_JSON_RPC3_OUTGOING_DATA_HPP

#include <vector>
namespace fas{ namespace adv{ namespace json_rpc3{

class outgoing_data
{
public:
  typedef std::vector<char> data_type;

  template<typename T>
  void clear(T&)
  {
    if ( _data.capacity() > 2048 )
      data_type().swap(_data);
    _data.reserve(1024);
  }

  data_type& operator()() { return _data;}
  const data_type& operator()() const { return _data;}
private:
  data_type _data;
};

}}}

#endif
