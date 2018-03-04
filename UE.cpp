#include "UE.h"
#include "RadioMessages.h"
#include "StatsCollector.h"

UE::~UE()
{

}

void UE::attached(Runtime::TimePoint p_tm)
{
	m_trafficGenerator->setNextMessageArrivalTp(p_tm);
	auto l_msgData = m_trafficGenerator->getNextMsgData();
	m_runtime.schedule(l_msgData.arrivalTime, std::bind(&UE::getNewData, this, l_msgData, std::placeholders::_1), this);

	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm), std::bind(&UE::processTti, this, std::placeholders::_1), this);
}

void UE::processTti(Runtime::TimePoint p_tm)
{
	size_t m_harqProcess = m_runtime.curTti(p_tm) % 8;

	m_logger->debug("tti {}: UE[{}] process", m_runtime.curTti(p_tm), m_id);

	if (m_dataSize != 0 || !m_uplinkHarqBuffer[m_harqProcess].empty())
	{
		m_logger->debug("tti {}: UE[{}] has data for transsmition ({} new bytes, harq buffer not empty = {})",
			m_runtime.curTti(p_tm), m_id, m_dataSize, !m_uplinkHarqBuffer[m_harqProcess].empty());
		sendData(p_tm);
	}

	StatsCollector::getInstance().setBytesInHarqBuffersForENodeBUe(bytesInHarqBuffers(), m_eNodeBId, m_id);
	StatsCollector::getInstance().setBytesInBufferForENodeBUe(m_dataSize, m_eNodeBId, m_id);

	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm), std::bind(&UE::processTti, this, std::placeholders::_1), this);
}

void UE::getNewData(MsgData p_msgData, Runtime::TimePoint p_tm)
{
	if (m_dataSize == 0 && !hasResourcesForTransmission())
	{
		m_pendingSr = true;
	}

	m_dataBuffer.push_back(p_msgData);
	m_dataSize += p_msgData.size;

	StatsCollector::getInstance().newDataPacketForENodeBUe(p_msgData, m_eNodeBId, m_id);

	m_logger->debug("Ue[{}]: new data, buffer size = {}", m_id, m_dataSize);

	auto l_nextMsgData = m_trafficGenerator->getNextMsgData();
	m_runtime.schedule(l_nextMsgData.arrivalTime, std::bind(&UE::getNewData, this, l_nextMsgData, std::placeholders::_1), this);
}

void UE::sendData(Runtime::TimePoint p_tm)
{
	if (m_pendingSr)
	{
		m_logger->debug("tti {}: UE[{}] - no resources", m_runtime.curTti(p_tm), m_id);
		sendSr(p_tm);
		return;
	}
	else
	{
		size_t m_harqProcess = m_runtime.curTti(p_tm) % 8;
		m_logger->debug("Harq process {}", m_harqProcess);

		std::list<MsgData>* l_curBuffer;
		std::list<MsgData> l_tmpBuffer;

		if (!m_uplinkHarqBuffer[m_harqProcess].empty())
		{	
			for (auto& l_msg : m_uplinkHarqBuffer[m_harqProcess])
			{
				l_tmpBuffer.push_back(m_uplinkHarqBuffer[m_harqProcess].front());
				m_uplinkHarqBuffer[m_harqProcess].pop_front();
			}

			l_curBuffer = &l_tmpBuffer;
		}
		else
		{
			l_curBuffer = &m_dataBuffer;
		}

		auto& l_dataIt = l_curBuffer->begin();
		auto& l_resIt = m_allocatedResources.begin();

		size_t l_tbSize;
		std::shared_ptr<UplinkDataMessage> l_dataMsg;

		while (l_resIt != m_allocatedResources.end())
		{
			auto& radResource = *l_resIt;

			if (radResource.tti < m_runtime.curTti(p_tm))
			{
				m_logger->debug("Wasted resource!");
				m_allocatedResources.pop_front();
				l_resIt = m_allocatedResources.begin();
				continue;
			}

			if (radResource.tti != m_runtime.curTti(p_tm))
			{
				m_logger->debug("Next allocated resource in tti = {}, now = {}",
					radResource.tti, m_runtime.curTti(p_tm));
				return;
			}

			l_tbSize = 0;
			l_dataMsg =	std::make_shared<UplinkDataMessage>(m_eNodeBId, m_id, 0);


			while (l_dataIt != l_curBuffer->end())
			{
				if ((radResource.capacity() - l_tbSize) >= l_dataIt->size)
				{
					l_tbSize += l_dataIt->size;
					m_uplinkHarqBuffer[m_harqProcess].push_back(*l_dataIt);
					l_curBuffer->pop_front();
					l_dataIt = l_curBuffer->begin();
				}
				else
				{
					MsgData tmp = *l_dataIt;
					tmp.size = radResource.capacity() - l_tbSize;
					m_uplinkHarqBuffer[m_harqProcess].push_back(tmp);
					l_dataIt->size -= tmp.size;
					l_tbSize = radResource.capacity();
					break;
				}
			}

			m_logger->debug("l_tbSize: {}", l_tbSize);

			l_dataMsg->dataSize = l_tbSize;

			if (l_curBuffer == &m_dataBuffer)
			{
				m_dataSize -= l_tbSize;
			}

			auto& nextResIt = std::next(l_resIt);

			if (nextResIt == m_allocatedResources.end() ||
				nextResIt->tti != m_runtime.curTti(p_tm))
			{
				l_dataMsg->bsr = m_dataSize;
				m_logger->debug("bsr: {}", l_dataMsg->bsr);
			}

			m_PUSCH->send(l_dataMsg, radResource);

			m_allocatedResources.pop_front();
			l_resIt = m_allocatedResources.begin();
		}
	}
}

