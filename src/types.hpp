#pragma once

template<int N>
struct intkey {
	int data[N];

	void clear() {
		for (int i=0;i<N;i++) data[i] = 0;
	}
	bool operator==(const intkey<N> &other) const {
		for (int i=0;i<N;i++) if (data[i]!=other.data[i]) return false;
		return true;
	}
	bool operator<(const intkey<N> &other) const {
		for (int i=0;i<N;i++) if (data[i]<other.data[i]) return true;
		return false;
	}
};

namespace tbb {
template<int N>
class tbb_hash < intkey<N> >
{
public:
    tbb_hash() {}

    size_t operator()(const intkey<N>& t) const {
        size_t hash = 0;
        for (int i=0;i<N;i++) hash ^= tbb_hasher(t.data[i]);
        return hash;
    }
};

}


template<int N>
std::ostream& operator<<(std::ostream &out, const intkey<N> &val) {
	for (int i=0;i<N-1;i++) out << val.data[i] << " ";
	out << val.data[N-1];
	return out;
}

namespace std {
	template<int N> struct hash< intkey<N> > : public std::unary_function<intkey<N>, size_t> {
		size_t operator()(const intkey<N>& __s) const {
			size_t val = 14695981039346656037ULL;
			for (int i=0;i<N;i++) {
				val ^= static_cast<size_t>(__s.data[i]);
				val *= static_cast<size_t>(1099511628211ULL);
			}
			return val;
		}
	};
}



struct meta_t {
	int ts;
	int ind;
	int count;
	int version;

	int get_ts(int i) {
		return ts - count + (i-ind-1+count)%count;
	}
};
struct datameta_pair {
	aggregated_counter *data;
	meta_t* meta;

	aggregated_counter& get_by_tsdelta(int delta,int scale) {
		if (delta==0) return data[meta->ind];
		int dind = delta / scale;
		int ind = (meta->ind - dind + meta->count)%meta->count;
		return data[ind];
	}

	void free() {
		delete[] data;
		delete[] meta;
	}
};

