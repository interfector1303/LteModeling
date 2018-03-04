#include "UlScheduler.h"



UlScheduler::UlScheduler(Runtime& p_runtime,
	                     std::shared_ptr<spdlog::logger> p_logger,
	                     size_t p_prbNumber,
	                     std::shared_ptr<PDCCH> p_PDCCH,
					     uint64_t p_eNodeBId,
	                     SchedulerAlgorithm p_algo,
	                     UlSchedulerMode p_mode)
	: Scheduler(p_runtime, p_logger, p_algo),
	  m_prbNumber(p_prbNumber),
	  m_PDCCH(p_PDCCH),
	  m_eNodeBId(p_eNodeBId),
	  m_mode(p_mode)
{
	m_runtime.schedule(m_runtime.nextTtiPointRelNow(),
		std::bind(&UlScheduler::sendDownlinkControlInformation, this, std::placeholders::_1), this);
}


UlScheduler::~UlScheduler()
{
}

void UlScheduler::setSrPeriodicity(SrPeriodicity p_srPeriodocity)
{
	if (m_mode != UlSchedulerMode::DynamicScheduling)
	{
		m_logger->warn("Current Scheduling mode not required SR periodicity parameter!");
	}

	m_srPeriodocity = p_srPeriodocity;
}

void UlScheduler::schedule()
{
	size_t l_prbCapacity = 32;

	size_t l_harqProcess = (m_runtime.curTtiRelNow() + 4) % 8;

	m_logger->debug("schedule for harq process {}", l_harqProcess);

	if (m_ueContexts.empty())
	{
		return;
	}

	for (auto& l_ueCtx : m_ueContexts)
	{
		l_ueCtx.second.prbAllocated = 0;
	}

	auto& l_ueIt = m_ueContexts.begin();
	size_t i = 0;
	size_t l_lastUsedRes = i;

	for (i; i < 2 * m_prbNumber; ++i)
	{
		auto& l_ueCtx = l_ueIt->second;

		if (l_ueCtx.schedulingRequest)
		{
			m_logger->debug("UlScheduler: process SR for UE[{}]", l_ueIt->first);
			l_ueCtx.prbAllocated++;
			l_ueCtx.schedulingRequest = false;
			l_lastUsedRes = i;
		}

		if (l_ueCtx.harqProcessNotAcked[l_harqProcess] != 0)
		{
			l_ueCtx.prbAllocated++;
			l_ueCtx.harqProcessNotAcked[l_harqProcess] -=
				std::min(l_prbCapacity, l_ueCtx.harqProcessNotAcked[l_harqProcess]);
			l_ueCtx.isRetransmission = true;
			l_lastUsedRes = i;
		}

		// to next UE
		if (++l_ueIt == m_ueContexts.end())
		{
			l_ueIt = m_ueContexts.begin();
		}
	}

	l_ueIt = m_ueContexts.begin();

	for (i = l_lastUsedRes + 1; i < 2 * m_prbNumber; ++i)
	{
		auto& l_ueCtx = l_ueIt->second;

		if (l_ueCtx.bufferSize != 0 && !l_ueCtx.isRetransmission)
		{
			l_ueCtx.prbAllocated++;
			l_ueCtx.bufferSize -= std::min(l_prbCapacity, l_ueCtx.bufferSize);
		}

		// to next UE
		if (++l_ueIt == m_ueContexts.end())
		{
			l_ueIt = m_ueContexts.begin();
		}
	}

	size_t l_slot = 0;
	size_t l_prb = 0;

	for (auto& l_ueCtx : m_ueContexts)
	{
		std::shared_ptr<UplinkGrantMessage> l_grant =
			std::make_shared<UplinkGrantMessage>(l_ueCtx.first, m_eNodeBId, m_runtime.curTtiRelNow() + 4);

		while (l_ueCtx.second.prbAllocated)
		{
			Prb curPrb(l_prb, l_slot, l_prbCapacity);
			l_grant->radResource.prbs.push_back(curPrb);
			l_ueCtx.second.prbAllocated--;

			l_prb++;

			if (l_prb == m_prbNumber)
			{
				l_prb = 0;
				l_slot++;
			}
		}

		if (!l_grant->radResource.prbs.empty())
		{
			m_logger->debug("UE[{}], Harq [{}] not acked {}", l_ueCtx.first, l_harqProcess, l_grant->radResource.capacity());
			l_ueCtx.second.harqProcessNotAcked[l_harqProcess] = l_grant->radResource.capacity();
			m_dcis.push_back(l_grant);
		}

		l_ueCtx.second.isRetransmission = false;
	}
}
