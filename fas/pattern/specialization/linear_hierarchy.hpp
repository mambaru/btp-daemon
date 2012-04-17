//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_LINEAR_HIERARCHY_SPECIALIZATION_HPP
#define FAS_PATTERNS_LINEAR_HIERARCHY_SPECIALIZATION_HPP


namespace fas{ namespace pattern{


template<class TL>
class linear_hierarchy
  : public linear_hierarchy< typename type_list_traits<TL>::type >
{
public:
};

template<>
class linear_hierarchy<empty_type>
{
};



}}

#endif
