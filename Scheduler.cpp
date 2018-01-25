#include "Scheduler.h"



Scheduler::Scheduler(Runtime& p_runtime,
					 std::shared_ptr<spdlog::logger> p_logger,
					 SchedulerAlgorithm p_algo)
	: m_logger(p_logger),
	  m_runtime(p_runtime),
	  m_algo(p_algo)
{
}


Scheduler::~Scheduler()
{
}
