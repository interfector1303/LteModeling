#pragma once

#include "Scheduler.h"
#include "PDCCH.h"

#include <chrono>

struct UlSchedulerUeContext
{
	UlSchedulerUeContext()
		: schedulingRequest(false),
		  prbAllocated(0),
		  bufferSize(0),
		  isRetransmission(false)
	{
		memset(harqProcessNotAcked, 0, sizeof(harqProcessNotAcked));
	}

	bool schedulingRequest;
	size_t prbAllocated;
	size_t bufferSize;

	bool isRetransmission;

	size_t harqProcessNotAcked[8];
};

enum class UlSchedulerMode
{
	DynamicScheduling = 0,
	PreScheduling,
	CbPuschScheduling
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
		        size_t p_prbNumber,
		        std::shared_ptr<PDCCH> p_PDCCH,
				uint64_t p_eNodeBId,
		        SchedulerAlgorithm p_algo = SchedulerAlgorithm::RoundRobin,
		        UlSchedulerMode p_mode = UlSchedulerMode::DynamicScheduling);
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

	void addUe(uint64_t p_id) override
	{
		m_ueContexts[p_id] = UlSchedulerUeContext();
	}

	void removeUe(uint64_t p_id) override
	{
		m_ueContexts.erase(p_id);
	}

	void schedule();

	void clearState() override
	{
		m_ueContexts.clear();
	}

	void processSr(uint64_t p_ueId, Runtime::TimePoint p_tm)
	{
		m_ueContexts[p_ueId].schedulingRequest = true;
	}

	void processBsr(uint64_t p_ueId, uint64_t p_bsr, Runtime::TimePoint p_tm)
	{
		m_ueContexts[p_ueId].bufferSize = p_bsr;
		m_logger->debug("Buffer status for UE[{}] updated: {}",
			p_ueId, m_ueContexts[p_ueId].bufferSize);
	}

	void processAck(uint64_t p_ueId, Runtime::TimePoint p_tm)
	{
		m_logger->debug("UlScheduler process ACK for UE[{}] for HARQ[{}]",
			p_ueId, m_runtime.curTti(p_tm) % 8);
		m_ueContexts[p_ueId].harqProcessNotAcked[m_runtime.curTti(p_tm) % 8] = 0;
	}

	void sendDownlinkControlInformation(Runtime::TimePoint p_tm)
	{
		m_logger->debug("Sending DCIs, amount = {}", m_dcis.size());

		for (auto& l_dci : m_dcis)
		{
			m_PDCCH->send(l_dci);
		}

		m_dcis.clear();

		m_runtime.schedule(m_runtime.nextTtiPoint(p_tm),
			std::bind(&UlScheduler::sendDownlinkControlInformation, this, std::placeholders::_1), this);
	}

private:
	UlSchedulerMode m_mode;

	SrPeriodicity m_srPeriodocity = SrPeriodicity::none;

	size_t m_prbNumber;

	std::map<uint64_t, UlSchedulerUeContext> m_ueContexts;

	std::shared_ptr<PDCCH> m_PDCCH;

	uint64_t m_eNodeBId;

	std::list<std::shared_ptr<UplinkGrantMessage>> m_dcis;
};

