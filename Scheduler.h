#pragma once

#include "Runtime.h"
#include "spdlog/spdlog.h"

enum class SchedulerAlgorithm
{
	RoundRobin = 0
};

class Scheduler
{
public:
	Scheduler(Runtime& p_runtime, 
		      std::shared_ptr<spdlog::logger> p_logger,
		      SchedulerAlgorithm p_algo = SchedulerAlgorithm::RoundRobin);
	virtual ~Scheduler();

protected:
	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;

private:
	SchedulerAlgorithm m_algo;
};

