#include "eNodeB.h"

eNodeB::~eNodeB()
{
}

std::shared_ptr<UE> eNodeB::addUe(uint64_t p_tiiToAttach)
{
	auto l_newUe = m_connectedUes.emplace(std::make_pair(m_nextUeId, std::make_shared<UE>(m_nextUeId, m_runtime, m_logger)));
	m_logger->info("UE[{}] added, tti to attach: {}", m_nextUeId, p_tiiToAttach);
	m_runtime.schedule(m_runtime.timePointForTti(p_tiiToAttach), std::bind(&eNodeB::attachUe, this, m_nextUeId, std::placeholders::_1), this);
	m_nextUeId++;
	return l_newUe.first->second;
}

std::shared_ptr<UlScheduler> eNodeB::createUlScheduler(SchedulerAlgorithm p_algo,
													   UlSchedulerMode p_mode)
{
	ulScheduler = std::make_shared<UlScheduler>(m_runtime, m_logger, p_algo, p_mode);
	m_logger->info("Uplink Scheduler created");
	return ulScheduler;
}

void eNodeB::attachUe(uint64_t p_ueId, Runtime::TimePoint p_tm)
{
	m_logger->info("tti {}: UE[{}] attached", m_runtime.curTti(p_tm), p_ueId);

	auto l_ue = m_connectedUes[p_ueId];
	if (ulScheduler->getMode() == UlSchedulerMode::SchedulingRequest)
	{
		uint32_t l_srPeriodicity = static_cast<uint32_t>(ulScheduler->getSrPeriodicity());
		l_ue->configureSr(static_cast<uint32_t>(l_srPeriodicity), (m_runtime.curTti(p_tm) + 1) % l_srPeriodicity);
	}

	m_PDCCH->addReciever(l_ue.get());
	l_ue->setPUCCH(m_PUCCH);

	l_ue->attached(p_tm);
}

void eNodeB::schedule(Runtime::TimePoint p_tm)
{
	m_logger->debug("eNodeB::schedule for tti {}", m_runtime.curTti(p_tm) + 1);
	if (!ulScheduler)
	{
		throw std::runtime_error("Uplink Scheduler not created!");
	}
	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm) + 1ms - 2ns, std::bind(&eNodeB::schedule, this, std::placeholders::_1), this);
}
