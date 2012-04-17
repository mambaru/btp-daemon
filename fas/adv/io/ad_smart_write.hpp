//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_FILTERS_AD_SMART_WRITE_HPP
#define FAS_FILTERS_AD_SMART_WRITE_HPP

#include <fas/adv/io/ad_binary_write.hpp>

#include <vector>
#include <queue>
#include <stdexcept>

namespace fas { namespace adv { namespace io{


/// BUG: в самой идее - убрать

/// используется при непостоянной связи с получателем
/// Пришедший блок передается полностю, если не удалось, то при востановлении
/// передает его заного
/// Требуется ручной очистки буфера при новом сеансе!!! clear_front
template<typename W, typename N, typename C, typename E, size_t S = 0>
class ad_smart_write:
  ad_buf_binary_write< W, N, C, E, S >
{

  bool _ready_smart;
public:

  typedef ad_buf_binary_write< W, N, C, E> super;

  typedef typename super::write_advice_tag write_advice_tag;
  typedef std::vector<char> buffer_type;
  typedef std::pair< size_t, buffer_type > buffer_pair;
  typedef std::queue< buffer_pair > buffer_queue;

  ad_smart_write(): _ready_smart(false) {};
  /** Очищает внутренний буффер */
  void clear()
  {
    super::clear();
    if ( !_buffer_queue.empty() )
    {

      _ready_smart = true;
      const buffer_pair& pb = _buffer_queue.front();
      const_cast<size_t&>(pb.first) = pb.second.size();
    }
  }

  bool ready() const 
  { 

     return super::ready() || _ready_smart /*|| !_buffer_queue.empty();*/;
  }

  /*
  void clear_front() const
  {
    if ( !_buffer_queue.empty() )
    {
      const buffer_pair& pb = _buffer_queue.front();
      const_cast<size_t&>(pb.first) = pb.second.size();
    }
  }
  */

   template<typename T>
   void confirm( T& t, bool write_next )
   {

     if ( _buffer_queue.empty() )
       throw std::logic_error("ad_smart_write: confirm");
     else
     {
       const buffer_pair& pb = _buffer_queue.front();
       if (pb.first!=0 && pb.first!=pb.second.size() )
       {
         // std::cout<<"warning: ошибка счетчиков"<<std::endl;
       }
     }
     _buffer_queue.pop();

     if ( write_next )
     {
       _ready_smart = false;
       _write_queue(t);
     }
   }

  template<typename T>
  typename T::write_return_type operator()(T& t, const char* d, typename T::write_size_type s)
  {
    typedef typename T::write_return_type return_type;
    typedef typename T::write_size_type size_type;

    return_type rt = -1;
    if ( !_buffer_queue.empty() || super::ready() )
    {
      if (s > 0)
        _buffer_queue.push( std::make_pair(s, buffer_type(d, d+s) ) );
    }
    else
    {
      rt = super::operator ( )(t, d, s);
      if (/*rt!=s &&*/ s > 0 )
      {
        _buffer_queue.push( std::make_pair( rt != -1 ? s - rt : s, buffer_type(d, d + s) ) );
      }
    }
    return rt;
  }

  template<typename T>
  typename T::write_return_type operator()(T& t)
  {
    typedef typename T::write_return_type return_type;
    typedef typename T::write_size_type size_type;

    if ( super::ready() )
    {
      return_type rt = super::operator ( )(t);
      if (rt != -1)
      {
         if ( !_buffer_queue.empty() )
         {
           // Если зашли сюда, значит super::operator ( )(t); был записан кусок из первого элемента очереди 
           const buffer_pair& pb = _buffer_queue.front();
           if ( rt == 0 )
             const_cast<size_t&>(pb.first) = pb.second.size();
           else if ( rt > 0 )
             const_cast<size_t&>(pb.first) -= rt;

           //if ( pb.first == 0)
           //  _buffer_queue.pop();
         }
      }
    }
    else if ( _ready_smart )
    {
      _ready_smart = false;
      return _write_queue(t);
    }
    return -1;
  }

private:

  template<typename T>
  typename T::write_return_type _write_queue(T& t)
  {
    typedef typename T::write_return_type return_type;

    return_type rt = -1;
    if ( _buffer_queue.empty() )
      return rt;
    if ( !super::ready() )
    {
      buffer_pair& pb = _buffer_queue.front();
      if ( pb.first != pb.second.size() )
         throw std::logic_error("ad_smart_binary_write: error counter buffer 2");
      rt = super::operator ( )(t, &(pb.second[0]), pb.second.size() );
      if (rt > 0)
      {
        if ( rt > static_cast<return_type>(pb.first) )
          throw std::logic_error("ad_smart_binary_write: error counter buffer");
        pb.first -= rt;
        // if ( pb.first == 0)
        //  _buffer_queue.pop();
      }
    }
    return rt;
  }

  buffer_queue _buffer_queue;


};


}}}

#endif
