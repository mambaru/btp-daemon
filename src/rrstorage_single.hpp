#pragma once

/**
 * хранилище типа hashmap, key: int[N], value: rrd-массив из aggregated_counter
 */
template<int N>
struct RoundRobinStorage {
	typedef intkey<N> key_type;

	kc::HashDB *db_data;
	BufferedKC<kc::HashDB> *bdb_data;
	kc::HashDB *db_meta;
	BufferedKC<kc::HashDB> *bdb_meta;
	int scale_ts;
	int count;
	std::unordered_map<key_type, aggregated_counter> last_data;
	bool do_save_last_data;

	void init(int _scale_ts,int _count, bool _do_save_last_data) {
		scale_ts = _scale_ts;
		count = _count;
		do_save_last_data = _do_save_last_data;
	}
	void sync() {
		bdb_meta->sync();
		bdb_data->sync();
	}
	void close() {
		db_meta->close();
		db_data->close();
		delete db_meta;
		delete db_data;
		db_meta = 0;
		db_data = 0;
	}
	void open(std::string name,int tune = 1024) {
		db_data = new kc::HashDB();
		bdb_data = new BufferedKC<kc::HashDB>(db_data);
		db_data->tune_buckets(300000);
		db_data->tune_map(512*tune*1024);
		db_data->open(name+"_data.kch", kyotocabinet::HashDB::OREADER | kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);
		db_meta = new kc::HashDB();
		bdb_meta = new BufferedKC<kc::HashDB>(db_meta);
		db_meta->tune_buckets(300000);
		db_meta->tune_map(16*tune*1024);
		db_meta->open(name+"_meta.kch", kyotocabinet::HashDB::OREADER | kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);

		if (do_save_last_data) {
			auto cb = [this](datameta_pair &p,const key_type &key){
				this->last_data[key] = p.data[(p.meta->ind-2+p.meta->count)%p.meta->count];
			};
			for_each(cb);
		}
		std::cout << "loaded " << name << std::endl;
	}
	template<int ord>
	void remove(int key_ord) {
		kyotocabinet::HashDB::Cursor *cur = db_meta->cursor();
		cur->jump();
		std::string ckey;
		while (cur->get_key(&ckey, true)) {
			int *val = (int*)ckey.data();
			if (val[ord]==key_ord) {
				db_meta->remove(ckey);
				db_data->remove(ckey);
			}
		}
		delete cur;
	}

	template<typename T> void for_each(T &what) {
		//foreach in storage5s
		int ts = time(0);
		kyotocabinet::HashDB::Cursor *cur = db_meta->cursor();
		cur->jump();
		std::string ckey;
		key_type key;
		while (cur->get_key(&ckey, true)) {
			int *val = (int*)ckey.data();
			for (int i=0;i<N;i++) key.data[i] = val[i];
			datameta_pair p = get<true>(key,ts);
			what(p,key);
			p.free();
		}
		delete cur;
	}
	template<bool fill_empty>
	datameta_pair get(const key_type &key, int ts) {
		const char *kstr = (const char*)&key.data[0];
		size_t ksz = N*sizeof(key.data[0]);
		aggregated_counter* dataval;
		meta_t *metaval;
		size_t metasz;
		size_t datasz;

		metaval = (meta_t*)bdb_meta->get(kstr,ksz,&metasz);
		if (metasz != sizeof(meta_t)) {
			metaval = new meta_t[1];
			metaval->ind = 0;
			metaval->ts = 0;
			metaval->count = count;
			metaval->version = 1;
		}

		dataval = (aggregated_counter*)bdb_data->get(kstr,ksz,&datasz);
		if (datasz!=sizeof(aggregated_counter)*count) {
			if (!fill_empty) return datameta_pair{NULL,metaval};

			dataval = new aggregated_counter[count];
		}


		int old_ts = metaval->ts/scale_ts;
		int new_ts = ts/scale_ts;

		if (new_ts-old_ts>=count) {
			metaval->ind = 0;
			metaval->ts = new_ts*scale_ts;
			for (int i=0;i<count;i++) dataval[i].clear();
		} else if (new_ts!=old_ts){
			int ind = metaval->ind+(new_ts-old_ts);
			if (ind>=count) {
				//очищаем от старого индекса до конца
				for (int i=metaval->ind+1;i<count;i++) dataval[i].clear();
				ind = ind%count;
				metaval->ind=-1;	//чтобы 0ой тоже очистился
			}
			for (int i=metaval->ind+1;i<=ind;i++) dataval[i].clear();
			metaval->ind = ind;
			metaval->ts = new_ts*scale_ts;
		}

		return datameta_pair{dataval,metaval};
	}

	void save(const key_type &key, int ts, const aggregated_counter &counter) {
		if (counter.count==0) return;
		datameta_pair datameta = get<true>(key,ts);
		datameta.data[datameta.meta->ind] = counter;

		const char *kstr = (const char*)&key.data[0];
		size_t ksz = N*sizeof(key.data[0]);
		bdb_meta->set(kstr,ksz,(char*)datameta.meta,sizeof(meta_t));
		bdb_data->set(kstr,ksz,(char*)datameta.data,sizeof(aggregated_counter)*count);

		datameta.free();
	}

	template<typename T>
	void save_aggregated(const T &t, time_t ts, bool res) {
		if (!res) return;
		for (auto it = t.begin();it!=t.end();it++) {
			this->save(it->first,ts,it->second);
			if (do_save_last_data) last_data[it->first] = it->second;
		}
	}
};

