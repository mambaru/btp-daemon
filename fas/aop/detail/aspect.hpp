//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_ASPECT_DETAIL_HPP
#define FAS_AOP_ASPECT_DETAIL_HPP


namespace fas{ namespace aop { namespace detail {

template<typename L>
struct extract_aspect_tags_helper
{
  typedef typename L::left_type advice_type;

  typedef ap::type_list<
    typename extract_advice_tags< advice_type >::tag_list,
    typename extract_aspect_tags_helper< typename L::rigth_type>::tag_list
  > tag_list;

  typedef ap::type_list<
    typename extract_advice_tags< advice_type >::gtag_list,
    typename extract_aspect_tags_helper< typename L::rigth_type>::gtag_list
  > gtag_list;

  typedef ap::type_list<
    typename extract_advice_tags< advice_type >::all_tag_list,
    typename extract_aspect_tags_helper< typename L::rigth_type>::all_tag_list
  > all_tag_list;
};

template<>
struct extract_aspect_tags_helper< ap::empty_type >
{
  typedef ap::empty_type tag_list;
  typedef ap::empty_type gtag_list;
  typedef ap::empty_type all_tag_list;
};

}}}

#endif
