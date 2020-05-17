#pragma once
#include<thread>
#include<mutex>
#include<atomic>
#include<deque>
#include<condition_variable>
namespace YAL::threading {
	static int getThreadID() {
		static_assert(sizeof(std::thread::id) == 4);
		return *(int*)&std::this_thread::get_id();
	}
	template<typename T>
	class MQueue {
		std::deque<T> inner;
		std::mutex mutex;
		std::condition_variable not_empty;
	public:
		T pop() {
			std::unique_lock<std::mutex> locker(mutex);
			while (inner.empty()) not_empty.wait(locker);
			T rv(std::move(inner.front()));
			inner.pop_front();
			locker.unlock();
			return rv;
		}
		void push(T&& x) {
			std::unique_lock<std::mutex> locker(mutex);
			inner.emplace_back(std::forward<T>(x));
			locker.unlock();
			not_empty.notify_one();
		}
	};
	class simpleLock {
		std::atomic_flag locked = {};
	public:
		inline void lock() {
			while (locked.test_and_set()) std::this_thread::yield();
		}
		inline void unlock() {
			locked.clear();
		}
	};
	class reentrantLock {
		std::atomic_flag locked = {};
	public:
		int locker_tid;
		inline bool try_lock() {
			bool rv = locked.test_and_set();
			if (!rv) {
				locker_tid = getThreadID();
				return true;
			}
			return false;
		}
		inline void force_lock() {
			while (locked.test_and_set()) std::this_thread::yield();
			locker_tid = getThreadID();
		}
		inline void unlock() {
			locker_tid = 0;
			locked.clear();
		}
	};
	template<typename T>
	class LockGuardImpl;
	template<>
	class LockGuardImpl<class simpleLock> {
		simpleLock& lck;
	public:
		LockGuardImpl(simpleLock& lck2) :lck(lck2) { lck.lock(); };
		~LockGuardImpl() { lck.unlock(); }
	};
	template<>
	class LockGuardImpl<class reentrantLock> {
		reentrantLock& lck;
		bool locked;
	public:
		LockGuardImpl(reentrantLock& lck2) :lck(lck2) {
			if (lck.try_lock()) { //fast path:most of the time,nobody helds the lock
				locked = true;
				return; 
			}
			if (lck.locker_tid == getThreadID()) {
				locked = false; //reentrant 
				return;
			}
			lck.force_lock();
			locked = true;
		};
		~LockGuardImpl() { if(locked) lck.unlock(); }
	};
	template<typename T>
	class LockGuard :LockGuardImpl<T> {
	public:
		LockGuard(T& x) :LockGuardImpl<T>(x) {}; //deduction guide
	};
};