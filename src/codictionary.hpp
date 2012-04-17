#pragma once

#include <unordered_set>
#include <unordered_map>

/**
 * список связей между парами объектов разных типов
 */
struct codictionary {
	std::unordered_map<int, std::unordered_set<int> > data;
	void add(int k,int v) {
		data[k].insert(v);
	}
	bool has(int k,int v) {
		return data.count(k) && data[k].count(v);
	}
	void remove1(int k) {
		std::unordered_set<int> s;
		std::swap(data[k],s);
	}
	void remove2(int v) {
		for (auto it=data.begin();it!=data.end();it++) {
			if (it->second.count(v)) it->second.erase(v);
		}
	}
};

