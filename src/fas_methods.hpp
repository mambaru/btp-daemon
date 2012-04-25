#pragma once

#include <fas/adv/ad_mutex.hpp>
#include <fas/filter/basic_filter.hpp>
namespace af = ::fas::filter;
namespace ad = ::fas::adv;

#include "common/fas_statistics.hpp"

namespace btpmethods {

// удаление сервиса или скрипта
struct delete_op {
	const char *name() const { return "delete"; }
	typedef btprequest::get_graph_json invoke_request;
	typedef aj::value<bool> invoke_response;

	template<typename T> bool request(T& t, const btprequest::get_graph& cmd,int id, bool &result) {
		result = false;
		if (cmd.op.length() && cmd.service.length()) {
			int val1 = data->d_service.get(cmd.service,false);
			if (val1<=0) return true;
			int val2 = data->d_op.get(cmd.op,false);
			if (val2<=0) return true;
			data->c_service_op.remove12(val1,val2);
			data->stat_service_server_op.remove(intkey<3>{{val1,0,val2}});
		} else if (cmd.server.length() && cmd.service.length()) {
			int val1 = data->d_service.get(cmd.service,false);
			if (val1<=0) return true;
			int val2 = data->d_server.get(cmd.server,false);
			if (val2<=0) return true;
			data->c_service_server.remove12(val1,val2);
			data->stat_service_server_op.remove(intkey<3>{{val1,val2,0}});
		} else if (cmd.service.length()) {
			int val = data->d_service.remove(cmd.service);
			if (val == -1) return true;
			data->c_script_service.remove2(val);
			data->c_service_op.remove1(val);
			data->c_service_server.remove1(val);
			data->stat_service_server_op.remove(intkey<3>{{val,0,0}});
			data->stat_script_service_op.remove(intkey<3>{{0,val,0}});
		} else if (cmd.script.length()) {
			int val = data->d_script.remove(cmd.script);
			if (val == -1) return true;
			data->c_script_service.remove1(val);
			data->stat_script_service_op.remove(intkey<3>{{val,0,0}});
		} else return false;
		result = true;
		return true;
	}
};
struct delete_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<delete_op>
> {};

// добавление статистики
struct put {
	const char *name() const { return "put"; }
	typedef btprequest::script_data_json invoke_request;
	typedef btprequest::script_data_json invoke_notify;
	typedef aj::value<bool> invoke_response;

	template<typename T> bool request(T& t, const btprequest::script_data& cmd,int id, bool &result) {
		notify(t,cmd);
		result = true;
		return true;
	}

	template<typename T> void notify(T& t, const btprequest::script_data& cmd) {
		auto curr1 = data->stat_service_server_op.get_current();
		auto curr2 = data->stat_script_service_op.get_current();

		int n_script = data->d_script.get(cmd.script);
		for (auto it_service = cmd.items.begin(),end_service = cmd.items.end();it_service!=end_service;++it_service) {
			if (it_service->first.empty()) continue;
			int n_service = data->d_service.get(it_service->first);
			for (auto it_server = it_service->second.begin(),end_server = it_service->second.end();it_server!=end_server;++it_server) {
				if (it_server->first.empty()) continue;
				int n_server = data->d_server.get(it_server->first);
				for (auto it_op = it_server->second.begin(),end_op = it_server->second.end();it_op!=end_op;++it_op) {
					if (it_op->first.empty()) continue;
					int n_op = data->d_op.get(it_op->first);
					intkey<3> key1 = {{
						n_service
						,n_server
						,n_op
					}};
					intkey<3> key2 = {{
						n_script
						,n_service
						,n_op
					}};
					{
						//std::lock_guard<std::mutex> lck(data.mtx);
						auto &c1 = data->stat_service_server_op.get_current(key1,curr1);
						auto &c2 = data->stat_script_service_op.get_current(key2,curr2);
						if (cmd.replace) {
							c1.data.clear();
							c2.data.clear();
						}
						for (auto it = it_op->second.begin(),end_it = it_op->second.end();it!=end_it;++it) {
							c1.add(*it);
							c2.add(*it);
						}
					}
				}
			}
		}
	}
};
struct put_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<put>
> {};

// получение списка сервисов или скриптов с сортировкой/лимитом
struct get_list_advanced {
	const char *name() const { return "get_list_advanced"; }
	typedef btprequest::get_list_advanced_json invoke_request;
	typedef aj::array<std::vector<aj::value<std::string> > > invoke_response;

