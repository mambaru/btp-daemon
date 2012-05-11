#define MYMYSQL_DEBUG 0
#define LOG_VERBOSITY 0

#include <boost/program_options.hpp>
namespace po = ::boost::program_options;
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <list>
#include <thread>

#include <fas/mux/best_mux.hpp>
#include <fas/mux/imux.hpp>
#include <fas/inet/server.hpp>
#include <fas/inet/mt_server.hpp>

#include <fas/adv/json_rpc3/method.hpp>

#include "common/params.h"
#include "common/lock_atomic.hpp"
#include <fas/mux/epoller.hpp>

#include "common/misc.hpp"

namespace as = ::fas::system;
namespace asi = ::fas::system::inet;
namespace af = ::fas::filter;
namespace ai = ::fas::inet;
namespace am = ::fas::mux;
namespace aa = ::fas::aop;
namespace ap = ::fas::pattern;
namespace ad = ::fas::adv;
namespace ajr = ::fas::adv::json_rpc3;
namespace aj = ::fas::json;

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>

void set_my_scheduler(int policy, int param) {
	sched_param p = {param};
	int val = sched_setscheduler(pthread_self(), policy, &p);
	if (!val) return;
	if (val==EINVAL) std::cout << "sched_setscheduler: EINVAL\n";
	if (val==EPERM) std::cout << "sched_setscheduler: EPERM\n";
	if (val==ESRCH) std::cout << "sched_setscheduler: ESRCH\n";
}

#include "fas_queries.hpp"
#include "btpdata.hpp"
std::unique_ptr<BtpDaemonData> data;
volatile bool is_running;
#include "fas_methods.hpp"

fas::mux::best_mux mux;

struct server_tcp: public ai::server<
	btpmethods::json_aspect
> {
	template<typename T>
	void initobj(T &t, ::fas_stats::common_stat_mtx *stat) {
		t.get<ad::_span_reg_>().stat_object(stat);
		t.get<fas_stats::_get_statistics_>().stat_count = 1;
		t.get<fas_stats::_get_statistics_>().stat = stat;
	}

	void init(::fas_stats::common_stat_mtx *stat) {
		this->set_nonblock(true);
		initobj(get_prototype().get_aspect(), stat);
	}
};

#include "common/udp.hpp"
struct server_udp : public common::server_udp< ai::mux_connection_base , btpmethods::json_aspect> {
	void init(::fas_stats::common_stat_mtx *stat) {
		this->set_nonblock(true);
		this->get_aspect().get<ad::_span_reg_>().stat_object(stat);
		this->get_aspect().get<fas_stats::_get_statistics_>().stat_count = 1;
		this->get_aspect().get<fas_stats::_get_statistics_>().stat = stat;
	}
};

