#pragma once

struct bgthreads {
	std::mutex mtx;
	std::vector<std::thread *> data;

	void push(std::thread *thread) {
		std::lock_guard<std::mutex> lck(mtx);
		data.push_back(thread);
	}
	void stop() {
		clean(0);
	}
	void clean(unsigned int skip) {
		if (skip >= data.size()) return;
		std::lock_guard<std::mutex> lck(mtx);

		std::vector<std::thread *> newdata;
		for (unsigned int i=0;i<data.size()-skip;i++) {
			data[i]->join();
			delete data[i];
		}
		for (unsigned int i=data.size()-skip;i<data.size();i++) {
			newdata.push_back(data[i]);
		}
		std::swap(newdata,data);
	}


};