	template<typename T> bool request(T& t, const btprequest::get_list_advanced& cmd,int id, std::vector<std::string> &result) {
		//auto lck = std::lock_guard<readlock>(readlock(data.mtx_dict));
		//std::lock_guard<std::mutex> lck(data.mtx);

		std::vector<std::pair<unsigned int,aggregated_counter>> outdata;
		std::vector<std::string> *vect = 0;

		if (cmd.service =="?") {
			std::unordered_map<int,std::vector<aggregated_counter>> service2arr;
			//auto &stat = data->stat_service_server_op.storage30m.last_data.size()?data->stat_service_server_op.storage30m.last_data:data->stat_service_server_op.storage1m.last_data;
			auto &stat = data->stat_service_server_op.storage30m.last_data;
			for (auto it = stat.begin();it!=stat.end();it++) {
				service2arr[it->first.data[0]].push_back(it->second);
			}

			vect = &data->d_service.rev;
			for (auto it=service2arr.begin();it!=service2arr.end();it++) {
				aggregated_counter cnt;
				cnt.supaggregate(it->second);
				outdata.push_back(std::pair<unsigned int,aggregated_counter>(it->first,cnt));
			}
			//todo: sort, limit, return
		} else if (cmd.script == "?" && cmd.op.length() && cmd.service.length()) {
			int val1 = data->d_service.get(cmd.service,false);
			if (val1 == -1) return true;
			int val2 = data->d_op.get(cmd.op,false);
			if (val2 == -1) return true;

			vect = &data->d_script.rev;

//			auto &stat = data->stat_script_service_op.storage30m.last_data.size()?data->stat_script_service_op.storage30m.last_data:data->stat_script_service_op.storage1m.last_data;
			auto &stat = data->stat_script_service_op.storage30m.last_data;
			for (unsigned int val_script = 1;val_script<data->d_script.data.size();val_script++) {
				auto stat_it = stat.find(intkey<3>{{val_script,val1,val2}});
				if (stat_it != stat.end() && stat_it->second.count>0) {
					outdata.push_back(std::pair<unsigned int,aggregated_counter>(val_script, stat_it->second));
				}
			}
		} else if (cmd.server == "?" && cmd.service.length() && cmd.op.length()) {
			int val1 = data->d_service.get(cmd.service,false);
			if (val1 == -1) return true;
			int val2 = data->d_op.get(cmd.op,false);
			if (val2 == -1) return true;

			vect = &data->d_server.rev;

			auto &stat = data->stat_service_server_op.storage30m.last_data;
			for (unsigned int val_server = 1;val_server < data->d_server.data.size();val_server++) {
				auto stat_it = stat.find(intkey<3>{{val1,val_server,val2}});
				if (stat_it != stat.end() && stat_it->second.count>0) {
					outdata.push_back(std::pair<unsigned int,aggregated_counter>(val_server, stat_it->second));
				}
			}

		} else return false;

		if (cmd.sort_by=="count") {
			std::sort(outdata.begin(),outdata.end(),[](const std::pair<int, aggregated_counter> &a, const std::pair<int,aggregated_counter>&b) -> bool {
				return a.second.count > b.second.count;
			});
		} else if (cmd.sort_by == "total") {
			std::sort(outdata.begin(),outdata.end(),[](const std::pair<int, aggregated_counter> &a, const std::pair<int,aggregated_counter>&b) -> bool {
				return a.second.avg*a.second.count > b.second.avg*b.second.count;
			});
		} else if (cmd.sort_by == "perc80") {
			std::sort(outdata.begin(),outdata.end(),[](const std::pair<int, aggregated_counter> &a, const std::pair<int,aggregated_counter>&b) -> bool {
				return a.second.perc80 > b.second.perc80;
			});
		} else return false;

		const std::vector<std::string> &dict = *vect;
		for (auto it=outdata.begin();it!=outdata.end();it++) {
			if (result.size()>cmd.limit) break;
			if (it->first < dict.size() && dict[it->first].length()) {
				result.push_back(dict[it->first]);
			}
		}
		return true;
	}
};
struct get_list_advanced_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_list_advanced>
> {};



// получение списка сервисов, скриптов, серверов или операций
struct get_list {
	const char *name() const { return "get_list"; }
	typedef btprequest::get_graph_json invoke_request;
	typedef aj::array<std::set<aj::value<std::string> > > invoke_response;

