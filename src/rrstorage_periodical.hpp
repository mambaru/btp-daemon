#pragma once

/**
 * хранилище для счётчиков, обертка над несколькими RoundRobinStorage с разбивкой по временным периодам
 */
template<int N>
struct RoundRobinPeriodicalStorage {
	spinlock mtx;
	std::string name;
	typedef intkey<N> key_type;
	typedef std::unordered_map<key_type, counter > map_type;
	typedef std::unordered_map<key_type, aggregated_counter> aggregated_type;

	RoundRobinStorage<N> storage5s,storage1m,storage30m,storage6h;

	map_type *current_data;
	std::vector<map_type *>second_data;
	int second_current;

	static const int second_data_size = 300;

	void sync() {
		storage5s.sync();
		storage1m.sync();
		storage30m.sync();
		storage6h.sync();
	}
	std::thread *longaggregation;
	void stop() {
		if (longaggregation!=0) {
			longaggregation->join();
			delete longaggregation;
			longaggregation = 0;
		}
		sync();
		storage5s.close();
		storage1m.close();
		storage30m.close();
		storage6h.close();
	}

	template<int ord>
	void remove(int key_ord) {
		storage5s.remove<ord>(key_ord);
		storage1m.remove<ord>(key_ord);
		storage30m.remove<ord>(key_ord);
		storage6h.remove<ord>(key_ord);
	}
	RoundRobinPeriodicalStorage(std::string path, std::string name, int tune = 1024) : name(name) {
		current_data = new map_type();
		second_data.resize(second_data_size, NULL);

		storage5s.init(5,3000,false);
		storage1m.init(60,3000,false);
		storage30m.init(30*60,3000,true);
		storage6h.init(6*3600,3000,false);

		storage5s.open(path + name+"_5s", tune);
		storage1m.open(path + name+"_1m", tune);
		storage30m.open(path + name+"_30m", tune);
		storage6h.open(path + name+"_6h", tune);

		std::cout << "loaded intkeymap<" << N << ">: " << name << std::endl;
	}

	RoundRobinStorage<N> &get_storage(int scale) {
		if (scale <=storage1m.scale_ts) {
			if (scale<=storage5s.scale_ts) return storage5s; else return storage1m;
		} else {
			if (scale<=storage30m.scale_ts) return storage30m; else return storage6h;
		}
	}

	aggregated_type run_aggregation(time_t ts) {
		aggregated_type ret;
		{
			bool res = false;
			ret = this->aggregate_sub(res,storage5s.scale_ts, ts);
			if (res) storage5s.save_aggregated(ret,ts,res);
		}
		bool res1m = false;
		{
			aggregated_type t = this->aggregate_sub(res1m,storage1m.scale_ts, ts);
			if (res1m) storage1m.save_aggregated(t,ts,res1m);
		}
		if (res1m) {
			if (longaggregation) {
				longaggregation->join();
				delete longaggregation;
			}
			longaggregation = new std::thread([this,ts](){
				{
					bool res = false;
					aggregated_type t = this->aggregate_sup(res,this->storage30m.scale_ts, ts);
					if (res) this->storage30m.save_aggregated(t,ts,res);
				}
				{
					bool res = false;
					aggregated_type t = this->aggregate_sup(res,this->storage6h.scale_ts, ts);
					if (res) this->storage6h.save_aggregated(t,ts,res);
				}
			});
		}
		return ret;
	}

	map_type* roll(int ts) {
		map_type* ptr = new map_type();
		{
//			std::lock_guard<spinlock> lck(mtx);
			std::swap(ptr,current_data);
		}

		second_current = (ts)%second_data_size;
		std::swap(second_data[second_current],ptr);
		return ptr;
	}

	/**
	 * берёт часть статистики с детализацией до запроса и агрегирует её
	 */
	aggregated_type aggregate_sub(bool &res, int period_sec, int ts) {
		aggregated_type result;
		assert((second_data_size%period_sec)==0);
		if ((ts+1)%period_sec) { res = false; return result;}
		res = true;

		map_type tmp;
		int second_ts = (ts)%second_data_size;
		int second_ind = (ts+second_data_size+1-period_sec)%second_data_size;
		assert(second_ind < (ts%second_data_size));

		for (int i=second_ind;i<=second_ts;i++) if (second_data[i]) {
			for (auto it = second_data[i]->begin();it!=second_data[i]->end();it++) {
				tmp[it->first].add(it->second);
			}
		}

		for (auto it = tmp.begin();it!=tmp.end();it++) {
			result[it->first].aggregate(it->second);
		}
		return result;
	}

	/**
	 * берёт агрегированную статистику и агрегирует её в более крупные куски
	 */
	aggregated_type aggregate_sup(bool &res, int period_sec, int ts) {
		aggregated_type result;
		assert((period_sec%60)==0);
		if ((ts+1)%period_sec) { res = false; return result;}
		res = true;

		int agr_count = period_sec/ (period_sec <= storage30m.scale_ts ? storage5s.scale_ts : storage1m.scale_ts);

		auto cb = [&result,agr_count,this](datameta_pair &p,const key_type &key)->void{
			std::vector<aggregated_counter> out;
			out.reserve(agr_count);
			int start;
			if (p.meta->ind+1<agr_count) {
				for (int i=p.meta->count + (p.meta->ind+1) - agr_count;i<p.meta->count;i++) if (p.data[i].count>0) out.push_back(p.data[i]);
				start = 0;
			} else {
				start = (p.meta->ind+1)-agr_count;
			}
			for (int i=start;i<=p.meta->ind;i++) if (p.data[i].count>0) out.push_back(p.data[i]);
			aggregated_counter val;
			val.supaggregate(out);
			result[key] = val;
		};
		if (period_sec <= storage30m.scale_ts) {
			storage5s.for_each(cb);
		} else {
			storage1m.for_each(cb);
		}
		return result;
	}


	map_type* get_last() {
		//if (second_current<0) return 0;
		return second_data[second_current];
	}

	counter& get_current(key_type& key) {
		std::lock_guard<spinlock> lck(mtx);
		return (*current_data)[key];
	}
	map_type* get_current() {
		return current_data;
	}
	counter& get_current(key_type& key, map_type* curr) {
		std::lock_guard<spinlock> lck(mtx);
		return (*curr)[key];
	}

};

