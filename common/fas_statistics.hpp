#pragma once

#include <sys/times.h>
#include <malloc.h>
#include <dirent.h>
#include <list>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <mutex>
#endif

#include <fas/adv/ad_mutex.hpp>
#include <fas/filter/basic_filter.hpp>
namespace ad = ::fas::adv;
namespace af = ::fas::filter;

namespace fas_stats {

#ifdef __GXX_EXPERIMENTAL_CXX0X__
struct common_stat_mtx : public fas::misc::common_stat {
	std::mutex mtx;
	void mark(int i) {
		std::lock_guard<std::mutex> lck(mtx);
		fas::misc::common_stat::mark(i);
	}
	bool marked_span( timeval tv ) {
		std::lock_guard<std::mutex> lck(mtx);
		return fas::misc::common_stat::marked_span(tv);
	}
};
#ifdef STATS_USE_GLOBAL
common_stat_mtx global_common_stat;
#endif

class ad_span_reg_mtx {
	common_stat_mtx* _common_stat;
public:
#ifdef STATS_USE_GLOBAL
	ad_span_reg_mtx(): _common_stat(&global_common_stat) {};	//TODO: костыль
#endif
	void stat_object(common_stat_mtx* stat) { _common_stat = stat; }
	const common_stat_mtx* stat_object() const { return _common_stat;}
	void mark(int i) { if (_common_stat!=0) _common_stat->mark(i); };
	void span(timeval tv) { if (_common_stat!=0) _common_stat->marked_span(tv); };
};

typedef aa::advice< aa::tag<ad::_span_reg_>, ad_span_reg_mtx> ad_span_reg_mtx_advice;

#endif



time_t g_uptime = time(0);

float maketime(timeval span) {
	 return span.tv_sec*1000 + span.tv_usec/1000.0F;
}
template<int pos> void _method_statistics( ap::empty_type, const ::fas::misc::common_stat*, int stat_count, std::ostringstream&){
}
template<int pos, typename L> void _method_statistics( L, const ::fas::misc::common_stat* stat, int stat_count, std::ostringstream& stream) {
	typedef typename L::left_type method_type;
	typedef typename L::right_type method_list;

	method_type method;
	if (pos!=0) stream << ",";
	int count = 0;
	double average_time = 0.0F;
	double total_time = 0.0F;

	for (int i=0;i<stat_count;i++) {
	  count += stat[i][pos].count();
	  average_time += maketime(stat[i][pos].get_average())/((float)stat_count);
	  total_time += maketime(stat[i][pos].get_sum());
	}

	stream << '"' << method.name() << "\":{";
	stream << "\"count\":" << count << ",";
	stream << "\"average_time\":" << average_time << ",";
	stream << "\"total_time\":" << total_time << "}";
	//std::endl;
	//  _command_statistics( stat[pos], cmd);
	_method_statistics<pos + 1>( method_list(), stat, stat_count, stream);
}

static void _memory_statistics(std::ostringstream &str) {
	char *vmsize = NULL;
	char *vmrss = NULL;

	char *line = (char*)malloc(1024);
	size_t len = 1024;

	FILE *f = fopen("/proc/self/status", "r");
	if (!f) return;

	while (!vmsize || !vmrss) {
		if (getline(&line, &len, f) == -1) {
			fclose(f);
			return;
		}

		if (!strncmp(line, "VmSize:", 7)) {
			vmsize = strdup(&line[7]);
		} else if (!strncmp(line, "VmRSS:", 6)) {
			vmrss = strdup(&line[7]);
		}
	}
	fclose(f);

	/* Get rid of " kB\n"*/
	len = strlen(vmsize);
	vmsize[len - 4] = 0;
	len = strlen(vmrss);
	vmrss[len - 4] = 0;
	str << ",\"memory\":{\"virtual\":" << vmsize << ",\"resident\":" << vmrss << "}";

	free(vmsize);
	free(vmrss);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
struct get_statistics_callback_list {
	static std::list< std::function<void (std::ostringstream&) > > list;

	static void get_statistics(std::ostringstream &str) {
		for (auto it = list.begin();it!=list.end();it++) {
			(*it)(str);
		}
	}
};
std::list< std::function<void (std::ostringstream&) > > get_statistics_callback_list::list;
#endif

template<typename H>
struct get_statistics {
	const char *name() const { return "get_statistics"; }
	typedef aj::value<int> invoke_request;
	typedef aj::raw_value< std::string > invoke_response;

	fas::misc::common_stat *stat;
	int stat_count;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#ifdef STATS_USE_GLOBAL
	get_statistics(): stat(&global_common_stat),stat_count(1) {
	}
#else
	get_statistics(): stat(0),stat_count(0) {
	}
	get_statistics(const get_statistics<H> &t): stat(t.stat),stat_count(t.stat_count){
	}
#endif
#endif

	template<typename T> bool request(T& t, const int& cmd, int id, std::string& result ) {
		typedef typename T::aspect A;
		typedef typename A::advice_list advice_list;
		typedef typename ap::select< ajr::_gmethod_, advice_list >::type methods_list;
		std::ostringstream str;
		str << "{";
		if (stat!=0) _method_statistics<0>(methods_list(), stat, stat_count, str); else str << "\"stat_is_zero\":true";
		H().get_statistics(str);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
		get_statistics_callback_list::get_statistics(str);
#endif

		tms times;
	    ::times(&times);
		str << ",\"system\":{";
		str << "\"pid\":" << ::getpid();
		str << ",\"uptime\":" << time(0) - g_uptime;
		str << ",\"user\":" << times.tms_utime / ::sysconf(_SC_CLK_TCK);
		str << ",\"system\":" << times.tms_stime / ::sysconf(_SC_CLK_TCK);

		DIR *dirp = opendir("/proc/self/fd");
		if(dirp!=NULL) {
			int cnt = 0;
			while (readdir(dirp)) cnt++;
			closedir(dirp);
			str << ",\"files\":" << cnt;
		}
		str << "}";

		_memory_statistics(str);

		str << "}";
		result = str.str();
		//invoke_response::serializer()(result, beg, end)
		return true;
	}
};
struct _get_statistics_{};

struct get_statistics_helper_null{
	void get_statistics(std::ostringstream &str) {
	}
};

template<typename H>
struct get_statistics_advice : aa::advice<
	aa::tag_list_n<
		aa::tag<_get_statistics_>
		,aa::gtag< ajr::_gmethod_ >
	>::type,
	ajr::method<get_statistics<H> >
> {
};


}