	template<typename T> bool request(T& t, const btprequest::get_graph& cmd,int id, std::set<std::string> &result) {
		//auto lck = std::lock_guard<readlock>(readlock(data.mtx_dict));
		//std::lock_guard<std::mutex> lck(data.mtx);
		std::vector<std::string> *vect = 0;
		std::vector<std::string> mvect;

		if (cmd.service =="?") {
			if (cmd.script.length()) {
				int val = data->d_script.get(cmd.script,false);
				if (val == -1) return true;
				auto &t = data->c_script_service.data[val];
				int sz = data->d_service.rev.size();
				for (auto it = t.begin();it!=t.end();it++) {
					if (*it<sz) mvect.push_back(data->d_service.rev[*it]);
				}
			} else {
				vect = &data->d_service.rev;
			}
		} else if (cmd.op == "?") {
			if (cmd.service.length()) {
				int val = data->d_service.get(cmd.service,false);
				if (val == -1) return true;
				auto &t = data->c_service_op.data[val];
				int sz = data->d_op.rev.size();
				for (auto it = t.begin();it!=t.end();it++) {
					if (*it<sz) mvect.push_back(data->d_op.rev[*it]);
				}
			} else {
				vect = &data->d_op.rev;
			}
		} else if (cmd.script == "?") {
			if (cmd.service.length() && cmd.op.length()) {
				int val1 = data->d_service.get(cmd.service,false);
				if (val1 == -1) return true;
				int val2 = data->d_op.get(cmd.op,false);
				if (val2 == -1) return true;
				std::vector<std::pair<int,long long int>> script_count;
				//auto &scripts= data->d_script.rev;
				for (unsigned int val_script = 1;val_script<data->d_script.data.size();val_script++) {
					auto &stat = data->stat_script_service_op.storage30m.last_data;
					auto stat_it = stat.find(intkey<3>{{val_script,val1,val2}});
					if (stat_it != stat.end() && stat_it->second.count>0) {
						script_count.push_back(std::pair<int,long long int>(val_script, stat_it->second.count));
						//mvect.push_back(data->d_script.rev[*it]);
					}
				}
				std::sort(script_count.begin(),script_count.end(),[](const std::pair<int, long long int> &a, const std::pair<int, long long int>&b) -> bool {
					return a.second>b.second;
				});
				for (auto it=script_count.begin();it!=script_count.end();it++) {
					if (mvect.size()>cmd.limit) break;
					mvect.push_back(data->d_script.rev[it->first]);
				}
			} else {
				vect = &data->d_script.rev;
			}
		} else if (cmd.server == "?") {
			if (cmd.service.length()) {
				int val = data->d_service.get(cmd.service,false);
				if (val == -1) return true;
				auto &t = data->c_service_server.data[val];
				int sz = data->d_server.rev.size();
				for (auto it = t.begin();it!=t.end();it++) {
					if (*it<sz) mvect.push_back(data->d_server.rev[*it]);
				}
			} else {
				vect = &data->d_server.rev;
			}
//		} else if (cmd.source_srv == "?") {
//			vect = &data->d_ssrv.rev;
		} else return false;

		if (vect) for (unsigned int i=0;i<vect->size();i++) result.insert((*vect)[i]);
		if (mvect.size()) for (unsigned int i=0;i<mvect.size();i++) result.insert(mvect[i]);
		result.erase("");
		return true;
	}
};
struct get_list_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_list>
> {};

// получение списка предупреждений
struct get_warnings {
	const char *name() const { return "get_warnings"; }
	typedef btprequest::get_graph_json invoke_request;
	typedef aj::array<std::set<aj::value<std::string> > > invoke_response;


