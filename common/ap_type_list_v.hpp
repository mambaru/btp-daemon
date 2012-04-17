#pragma once

namespace fas { namespace pattern {


	/**
	 * список типов переменной длины
	 */
	template<typename ...List>
	struct type_list_v;

	template<typename Head, typename ...Tail>
	struct type_list_v<Head, Tail...> {
		//typedef Head left_type;
		//typedef type_list_v<Tail...> right_type;
		//typedef type_list_v<Head, Tail...> self;
		typedef typename detail::type_list_n_helper<Head>::type head_type;
		typedef type_list<head_type, typename type_list_v<Tail...>::type > type;
	};
	template<>
	struct type_list_v<> {
		//typedef empty_type left_type;
		//typedef empty_type right_type;
		typedef ap::empty_type type;
	};


}}
