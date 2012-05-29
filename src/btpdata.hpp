#pragma once

#include <thread>

#include <kcdb.h>
#include <kchashdb.h>
namespace kc=::kyotocabinet;

#include "codictionary.hpp"
#include "dictionary.hpp"

#include "types.hpp"

#include "buffered_kc.hpp"

#include "rrstorage_single.hpp"
#include "rrstorage_periodical.hpp"



/**
 * общая структура данных со всеми хранилищами, словарями и словарями связей
 */
struct BtpDaemonData {
	//multispinlock mtx_dict;
	//std::mutex mtx;
	RoundRobinPeriodicalStorage<3> stat_service_server_op;
	RoundRobinPeriodicalStorage<3> stat_script_service_op;
	dictionary d_op;
	dictionary d_service;
	dictionary d_server;
	dictionary d_farm;
	dictionary d_script;
	codictionary c_service_op;
	codictionary c_service_server;
	codictionary c_script_service;

	BtpDaemonData(std::string path, int tune) :
		stat_service_server_op(path,"service_dsrv_op", tune)
		,stat_script_service_op(path,"script_service_op", tune)
		,d_op(path,"dict_op")
		,d_service(path,"dict_service")
		,d_server(path,"dict_dsrv")
		,d_farm(path,"dict_farm")
		,d_script(path,"dict_script")
	{
		std::thread thr1 = std::thread([this](){
			auto process1 = [this](kyotocabinet::HashDB::Cursor *cur) {
				cur->jump();
				std::string ckey;
				while (cur->get_key(&ckey, true)) {
					int* k = (int*)ckey.data();
					c_service_server.add(k[0],k[1]);
					c_service_op.add(k[0],k[2]);
				}
				delete cur;
			};
			process1(stat_service_server_op.storage30m.db_meta->cursor());
			process1(stat_service_server_op.storage1m.db_meta->cursor());
		});
		std::thread thr2 = std::thread([this](){
			auto process2 = [this](kyotocabinet::HashDB::Cursor *cur) {
				cur->jump();
				std::string ckey;
				while (cur->get_key(&ckey, true)) {
					int* k = (int*)ckey.data();
					c_script_service.add(k[0],k[1]);
				}
				delete cur;
			};
			process2(stat_script_service_op.storage30m.db_meta->cursor());
			process2(stat_script_service_op.storage1m.db_meta->cursor());
		});
		//db_script_service_op = btpstorage<3>(5, 3000);
		thr1.join();
		thr2.join();
	}

	void close() {
		stat_service_server_op.stop();
		stat_script_service_op.stop();
	}

	void sync() {
		stat_service_server_op.sync();
		stat_script_service_op.sync();
	}

	volatile int ts_roll;
	bool is_aggregation_allowed(time_t ts) {
		return ts < ts_roll - 5;
	}

	void roll(time_t ts) {
		if (LOG_VERBOSITY>=1) std::cout << "roll beg: " << microtime_str() << " for " << ts << std::endl;
		auto p1 = stat_service_server_op.roll(ts);
		auto p2 = stat_script_service_op.roll(ts);
		ts_roll = ts;
		if (LOG_VERBOSITY>=1) std::cout << "roll end: " << microtime_str() << std::endl;
		if (p1!=NULL) delete p1;
		if (p2!=NULL) delete p2;
	}

	void aggregate(time_t ts) {
		//while (ts_roll<=ts+5) usleep(100000);

		if (LOG_VERBOSITY>=1) std::cout << "aggregate beg: " << ts << " @ " << microtime_str() << std::endl;
		std::thread thr1 = std::thread([this,ts](){
			set_my_scheduler(SCHED_IDLE,0);
			auto ret = stat_service_server_op.run_aggregation(ts);
			for (auto it = ret.begin();it!=ret.end();it++) {
				c_service_server.add(it->first.data[0],it->first.data[1]);
				c_service_op.add(it->first.data[0],it->first.data[2]);
			}
		});
		std::thread thr2 = std::thread([this,ts](){
			set_my_scheduler(SCHED_IDLE,0);
			auto ret = stat_script_service_op.run_aggregation(ts);
			for (auto it = ret.begin();it!=ret.end();it++) {
				c_script_service.add(it->first.data[0],it->first.data[1]);
			}
		});

		thr1.join();
		thr2.join();
		if (LOG_VERBOSITY>=1) std::cout << "aggregate end: " << ts << " @ " << microtime_str() << std::endl;
	}
};