	template<typename T>
	static void add_to_strlist(int &l, int&r, const std::string &val,T &d) {
		if (val =="*" || val=="?" || !val.size()) {
			l = 0;
			r = d.rev.size();
		} else if (d.data.count(val)) {
			l = d.data[val];
			r = l+1;
		} else {
			l = 0;
			r = 0;
		}
	}

	template<typename T> bool request(T& t, const btprequest::get_graph& cmd,int id, std::set<std::string> &result) {
		int p0l,p0r;
		int p1l,p1r;
		int p2l,p2r;
		//auto lck = std::lock_guard<readlock>(readlock(data.mtx_dict));
		//std::lock_guard<std::mutex> lck(data.mtx);

		bool scriptSearch;
		int srch;
		dictionary *dict_result;

		if (cmd.script == "?" || cmd.script.size()) {
			if (!cmd.service.size()) return false;
			scriptSearch = true;
			if (cmd.script == "?") {dict_result = &data->d_script; srch = 0;}
			else if (cmd.service == "?") {dict_result = &data->d_service; srch = 1;}
			else if (cmd.op == "?") {dict_result = &data->d_op; srch = 2;}
			else return false;
		} else {
			scriptSearch = false;
			if (cmd.service == "?") {dict_result = &data->d_service; srch = 0;}
			else if (cmd.server == "?") {dict_result = &data->d_server; srch = 1;}
			else if (cmd.op == "?") {dict_result = &data->d_op; srch = 2;}
			else return false;
		}

		RoundRobinPeriodicalStorage<3> &stat = scriptSearch ? data->stat_script_service_op : data->stat_service_server_op;
		codictionary &dict = scriptSearch ? data->c_script_service : data->c_service_server;
		if (scriptSearch) {
			//data->stat_script_service_op;
			add_to_strlist(p0l,p0r,cmd.script,data->d_script);
			add_to_strlist(p1l,p1r,cmd.service,data->d_service);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		} else {
			//data->stat_service_dsrv_op;
			add_to_strlist(p0l,p0r,cmd.service,data->d_service);
			add_to_strlist(p1l,p1r,cmd.server,data->d_server);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		}
		std::set<int> res;
		int curr = time(0);
		for (int i0 = p0l;i0<p0r;i0++) {
			for (int i1 = p1l;i1<p1r;i1++) if (dict.has(i0,i1)) {
				for (int i2 = p2l;i2<p2r;i2++) if (data->c_service_op.has(scriptSearch?i1:i0,i2)) {
					//result.data.push_back(
					auto r1 = stat.get_storage(60).get<false>(intkey<3>{{i0,i1,i2}},curr);
					auto r2 = stat.get_storage(1800).get<false>(intkey<3>{{i0,i1,i2}},curr);
					if (r1.meta->ts && r2.meta->ts && r1.data && r2.data) {
						aggregated_counter &d1 = r1.get_by_tsdelta(0, 60);
						aggregated_counter &d2 = r2.get_by_tsdelta(cmd.scale, 1800);
						if (d2.perc80>0 && d1.count>10 && d2.count>180 && (
							//(d1.perc95 > (long long int)(100+d2.perc99*1.2F)) ||
							(d1.perc80 > (long long int)(1000+d2.perc95*1.5F))
							|| (d1.perc80 > (long long int)(100+d2.perc99*1.4F))
						)) {
							if (LOG_VERBOSITY>=1) {
								if (scriptSearch) {
									std::cout << "w: " << data->d_script.rev[i0] << "-" << data->d_service.rev[i1] << "-" << data->d_op.rev[i2];
								} else {
									std::cout << "w: " << data->d_service.rev[i0] << "-" << data->d_server.rev[i1] << "-" << data->d_op.rev[i2];
								}
								std::cout << d1.perc80 << " vs " << d2.perc95 << std::endl;
							}
							res.insert(srch==0?i0: (srch==1?i1:i2));
							r1.free();
							r2.free();
							goto cont;
						}
					}
					r1.free();
					r2.free();
				}
			}
			cont: ;
		}
		for (auto it=res.begin();it!=res.end();it++) {
			result.insert(dict_result->rev[*it]);
		}
		return true;
	}
};
struct get_warnings_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_warnings>
> {};


// получение данных для графика одного счётчика, но всем метрикам
struct get_graph {
	const char *name() const { return "get_graph"; }
	typedef btprequest::get_graph_json invoke_request;
	typedef btpresponse::get_graph_json invoke_response;

