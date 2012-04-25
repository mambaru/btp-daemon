#pragma once

//#include <unordered_map>
#include <tbb/concurrent_unordered_map.h>

#if TBB_VERSION_MAJOR>=4
#include <tbb/concurrent_unordered_set.h>
typedef tbb::concurrent_unordered_set<int> codictionary_set;
#else
#warning "Your Intel TBB is old. TBB v.4+ is recommended. Otherwise you could get problems during a high workload"
#include <unordered_set>
typedef std::unordered_set<int> codictionary_set;
#endif

/**
 * список связей между парами объектов разных типов
 */

struct codictionary {
	tbb::concurrent_unordered_map<int, codictionary_set > data;
	void add(int k,int v) {
		data[k].insert(v);
	}
	bool has(int k,int v) {
		return data.count(k) && data[k].count(v);
	}
	void remove1(int k) {
		#if TBB_VERSION_MAJOR>=4
			data[k].empty();
		#else
			codictionary_set s;
			std::swap(data[k],s);
		#endif
	}
	void remove2(int v) {
		for (auto it=data.begin();it!=data.end();it++) {
			remove12(it->first, v);
		}
	}
	void remove12(int k,int v) {
		auto it = data.find(k);
		if (it==data.end()) return;
		codictionary_set s;
		std::swap(it->second,s);
		#if TBB_VERSION_MAJOR>=4
			s.unsafe_erase(v);
		#else
			s.erase(v);
		#endif
		std::swap(it->second,s);
	}
};