int main (int argc, char **argv) {
	std::string log_file;
	po::options_description options;
	po::options_description gopt("Generic options");
	std::string path;
	int port;
	int thread_count;
	int tune;
	int synctime;

	gopt.add_options()
		("help,h", "show help message" )
		("daemonize,d", "daemonize" )
		("log,l", po::value<std::string>(&log_file),"log file" )
		("port,p", po::value<int>(&port)->default_value(22400),"port number" )
		("threads,t", po::value<int>(&thread_count)->default_value(3),"number of threads" )
		("path", po::value<std::string>(&path)->default_value(""),"path to store database" )
		("tune", po::value<int>(&tune)->default_value(64),"tuning parameter, it (as a multiplier) affects size of mmaped regions and initial kyoto hashmap size. Mamba uses 1024" )
		("synctime", po::value<int>(&synctime)->default_value(30), "period to sync to disk")
	;
	options.add(gopt);

    po::variables_map vm;
    try {
		po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
		po::notify(vm);
    } catch(std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
    }
    if (vm.count("help")) {
    	std::cout << options << std::endl;
    	return 0;
    }

    std::cout << vm << std::endl;

    if (log_file.length()) {
        std::freopen(log_file.c_str(), "a+", stderr);
        std::freopen(log_file.c_str(), "a+", stdout);
        std::cout << vm << std::endl;
    }


    if (vm.count("daemonize")) {
    	common::daemonize(log_file.length()?true:false,log_file.length()?true:false);
    }


	fas::system::dumpable();
	if (path.length()) {
		if (*path.rbegin() != '/') path.append("/");
	}
	data = std::unique_ptr<BtpDaemonData>(new BtpDaemonData(path,tune));
	is_running = true;

	//тред, который сбрасывает секундные данные о счётчиках
	std::thread thr_roll = std::thread([]{
		time_t ts = time(0);
		set_my_scheduler(SCHED_FIFO,10);
		while (is_running) {
			ts++;
			while (microtime() < ts) usleep(std::max(1000,(int)((microtime()-ts)*500000)));	//sleep half of time - for precision
			data->roll(ts);
		}
	});

	//тред, который обсчитывает статистику
	std::thread thr_aggregate = std::thread([]{
		time_t ts = time(0);
		while (is_running) {
			ts++;
			double uts = microtime();
			if (uts-ts<RoundRobinPeriodicalStorage<3>::second_data_size-60-15) {
				while (microtime() < ts) usleep(std::max(100000,(int)((microtime()-ts)*500000)));	//sleep half of time - for precision
				while (is_running && !data->is_aggregation_allowed(ts)) usleep(100000);
				if (!is_running) break;
				data->aggregate(ts);
			} else {
				std::cout << "I AM LATE OH FUCK: " << (time_t)uts << " vs " << ts << std::endl;
				ts = time(0);
			}
		}
	});

	//тред, который сохраняет данные на диск
	std::thread thr_sync = std::thread([synctime]{
		set_my_scheduler(SCHED_IDLE,0);
		int cnt = 0;
		while (is_running) {
			if (!(cnt%synctime)) {
				std::cout << "sync: " << time(0) << std::endl;
				data->sync();
			}
			cnt++;
			sleep(1);
		}
	});

	::fas_stats::common_stat_mtx stat;
	fas::mux::best_mux mux;

	//tcp-тред
	std::thread thr_tcp = std::thread([&stat,port]{
		server_tcp json_tcp_srv;
		fas::mux::best_mux mux;
		json_tcp_srv.set_mux(&mux);
		json_tcp_srv.init( &stat);
		json_tcp_srv.start( port );


		while (is_running) {
			try {
				mux.select(5);
			} catch(std::exception& e) {
				log_write ("std::exception:\n" << e.what());
			} catch(...) {
				log_write ("unhandled exception");
			}
		}
	});

	server_udp json_udp_srv;
	json_udp_srv.set_mux(&mux);
	json_udp_srv.init(&stat);
	json_udp_srv.start( port );

	//несколько udp-тредов
	std::thread* thrudp[thread_count];
	for (int i=0;i<thread_count;i++) thrudp[i] = new std::thread([&json_udp_srv,&stat]{
		set_my_scheduler(SCHED_FIFO,1);
		fas::mux::best_mux mux;
		server_udp json_udp_srv2;
		json_udp_srv2.set_mux(&mux);
		json_udp_srv2.init(&stat);
		json_udp_srv2.start_on_same_socket(json_udp_srv);
		while (is_running) {
			try {
				mux.select(5);
			} catch(std::exception& e) {
				log_write ("std::exception:\n" << e.what());
			} catch(...) {
				log_write ("unhandled exception");
			}
		}
	});

	auto sighndlr = [](int sig){
		is_running = false;
		std::cout << "Shutting down...\n";
	};
	signal(SIGINT,sighndlr);
	signal(SIGTERM,sighndlr);

	while (is_running) {
		try {
			mux.select(5);
		} catch(std::exception& e) {
			log_write ("std::exception:\n" << e.what());
		} catch(...) {
			log_write ("unhandled exception");
		}
	}

	thr_roll.join();
	thr_aggregate.join();
	thr_sync.join();
	thr_tcp.join();
	for (int i=0;i<thread_count;i++) thrudp[i]->detach();
	sleep(1);
	data->close();
    return 0;
}
