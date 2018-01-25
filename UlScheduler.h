#pragma once
#include "Scheduler.h"
#include <chrono>

enum class UlSchedulerMode
{
	SchedulingRequest = 0,
	PersistentScheduling,
	SemiPersistentScheduling
};

enum class SrPeriodicity
{
	_5ms = 5,
	_10ms = 10,
	_20ms = 20,
	_40ms = 40,
	_80ms = 80,
	_2ms = 2,
	_1ms = 1,
	none = 0
};

class UlScheduler :	public Scheduler
{
public:
	UlScheduler(Runtime& p_runtime,
				std::shared_ptr<spdlog::logger> p_logger,
		        SchedulerAlgorithm p_algo = SchedulerAlgorithm::RoundRobin,
		        UlSchedulerMode p_mode = UlSchedulerMode::SchedulingRequest);
	~UlScheduler();

	UlSchedulerMode getMode() const
	{
		return m_mode;
	}

	void setSrPeriodicity(SrPeriodicity p_srPeriodocity);
	SrPeriodicity getSrPeriodicity() const
	{
		return m_srPeriodocity;
	}

private:
	UlSchedulerMode m_mode;

	SrPeriodicity m_srPeriodocity = SrPeriodicity::none;
};

