/*
 * queries.hpp
 *
 *  Created on: Jun 9, 2011
 *      Author: shepik
 */

#ifndef QUERIES_HPP_
#define QUERIES_HPP_

#include <fas/system/system.hpp>
#include <fas/adv/json_rpc3/method.hpp>
#include "common/aspect/rn_json_aspect.hpp"
#include "common/aspect/json_aspect.hpp"
#include "common/ap_type_list_v.hpp"
#include "common/json_double.hpp"

namespace aj = ::fas::json;
namespace ap = ::fas::pattern;

#define NAME(X) struct n_##X { const char* operator()() const{ return #X;} }
NAME(srv);
NAME(farm);
NAME(ts);
NAME(time);
NAME(replace);
NAME(script);
NAME(source_srv);
NAME(server);
NAME(service);
NAME(op);
NAME(items);
NAME(data);
NAME(scale);
NAME(limit);
NAME(field);
NAME(sort_by);

NAME(avg);
NAME(count);
NAME(perc50);
NAME(perc80);
NAME(perc95);
NAME(perc99);

#include "counters.hpp"

namespace btprequest {

	NAME(s);
	NAME(u);
	NAME(a);

	//массив счётчиков
	typedef std::unordered_map<std::string,	//service =>
		std::unordered_map<std::string,	//server =>
			std::unordered_map<std::string,	//op =>
				std::vector<long long int>
			>
		>
	> counters_data;

	//сериализация массива счётчиков
	typedef aj::array<std::unordered_map<aj::value<std::string>,	//service =>
		aj::array<std::unordered_map<aj::value<std::string>,	//service =>
			aj::array<std::unordered_map<aj::value<std::string>,	//op =>
				aj::array<std::vector<aj::value<long long int> > >
			> >
		> >
	> > counters_data_json;

	//структура со счётчиками, которая приходит с фронтов
	struct script_data {
		std::string srv;
		std::string script;
		double time;
		counters_data items;
		bool replace;

		script_data() {
			replace = false;
		}
	};

	//сериализация структуры со счётчиками, которая приходит с фронтов
	typedef aj::object<
		script_data,
		ap::type_list_v<
			aj::member<n_srv, script_data, std::string, &script_data::srv>
			,aj::member<n_script, script_data, std::string, &script_data::script>
			,aj::member<n_time , script_data, double, &script_data::time>
			,aj::member<n_items, script_data, counters_data, &script_data::items, counters_data_json >
			,aj::member<n_replace, script_data, bool, &script_data::replace>
		>::type
	> script_data_json;


	//сериализация enum'а
	typedef ap::type_list_n<
		aj::enum_value<n_count, aggregated_counter_field, aggregated_counter_field::count>
		,aj::enum_value<n_avg, aggregated_counter_field, aggregated_counter_field::avg>
		,aj::enum_value<n_perc50, aggregated_counter_field, aggregated_counter_field::perc50>
		,aj::enum_value<n_perc80, aggregated_counter_field, aggregated_counter_field::perc80>
		,aj::enum_value<n_perc95, aggregated_counter_field, aggregated_counter_field::perc95>
		,aj::enum_value<n_perc99, aggregated_counter_field, aggregated_counter_field::perc99>
	> aggregated_counter_field_list;


	//запрос на получение графика/списков
	struct get_graph {
		std::string script;
		std::string service;
		std::string server;
		std::string op;

		int scale;	//для get_graph
		unsigned int limit;	//в get_list - сложные частичные выборки - лимит.
		//bool sort_total;//в get_list - сортировка по полному времени
		aggregated_counter_field field; //multigraph - имя параметра, который мы хотим узнать
	};
	typedef aj::object<
		get_graph,
		ap::type_list_v<
			aj::member<n_server, get_graph, std::string, &get_graph::server>
			,aj::member<n_script, get_graph, std::string, &get_graph::script>
			,aj::member<n_service, get_graph, std::string, &get_graph::service>
			,aj::member<n_op, get_graph, std::string, &get_graph::op>

			,aj::member<n_scale, get_graph, int, &get_graph::scale>
			,aj::member<n_limit, get_graph, unsigned int, &get_graph::limit>
			//,aj::member<n_sort_total, get_graph, int, &get_graph::limit>
			,aj::member<n_field, get_graph, aggregated_counter_field, &get_graph::field, aj::enumerator<aggregated_counter_field, aggregated_counter_field_list> >
		>::type
	> get_graph_json;


	//запрос на получение списков с сортировкой и фильтром
	struct get_list_advanced {
		std::string script;
		std::string service;
		std::string op;
		std::string server;

		int scale;
		unsigned int limit;
		std::string sort_by;
	};
	typedef aj::object<
		get_list_advanced,
		ap::type_list_v<
			aj::member<n_script, get_list_advanced, std::string, &get_list_advanced::script>
			,aj::member<n_service, get_list_advanced, std::string, &get_list_advanced::service>
			,aj::member<n_op, get_list_advanced, std::string, &get_list_advanced::op>
			,aj::member<n_server, get_list_advanced, std::string, &get_list_advanced::server>

			,aj::member<n_scale, get_list_advanced, int, &get_list_advanced::scale>
			,aj::member<n_limit, get_list_advanced, unsigned int, &get_list_advanced::limit>
			,aj::member<n_sort_by, get_list_advanced, std::string, &get_list_advanced::sort_by>
		>::type
	> get_list_advanced_json;


}

namespace btpresponse {

	struct get_graph {
		std::vector<aggregated_counter> data;
		int scale;
		int ts;
	};
	typedef aj::object<
		get_graph,
		ap::type_list_v<
			aj::member<n_data, get_graph, std::vector<aggregated_counter>, &get_graph::data, aj::array<std::vector<aggregated_counter_json> > >
			,aj::member<n_scale, get_graph, int, &get_graph::scale>
			,aj::member<n_ts, get_graph, int, &get_graph::ts>
		>::type
	> get_graph_json;

	struct get_multigraph {
		std::map<std::string, std::vector<long long int>> data;
		int scale;
		int ts;
	};
	typedef aj::object<
		get_multigraph,
		ap::type_list_v<
			aj::member<n_data, get_multigraph, decltype(get_multigraph::data), &get_multigraph::data, aj::array< std::map< aj::value<std::string>, aj::array<std::vector< aj::value<long long int> > > > > >
			,aj::member<n_scale, get_multigraph, int, &get_multigraph::scale>
			,aj::member<n_ts, get_multigraph, int, &get_multigraph::ts>
		>::type
	> get_multigraph_json;




}

#endif /* QUERIES_HPP_ */
