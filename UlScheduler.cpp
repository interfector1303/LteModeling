#include "UlScheduler.h"



UlScheduler::UlScheduler(Runtime& p_runtime,
	                     std::shared_ptr<spdlog::logger> p_logger,
	                     SchedulerAlgorithm p_algo,
	                     UlSchedulerMode p_mode)
	: Scheduler(p_runtime, p_logger, p_algo),
	  m_mode(p_mode)
{
}


UlScheduler::~UlScheduler()
{
}

void UlScheduler::setSrPeriodicity(SrPeriodicity p_srPeriodocity)
{
	if (m_mode != UlSchedulerMode::SchedulingRequest)
	{
		m_logger->warn("Current Scheduling mode not required SR periodicity parameter!");
	}

	m_srPeriodocity = p_srPeriodocity;
}
