#include "eNodeB.h"
#include "RadioMessages.h"

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
	m_ulScheduler = std::make_shared<UlScheduler>(m_runtime, m_logger, m_prbNumber, m_PDCCH, m_id, p_algo, p_mode);
	m_logger->info("Uplink Scheduler created");
	return m_ulScheduler;
}

void eNodeB::attachUe(uint64_t p_ueId, Runtime::TimePoint p_tm)
{
	m_logger->info("tti {}: UE[{}] attached", m_runtime.curTti(p_tm), p_ueId);

	auto l_ue = m_connectedUes[p_ueId];
	if (m_ulScheduler->getMode() == UlSchedulerMode::DynamicScheduling)
	{
		uint32_t l_srPeriodicity = static_cast<uint32_t>(m_ulScheduler->getSrPeriodicity());
		l_ue->configureSr(static_cast<uint32_t>(l_srPeriodicity), (m_runtime.curTti(p_tm) + 1) % l_srPeriodicity);
	}

	m_PDCCH->addReciever(l_ue.get());
	m_PHICH->addReciever(l_ue.get());

	l_ue->setPUCCH(m_PUCCH);
	l_ue->setPUSCH(m_PUSCH);

	m_ulScheduler->addUe(p_ueId);

	l_ue->setENodeBId(getId());
	l_ue->attached(p_tm);
}

void eNodeB::schedule(Runtime::TimePoint p_tm)
{
	m_logger->debug("eNodeB::schedule for tti {}", m_runtime.curTti(p_tm) + 1);
	if (!m_ulScheduler)
	{
		throw std::runtime_error("Uplink Scheduler not created!");
	}

	m_ulScheduler->schedule();

	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm) + 1ms - 3ns, std::bind(&eNodeB::schedule, this, std::placeholders::_1), this);
}

void eNodeB::recieve(std::shared_ptr<RadioMessage> p_msg, Runtime::TimePoint p_tm)
{
	m_logger->debug("tti {}: Message recieved from UE[{}], crc = {}",
		m_runtime.curTti(p_tm), p_msg->senderId, p_msg->crcValid);

	if (p_msg->type == RadioMessageType::SchedulingRequest)
	{
		processSchedulingRequest(std::static_pointer_cast<SchedulingRequestMessage>(p_msg), p_tm);
	}

	if (p_msg->type == RadioMessageType::UplinkData)
	{
		processUplinkData(std::static_pointer_cast<UplinkDataMessage>(p_msg), p_tm);
	}
}

void eNodeB::processSchedulingRequest(std::shared_ptr<SchedulingRequestMessage> p_msg, Runtime::TimePoint p_tm)
{
	m_logger->debug("tti {}: Process Scheduling request", m_runtime.curTti(p_tm));
	m_runtime.schedule(m_runtime.curTtiPoint(p_tm) + 3ms, std::bind(&UlScheduler::processSr, m_ulScheduler.get(), p_msg->senderId, std::placeholders::_1), this);
}

void eNodeB::processUplinkData(std::shared_ptr<UplinkDataMessage> p_msg, Runtime::TimePoint p_tm)
{
	m_logger->debug("tti {}: Process uplink data, crc = {}", m_runtime.curTti(p_tm), p_msg->crcValid);

	bool isNack = false;

	if (!p_msg->crcValid)
	{
		isNack = true;
	}
	else
	{
		m_ulScheduler->processAck(p_msg->senderId, p_tm);

		if (p_msg->bsr != 0)
		{
			m_runtime.schedule(m_runtime.curTtiPoint(p_tm) + 2ms,
				std::bind(&UlScheduler::processBsr, m_ulScheduler.get(), p_msg->senderId, p_msg->bsr, std::placeholders::_1), this);
		}
	}

	m_runtime.schedule(m_runtime.curTtiPoint(p_tm) + 4ms,
		std::bind(&eNodeB::sendAcknowledgement, this, isNack, p_msg->senderId, std::placeholders::_1), this);
}

void eNodeB::sendAcknowledgement(bool p_isNegative, uint64_t p_ueId, Runtime::TimePoint p_tm)
{
	m_logger->debug("tti {}: Sending Acknowledgement", m_runtime.curTti(p_tm));

	if (!p_isNegative)
	{
		std::shared_ptr<HarqAckMessage> l_ack =
			std::make_shared<HarqAckMessage>(p_ueId, m_id);

		m_PHICH->send(l_ack);
	}
	else
	{
		std::shared_ptr<HarqNackMessage> l_nack =
			std::make_shared<HarqNackMessage>(p_ueId, m_id);

		m_PHICH->send(l_nack);
	}
}
