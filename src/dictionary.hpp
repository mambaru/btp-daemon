#pragma once


/**
 * словарь string => int
 */
struct dictionary {
	multispinlock lck;
	std::unordered_map<std::string, int> data;
	//tbb::concurrent_vector<std::string> rev;
	std::vector<std::string> rev;
	kc::HashDB *db;

	dictionary (std::string path,std::string name) {
		db = new kc::HashDB();
		db->open(path + name+".kch", kyotocabinet::HashDB::OREADER | kyotocabinet::HashDB::OAUTOSYNC | kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);
		kyotocabinet::HashDB::Cursor *cur = db->cursor();
		cur->jump();
		std::string ckey, cvalue;
		int maxval = 0;
		while (cur->get(&ckey, &cvalue, true)) {
			int *val = (int*)cvalue.data();
			data[ckey] = *val;
			maxval = std::max(maxval, *val);
		}
		delete cur;

		rev.resize(maxval+1);
		rev.reserve(maxval + (maxval>>1) + 100);
		for (auto it=data.begin();it!=data.end();it++) rev[it->second] = it->first;

		std::cout << "loaded dictionary " << name << std::endl;
	}
	~dictionary() {
		db->close();
	}
	int get(const std::string &val, bool allow_add = true) {
		lck.lock_r();
		{
			auto it = data.find(val);
			if (it!=data.end()) {int res = it->second; lck.unlock_r(); return res;}
		}
		lck.unlock_r();
		if (!allow_add) return -1;
		lck.lock();
		{
			auto it = data.find(val);
			if (it!=data.end()) {
				int res = it->second;
				lck.unlock();
				return res;
			}
		}
		int r;
		if (rev.size()>data.size()) {
			for (unsigned int i=1;i<rev.size();i++) {
				if (!rev[i].length()) {
					r = i;
					rev[r] = val;
					data[val] = i;
					db->set(val.c_str(),val.size(),(char*)&r,sizeof(r));
					lck.unlock();
					return r;
				}
			}
		}

		r = rev.size();
		data[val] = r;
		rev.push_back(val);
		db->set(val.c_str(),val.size(),(char*)&r,sizeof(r));
		lck.unlock();
		return r;
	}
	std::string get_name(int i) {
		lck.lock_r();
		if (i<=0 || i>=(int)rev.size()) {
			lck.unlock_r();
			return "";
		}
		std::string ret = rev[i];
		lck.unlock_r();
		return ret;
	}
	int remove(const std::string &val) {
		std::lock_guard<multispinlock> _g(lck);
		auto it = data.find(val);
		if (it==data.end()) return -1;
		int v = it->second;
		if (v==-1) return -1;
		rev[v] = "";
		data[val] = -1;
		db->remove(val.c_str(),val.size());
		return v;
	}
};

