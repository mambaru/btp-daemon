#pragma once

int64_t set_bulk(kyotocabinet::BasicDB *db, const std::unordered_map<std::string, std::string>& recs) {
	std::vector<std::string> keys;
	keys.reserve(recs.size());
	auto rit = recs.begin();
	auto ritend = recs.end();
	while (rit != ritend) {
		keys.push_back(rit->first);
		++rit;
	}
	class VisitorImpl : public kyotocabinet::DB::Visitor {
	public:
		explicit VisitorImpl(const std::unordered_map<std::string, std::string>& recs) : recs_(recs) {}
	private:
		const char* visit_full(const char* kbuf, size_t ksiz,
				const char* vbuf, size_t vsiz, size_t* sp) {
			auto rit = recs_.find(std::string(kbuf, ksiz));
			if (rit == recs_.end()) return kyotocabinet::DB::Visitor::NOP;
			*sp = rit->second.size();
			return rit->second.data();
		}
		const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
			auto rit = recs_.find(std::string(kbuf, ksiz));
			if (rit == recs_.end()) return kyotocabinet::DB::Visitor::NOP;
			*sp = rit->second.size();
			return rit->second.data();
		}
		const std::unordered_map<std::string, std::string>& recs_;
	};
	VisitorImpl visitor(recs);
	if (!db->accept_bulk(keys, &visitor, true)) return -1;
	return keys.size();
}
