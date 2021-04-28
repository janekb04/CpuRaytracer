#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <vector>
#include <thread>
#include <barrier>
#include "duplex.h"
#include "holder_or_void.h"

template <typename CRTP>
class scheduler
{
	struct sync_func
	{
		scheduler& sch;
		
		void operator()() const noexcept
		{
			sch.render_main_sync.release();
			static_cast<CRTP*>(&sch)->worker_sync();
			sch.render_main_sync.acquire();
		}
	};
	
	std::vector<std::thread> workers;
	bool should_run;
	duplex render_main_sync;
	std::barrier<sync_func> sync;

	void worker(size_t idx)
	{
		holder_or_void init_data{ &CRTP::worker_init, static_cast<CRTP*>(this), idx };
		while (should_run) {
			if (enable_synchronization)
			{
				sync.arrive_and_wait();
			}
			init_data.invoke(&CRTP::worker_run, static_cast<CRTP*>(this), idx);
		}
	}

	[[nodiscard]] constexpr bool main_run() const noexcept { return false; }
	constexpr void worker_init(size_t worker_idx) const noexcept {}
	constexpr void worker_run(size_t worker_idx) const noexcept {}
	constexpr void worker_sync() const noexcept {}
public:
	scheduler(size_t worker_count = std::thread::hardware_concurrency()) :
		workers{ worker_count },
		should_run{ true },
		sync{ static_cast<ptrdiff_t>(worker_count), { *this } },
		enable_synchronization{true}
	{
	}

	void run()
	{
		should_run = true;
		for (int i = 0; i < worker_count(); ++i) {
			workers[i] = std::thread{ &scheduler::worker, this, i };
		}
		while (should_run)
		{
			if(enable_synchronization)
			{
				std::unique_lock lk{ render_main_sync };
				should_run = static_cast<CRTP*>(this)->main_run();
			}
			else
			{
				should_run = static_cast<CRTP*>(this)->main_run();
			}
		}
		for (auto& worker : workers) {
			worker.join();
		}
		workers.clear();
	}
protected:
	std::atomic<bool> enable_synchronization;
	[[nodiscard]] size_t worker_count() const noexcept
	{
		return workers.size();
	}
};
#endif // SCHEDULER_H
