#pragma once

#include "kc_setbulk.hpp"

/**
 * буферизирующая обёртка над kyoto tycoon
 * снижает нагрузку на IO-подсистему
 */
template<typename DB>
struct BufferedKC {
	std::unordered_map<std::string,std::string> data1;
	std::unordered_map<std::string,std::string> data2;
	int ts;
	multispinlock mtx;
	DB *db;
	BufferedKC(DB *db) : db(db) {
		sync();
	}

	void _sync(const std::unordered_map<std::string,std::string> &data) {
		ts = time(0);
		kc::HashDB t;
		//t.set_bulk()
		set_bulk(db,data);
		//db->set_bulk(data);
		/*for (auto it = data.begin();it!=data.end();it++) {
			db->set(it->first,it->second);
		}*/
	}
	bool set(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
		std::string s_key (kbuf,ksiz);
		std::string s_val (vbuf,vsiz);
		{
			std::lock_guard<multispinlock> lck(mtx);
			data1[s_key] = s_val;
		}
		//if (time(0)-ts>30) { _sync(data); data.clear(); }
		return true;
	}
	char* get(const char* kbuf, size_t ksiz, size_t* sp) {
		std::string s_key (kbuf,ksiz);
		std::string val;

		mtx.lock_r();
		if (data1.count(s_key)) {
			val = data1[s_key];
			mtx.unlock_r();
		} else if (data2.count(s_key)) {
			val = data2[s_key];
			mtx.unlock_r();
		} else {
			mtx.unlock_r();
			return db->get(kbuf,ksiz,sp);
		}
		//найдено в data1 или data2
		*sp = val.size();
		char *ret = new char[val.size()+1];
		memcpy(&ret[0],val.c_str(),val.size());
		ret[val.size()] = 0;
		return ret;
	}
	void sync() {
		std::unordered_map<std::string,std::string> olddata;
		{
			std::lock_guard<multispinlock> lck(mtx);
			std::swap(olddata, data2);	//clear data2
			std::swap(data1, data2);	//move data1 to data2, clear data1
		}
		olddata = data2;	//копия data2
		_sync(olddata);
		olddata.clear();
		{
			std::lock_guard<multispinlock> lck(mtx);
			std::swap(olddata, data2);	//clear data2
		}
	}

};
