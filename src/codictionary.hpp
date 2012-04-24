#pragma once

//#include <unordered_set>
//#include <unordered_map>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>

/**
 * список связей между парами объектов разных типов
 */
struct codictionary {
	tbb::concurrent_unordered_map<int, tbb::concurrent_unordered_set<int> > data;
	void add(int k,int v) {
		data[k].insert(v);
	}
	bool has(int k,int v) {
		return data.count(k) && data[k].count(v);
	}
	void remove1(int k) {
		//tbb::concurrent_unordered_set<int> s;
		//std::swap(data[k],s);
		data[k].empty();
	}
	void remove2(int v) {
		for (auto it=data.begin();it!=data.end();it++) {
			remove12(it->first, v);
		}
	}
	void remove12(int k,int v) {
		auto it = data.find(k);
		if (it==data.end()) return;
		tbb::concurrent_unordered_set<int> s;
		std::swap(it->second,s);
		s.unsafe_erase(v);
		std::swap(it->second,s);
	}
};