	template<typename T>
	static void add_to_strlist(int &l, int&r, const std::string &val,T &d) {
		if (val =="*" || !val.size()) {
			l = 0;
			r = d.rev.size();
		} else if (d.data.count(val)) {
			l = d.data[val];
			r = l+1;
		} else {
			l = 0;
			r = 0;
		}
	}

	template<typename T> bool request(T& t, const btprequest::get_graph& cmd,int id, btpresponse::get_graph &result) {
		int p0l,p0r;
		int p1l,p1r;
		int p2l,p2r;
		//auto lck = std::lock_guard<readlock>(readlock(data.mtx_dict));
		//std::lock_guard<std::mutex> lck(data.mtx);

		std::map< int, std::vector< aggregated_counter > > res;
		int curr = time(0);
		bool scriptSearch = cmd.script!="";

		RoundRobinStorage<3> &st = scriptSearch ? data->stat_script_service_op.get_storage(cmd.scale) : data->stat_service_server_op.get_storage(cmd.scale);
		codictionary &dict = scriptSearch ? data->c_script_service : data->c_service_server;
		if (scriptSearch) {
			//data->stat_script_service_op;
			add_to_strlist(p0l,p0r,cmd.script,data->d_script);
			add_to_strlist(p1l,p1r,cmd.service,data->d_service);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		} else {
			//data->stat_service_dsrv_op;
			add_to_strlist(p0l,p0r,cmd.service,data->d_service);
			add_to_strlist(p1l,p1r,cmd.server,data->d_server);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		}
		result.scale = st.scale_ts;

		for (int i0 = p0l;i0<p0r;i0++) {
			for (int i1 = p1l;i1<p1r;i1++) if (dict.has(i0,i1)) {
				for (int i2 = p2l;i2<p2r;i2++) {
					//result.data.push_back(
					auto r = st.get<false>(intkey<3>{{i0,i1,i2}},curr);
					if (r.meta->ts && r.data) for (int i=0;i<r.meta->count;i++) {
						int ts = r.meta->get_ts(i);
						if (r.data[i].count) {
							res[ts].push_back(r.data[i]);
						} else res[ts];
					}
					r.free();
				}
			}
		}

		result.data.reserve(res.size());
		for (auto it = res.begin();it!=res.end();it++) {
			aggregated_counter agr;
			//std::cout << it->first << " ";
			agr.supaggregate(it->second);
			result.data.push_back(agr);
		}
		 result.ts = res.size() ? res.rbegin()->first : time(0);
		//std::cout << "\n";
		return true;
	}
};
struct get_graph_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_graph>
> {};



// получение данных для графика нескольких счётчикво, но по одной метрике (count/perc50/perc80....)
struct get_multigraph {
	const char *name() const { return "get_multigraph"; }
	typedef btprequest::get_graph_json invoke_request;
	typedef btpresponse::get_multigraph_json invoke_response;

	template<typename T>
	static void add_to_strlist(int &l, int&r, const std::string &val,T &d) {
		if (val =="*" || !val.size()) {
			l = 0;
			r = d.rev.size();
		} else if (d.data.count(val)) {
			l = d.data[val];
			r = l+1;
		} else {
			l = 0;
			r = 0;
		}
	}

