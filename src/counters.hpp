#pragma once

#include <unordered_map>
#include <vector>


/**
 * данные об индивидуальных событиях
 */
struct counter {
	tbb::concurrent_vector<long long int> data;

	void add(long long int ts) {
		data.push_back(ts);
	}
	void add(const counter &other) {
		data.reserve(data.size()+other.data.size());
		for (size_t i=0;i<other.data.size();i++) data.push_back(other.data[i]);
	}
};

enum class aggregated_counter_field { avg, count, perc50, perc80, perc95, perc99 };

/**
 * аггрегированные данные о событиях
 */
struct aggregated_counter {
	long long int avg;
	long long int count;
	long long int perc50;
	long long int perc80;
	long long int perc95;
	long long int perc99;

	long long int get_field(aggregated_counter_field f) {
		if (f==aggregated_counter_field::avg) return avg;
		if (f==aggregated_counter_field::count) return count;
		if (f==aggregated_counter_field::perc50) return perc50;
		if (f==aggregated_counter_field::perc80) return perc80;
		if (f==aggregated_counter_field::perc95) return perc95;
		return perc99;
	}

	void supaggregate(std::vector<aggregated_counter> v) {
		if (v.size()==1) {
			*this = v[0];
			return;
		}
		clear();
		for (unsigned int i=0;i<v.size();i++) {
			avg += v[i].avg*v[i].count;
			count += v[i].count;
		}
		if (!count) return;
		avg = avg/count;

		std::sort(v.begin(),v.end(),[](const aggregated_counter &a,const aggregated_counter &b)->bool {
			return a.perc50*a.count < b.perc50*b.count;
		});
		perc50 = v[(v.size()*50)/100].perc50;

		std::sort(v.begin(),v.end(),[](const aggregated_counter &a,const aggregated_counter &b)->bool {
			return a.perc80*a.count < b.perc80*b.count;
		});
		perc80 = v[(v.size()*50)/100].perc80;

		std::sort(v.begin(),v.end(),[](const aggregated_counter &a,const aggregated_counter &b)->bool {
			return a.perc95*a.count < b.perc95*b.count;
		});
		perc95 = v[(v.size()*50)/100].perc95;

		std::sort(v.begin(),v.end(),[](const aggregated_counter &a,const aggregated_counter &b)->bool {
			return a.perc99*a.count < b.perc99*b.count;
		});
		perc99 = v[(v.size()*50)/100].perc99;
	}

	void aggregate(counter &c) {
		count = c.data.size();
		if (!count) {
			clear();
			return;
		}
		unsigned long long int step = count>70000? std::min(count/35000,50LL) : 1;
		int perccount = std::max(1ULL,c.data.size()/step);

		std::sort(c.data.begin(),c.data.begin()+perccount);
		perc50 = c.data[(perccount*50)/100];
		perc80 = c.data[(perccount*80)/100];
		perc95 = c.data[(perccount*95)/100];
		perc99 = c.data[(perccount*99)/100];

		long long int sum = 0;
		for (auto i=0;i<count;i++) sum += c.data[i];
		avg = sum/count;
	}
	void clear() {
		//memset(this,0,sizeof(*this));
		avg = 0;
		count = 0;
		perc50 = 0;
		perc80 = 0;
		perc95 = 0;
		perc99 = 0;

	}
};

typedef aj::object<
	aggregated_counter,
	ap::type_list_n<
		aj::member<n_avg, aggregated_counter, long long int, &aggregated_counter::avg>
		,aj::member<n_count, aggregated_counter, long long int, &aggregated_counter::count>
		,aj::member<n_perc50, aggregated_counter, long long int, &aggregated_counter::perc50>
		,aj::member<n_perc80, aggregated_counter, long long int, &aggregated_counter::perc80>
		,aj::member<n_perc95, aggregated_counter, long long int, &aggregated_counter::perc95>
		,aj::member<n_perc99, aggregated_counter, long long int, &aggregated_counter::perc99>
	>::type
> aggregated_counter_json;