void UE::sendSr(Runtime::TimePoint p_tm)
{
	if ((m_runtime.curTti(p_tm) - m_srSubfarmeOffset) % m_srPeriodicity == 0 &&
		m_runtime.curTti(p_tm) > m_waitUntilForResponse)
	{
		m_logger->debug("tti {}: UE[{}] - SR opportunity, sending", m_runtime.curTti(p_tm), m_id);
		m_PUCCH->send(std::make_shared<SchedulingRequestMessage>(getId()));
		m_waitUntilForResponse = m_runtime.curTti(p_tm) + 5;
	}
}

bool UE::hasResourcesForTransmission() const
{
	return !m_allocatedResources.empty();
}

void UE::recieve(std::shared_ptr<RadioMessage> p_msg, Runtime::TimePoint p_tm)
{
	m_logger->debug("UE[{}]: recieve message", m_id);

	if (p_msg->type == RadioMessageType::UplinkGrant)
	{
		processUplinkGrant(std::static_pointer_cast<UplinkGrantMessage>(p_msg), p_tm);
	}

	if (p_msg->type == RadioMessageType::HarqAck)
	{
		processHarqAck(std::static_pointer_cast<HarqAckMessage>(p_msg), p_tm);
	}

	if (p_msg->type == RadioMessageType::HarqNack)
	{
		processHarqNack(std::static_pointer_cast<HarqNackMessage>(p_msg), p_tm);
	}
}

void UE::processUplinkGrant(std::shared_ptr<UplinkGrantMessage> p_msg,
	Runtime::TimePoint p_tm)
{
	m_logger->debug("UE[{}]: recieve grant for tti {}", m_id, p_msg->radResource.tti);

	for (auto& l_prb : p_msg->radResource.prbs)
	{
		m_logger->debug("\ts={}:p={}", l_prb.slot, l_prb.num);
	}

	m_allocatedResources.push_back(p_msg->radResource);

	m_pendingSr = false;
}

void UE::processHarqAck(std::shared_ptr<HarqAckMessage> p_msg,
	                    Runtime::TimePoint p_tm)
{
	m_logger->debug("UE[{}]: ACK recieved", m_id);

	size_t m_harqProcess = (m_runtime.curTti(p_tm) - 4) % 8;
	m_logger->debug("Harq process {}", m_harqProcess);

	for (auto& l_mdgData : m_uplinkHarqBuffer[m_harqProcess])
	{
		StatsCollector::getInstance().txDataForENodeBUe(l_mdgData, m_eNodeBId, m_id);
		m_logger->debug("in: {} ms - out: {} ms - delay: {} ",
			(double)m_runtime.microsecondsFromStart(l_mdgData.arrivalTime).count() / 1000,
			(double)(m_runtime.microsecondsFromStart(p_tm) - 1ms).count() / 1000,
			(double)(std::chrono::time_point_cast<std::chrono::microseconds>(m_runtime.curTtiPoint(p_tm)) -
				std::chrono::time_point_cast<std::chrono::microseconds>(l_mdgData.arrivalTime)).count() / 1000);
		StatsCollector::getInstance().addUplinkPacketDelayForENodeBUe((std::chrono::time_point_cast<std::chrono::microseconds>(m_runtime.curTtiPoint(p_tm)) -
			std::chrono::time_point_cast<std::chrono::microseconds>(l_mdgData.arrivalTime)).count(), m_eNodeBId, m_id);
	}

	StatsCollector::getInstance().setBytesInHarqBuffersForENodeBUe(bytesInHarqBuffers(), m_eNodeBId, m_id); 

	m_uplinkHarqBuffer[m_harqProcess].clear();
}

void UE::processHarqNack(std::shared_ptr<HarqNackMessage> p_msg,
	                     Runtime::TimePoint p_tm)
{
	m_logger->debug("UE[{}]: NACK recieved", m_id);

	size_t m_harqProcess = (m_runtime.curTti(p_tm) - 4) % 8;
	m_logger->debug("Harq process {}", m_harqProcess);
	m_logger->debug("Harq buffer size {}", m_uplinkHarqBuffer[m_harqProcess].size());
}

uint64_t UE::bytesInHarqBuffers() const
{
	uint64_t l_dataInHarqBuffers = 0;
	for (size_t l_harqId = 0; l_harqId < 8; ++l_harqId)
	{
		for (auto& l_mdgData : m_uplinkHarqBuffer[l_harqId])
		{
			l_dataInHarqBuffers += l_mdgData.size;
		}
	}

	return l_dataInHarqBuffers;
}
