#pragma once

#include <atomic>

/**
 * спинлок на атомиках, совместимый с std. можно делать std::lock_guard<spinlock> (lck);
 */
class spinlock {
	std::atomic<bool> value;
public:
    spinlock() : value(false) {
    }

    spinlock(const spinlock&) = delete;
    spinlock& operator=(const spinlock&) = delete;

    void lock() {
    	  while (value.exchange(true)) { };  // now we have the lock
    }

    bool try_lock() {
    	return !value.exchange(true);
    }

    void unlock() {
    	value = false; // release lock
    }
};

class longspinlock : public spinlock {
}  __attribute__ ((aligned (32)));

/**
 * спинлок на атомиках, частично совместимый с std. можно делать std::lock_guard<spinlock> (lck) - локгард для записи
 * lock/unlock - запись (только один писатель)
 * lock_r/unlock_r - чтение (много читателей)
 */
class multispinlock {
	std::atomic<int> rvalue;
	std::atomic<bool> wvalue;
public:
	multispinlock() : rvalue(0),wvalue(false) {
    }

	multispinlock(const multispinlock&) = delete;
    multispinlock& operator=(const multispinlock&) = delete;

    void lock() {
    	while (wvalue.exchange(true)) { };
    	while (rvalue.load());
    }
    void unlock() {
    	wvalue = false;
    }

    void lock_r() {
    	while (wvalue.load()) { };
    	rvalue++;
    }
    void unlock_r() {
    	rvalue--;
    }
};

class readlock {
	multispinlock *lck;
	readlock(multispinlock *lck):lck(lck){
	}
	void lock() { lck->lock_r();}
	void unlock() { lck->unlock_r();}
};
