#include "UE.h"
#include "RadioMessages.h"

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
	m_logger->debug("tti {}: UE[{}] process", m_runtime.curTti(p_tm), m_id);

	if (m_dataSize != 0)
	{
		m_logger->debug("tti {}: UE[{}] has data for transsmition", m_runtime.curTti(p_tm), m_id);
		sendData(p_tm);
	}

	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm), std::bind(&UE::processTti, this, std::placeholders::_1), this);
}

void UE::getNewData(MsgData p_msgData, Runtime::TimePoint p_tm)
{
	m_dataBuffer.push_back(p_msgData);
	m_dataSize += p_msgData.size;

	m_logger->debug("Ue[{}]: new data, buffer size = {}", m_id, m_dataSize);
	//m_logger->debug("Ue[{}]: RATE = {}", m_id, (double)m_dataBuffer.size() / m_runtime.curTti(p_tm));

	auto l_nextMsgData = m_trafficGenerator->getNextMsgData();
	m_runtime.schedule(l_nextMsgData.arrivalTime, std::bind(&UE::getNewData, this, l_nextMsgData, std::placeholders::_1), this);
}

void UE::sendData(Runtime::TimePoint p_tm)
{
	if (!hasResourcesForTransmission())
	{
		m_logger->debug("tti {}: UE[{}] - no resources", m_runtime.curTti(p_tm), m_id);
		sendSr(p_tm);
		return;
	}
}

void UE::sendSr(Runtime::TimePoint p_tm)
{
	if ((m_runtime.curTti(p_tm) - m_srSubfarmeOffset) % m_srPeriodicity == 0)
	{
		m_logger->debug("tti {}: UE[{}] - SR opportunity, sending", m_runtime.curTti(p_tm), m_id);
		m_PUCCH->send(std::make_shared<SchedulingRequestMessage>());
	}
}

bool UE::hasResourcesForTransmission() const
{
	return false;
}