	template<typename T> bool request(T& t, const btprequest::get_graph& cmd,int id, btpresponse::get_multigraph &result) {
		int p0l,p0r;
		int p1l,p1r;
		int p2l,p2r;
		//std::lock_guard<std::mutex> lck(data.mtx);

		std::map<int, std::map< int, std::vector< aggregated_counter > > > res;
		int curr = time(0);
		bool scriptSearch = cmd.script!="";

		RoundRobinStorage<3> &st = scriptSearch ? data->stat_script_service_op.get_storage(cmd.scale) : data->stat_service_server_op.get_storage(cmd.scale);
		codictionary &codict = scriptSearch ? data->c_script_service : data->c_service_server;

		if (scriptSearch) {
			//data->stat_script_service_op;
			add_to_strlist(p0l,p0r,cmd.script,data->d_script);
			add_to_strlist(p1l,p1r,cmd.service,data->d_service);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		} else {
			//data->stat_service_server_op;
			add_to_strlist(p0l,p0r,cmd.service,data->d_service);
			add_to_strlist(p1l,p1r,cmd.server,data->d_server);
			add_to_strlist(p2l,p2r,cmd.op,data->d_op);
		}

		dictionary *dict_result;
		int srch;
		if (cmd.script == "*" || cmd.script.size()) {
			scriptSearch = true;
			if (cmd.script == "*") {dict_result = &data->d_script; srch = 0;}
			else if (cmd.service == "*") {dict_result = &data->d_service; srch = 1;}
			else if (cmd.op == "*") {dict_result = &data->d_op; srch = 2;}
			else return false;
		} else {
			scriptSearch = false;
			if (cmd.service == "*") {dict_result = &data->d_service; srch = 0;}
			else if (cmd.server == "*") {dict_result = &data->d_server; srch = 1;}
			else if (cmd.op == "*") {dict_result = &data->d_op; srch = 2;}
			else return false;
		}

		result.scale = st.scale_ts;

		for (int i0 = p0l;i0<p0r;i0++) {
			for (int i1 = p1l;i1<p1r;i1++) if (codict.has(i0,i1)) {
				for (int i2 = p2l;i2<p2r;i2++) {
					//result.data.push_back(
					auto r = st.get<false>(intkey<3>{{i0,i1,i2}},curr);
					if (r.meta->ts && r.data) for (int i=0;i<r.meta->count;i++) {
						int ts = r.meta->get_ts(i);
						int ind = srch==0?i0:(srch==1?i1:i2);
						if (r.data[i].count) {
							res[ind][ts].push_back(r.data[i]);
						} else res[ind][ts];
					}
					r.free();
				}
			}
		}

		//result.data.reserve(res.size());
		for (auto it = res.begin();it!=res.end();it++) {
			for (auto it2 = it->second.begin();it2!=it->second.end();it2++) {
				aggregated_counter agr;
				//std::cout << it->first << " ";
				agr.supaggregate(it2->second);
				result.data[dict_result->rev[it->first]].push_back(agr.get_field(cmd.field));
			}
		}
		 result.ts = res.size() ? res.begin()->second.rbegin()->first : time(0);
		//std::cout << "\n";
		return true;
	}
};
struct get_multigraph_advice : aa::advice<
	aa::tag_list_n<
		aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_multigraph>
> {};

struct mutex_advice : aa::advice< af::_mutex_, ad::ad_mutex< ::fas::system::thread::mutex> > { };

typedef ap::type_list_n<
	put_advice
	, delete_advice
	, get_list_advice
	, get_list_advanced_advice
	, get_graph_advice
	, get_multigraph_advice
	, get_warnings_advice
	, mutex_advice
	, ::fas_stats::get_statistics_advice< ::fas_stats::get_statistics_helper_null >
	, ::fas_stats::ad_span_reg_mtx_advice
>::type method_list;

typedef aa::aspect<method_list> method_aspect;
typedef aa::aspect_merge< common::aspect::rn_json_aspect, method_aspect>::type json_aspect;

}
