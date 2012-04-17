//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_AOP_ASPECT_SPECIALIZATION_HPP
#define FAS_AOP_ASPECT_SPECIALIZATION_HPP


namespace fas{ namespace aop {

template<typename L>
struct aspect_merge<L, void>
{
public:
  typedef L type;
};

template<>
struct aspect_merge<void, void>
{
public:
  typedef aspect<> type;
};

template<typename A>
class aspect_class<A, void>
  : protected A
{
public:
  typedef A aspect;
  typedef typename aspect::advice_list advice_list;
  aspect& get_aspect() { return *this;}
  const aspect& get_aspect() const { return *this;}

  template<typename AA = A, typename BB = aspect >
  struct rebind
  {
    typedef aspect_class< aspect, typename aspect_merge<AA, BB>::type > type;
  };
};


}}

#endif
