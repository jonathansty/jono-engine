#include "core.pch.h"


namespace Tasks {
std::unique_ptr<enki::TaskScheduler> g_Scheduler;

enki::TaskScheduler* get_scheduler() {
	if(!g_Scheduler) {
		g_Scheduler = std::make_unique<enki::TaskScheduler>();
	}
	return g_Scheduler.get();
}

}
