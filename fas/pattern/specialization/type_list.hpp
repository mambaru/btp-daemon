//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_PATTERNS_TYPE_LIST_SPECIALIZATION_HPP
#define FAS_PATTERNS_TYPE_LIST_SPECIALIZATION_HPP

namespace fas{ namespace pattern {

template<typename T>
struct type_count<T, empty_type>
{
  enum { result = 0};
};

/*
template<typename T, typename L, typename R>
struct type_count<T, type_list<L, R> >
{
  enum 
  {
    result = detail::type_count_helper<T, type_list<L, R>,
                                       super_sub_class<T, L>::result >::result
  };
};
*/




/// /////////////////////////////////////////////////////////////////////////

/*
template<typename T, typename L, typename R>
struct type_position<T, type_list<L, R> >
{
  enum { result = detail::type_position_helper<T, R, super_sub_class<T, L>::result >::result  } ;
};
*/

template<>
struct type_position<empty_type, empty_type >
{
  enum { result = 0 } ;
};



/// /////////////////////////////////////////////////////////////////////////

template<typename T>
struct erase<T, empty_type >
{
  typedef empty_type type;
};

template<typename L>
struct erase<empty_type, L>
{
  typedef L type;
};


/// /////////////////////////////////////////////////////////////////////////


template<typename T>
struct erase_all<T, empty_type >
{
  typedef empty_type type;
  typedef empty_type removed;
};

template<typename L>
struct erase_all<empty_type, L>
{
  typedef L type;
  typedef empty_type removed;
};


/// /////////////////////////////////////////////////////////////////////////

template< typename S, int C>
struct erase_from_list<empty_type, S, C>
{
  typedef empty_type type;
};

template< typename F, int C>
struct erase_from_list<F, empty_type, C>
{
  typedef F type;
};

template< int C>
struct erase_from_list<empty_type, empty_type, C>
{
  typedef empty_type type;
};


/// /////////////////////////////////////////////////////////////////////////



/*

template<typename L>
struct merge<L, empty_type>
{
  typedef type_list<L, empty_type > type;
};

template<>
struct merge<empty_type, empty_type>
{
  typedef empty_type type;
};


template<typename L>
struct merge<empty_type, L>
{
  typedef type_list<L, empty_type > type;
};

template<typename L, typename R, typename RR>
struct merge<type_list<L, R>, RR>
{
  typedef typename merge<L, type_list<R, RR> >::type type;
};

template<typename L, typename R, typename RR>
struct merge<RR, type_list<L, R> >
{
  typedef type_list<RR, typename merge<L, R>::type > type;
};

template<typename L, typename R>
struct merge<type_list<L, R>, empty_type>
{
  typedef typename merge<L, R>::type type;
};

template<typename L, typename R>
struct merge<empty_type, type_list<L, R> >
{
  typedef typename merge<L, R>::type type;
};

template<typename L, typename R, typename L1, typename R1>
struct merge<type_list<L, R>, type_list<L1, R1> >
{
  typedef typename merge<L,
            typename merge<R,
              typename merge<L1, R1>::type
            >::type
          >::type type;
};


template<typename L,  typename L1, typename R1>
struct merge< type_list<L, empty_type>, type_list<L1, R1> >
{
  typedef typename merge<L,
              typename merge<L1, R1>::type
            >::type
          type;
};
*/


/// /////////////////////////////////////////////////////////////////////////


template<>
struct organize< empty_type >
{
  typedef empty_type type;
};

/*
template<typename L, typename R>
struct organize< type_list<L, R> >
{
  typedef type_list<L, typename organize<R>::type > type;
};

template<typename R>
struct organize< type_list< empty_type, R> >
{
  typedef typename organize<R>::type type;
};

template<typename L, typename R, typename RR>
struct organize< type_list< type_list<L, R>, RR> >
{
  typedef typename organize< typename merge< type_list<L, R>, RR >::type >::type type;
};
*/


/*
template<typename L, typename R>
struct unique< type_list<L, R> >
{
  typedef typename detail::unique_helper<
            type_list<L, R>,
            type_count<L, type_list<L, R> >::result - 1
          >::type type;
};
*/

/*
template<>
struct unique< empty_type >
{
  typedef empty_type type;
};
*/
template<>
struct unique< empty_type >
{
  typedef empty_type type;
};





template<typename L1>
struct unique_from_list<L1, empty_type>
{
  typedef L1 type;
};

template<typename T, typename L1, typename R1>
struct unique_from_list< T, type_list<L1, R1> >
{
  typedef typename detail::unique_from_list_helper<
            L1,
            T,
            type_count<L1, T>::result >= 2
          >::type L1_unique;

  typedef typename unique_from_list< L1_unique, R1>::type type;
};



/// /////////////////////////////////////////////////////////////////////////

/*
template<typename L, typename R>
struct type_list_cast< L, type_list<L, R> >
{
  typedef type_list<L, R> type;
};
*/
template<typename T>
struct type_list_cast< T, empty_type >
{
  typedef empty_type type;
};

/*
template<typename T, typename L, typename R>
struct type_list_cast<T, type_list<L, R> >
{
  typedef typename detail::type_list_cast_helper<T, L, type_list<L, R>, super_sub_class<T, L>::result >::type type;
};
*/

/// //////////////////////////////////////////


}}

#endif
