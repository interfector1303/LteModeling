#pragma once

#include <list>

#include "Runtime.h"
#include "spdlog/spdlog.h"
#include "TrafficGenerator.h"
#include "RadioChannelReciever.h"
#include "PUCCH.h"
#include "PUSCH.h"

class TrafficGenerator;

class UE : public RadioChannelReciever
{
public:
	UE(uint64_t p_id, Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger)
		: m_id(p_id),
		  m_runtime(p_runtime),
		  m_logger(p_logger),
		  m_srConfigured(false),
		  m_srPeriodicity(0),
		  m_srSubfarmeOffset(0),
		  m_waitUntilForResponse(0),
		  m_pendingSr(false),
		  m_dataSize(0)
	{}

	~UE();

	void configureSr(uint32_t p_srPeriodicity, uint32_t p_srSubfarmeOffset)
	{
		m_srConfigured = true;
		m_srPeriodicity = p_srPeriodicity;
		m_srSubfarmeOffset = p_srSubfarmeOffset;
	}

	void attached(Runtime::TimePoint p_tm);

	void setTrafficGenerator(std::shared_ptr<TrafficGenerator> p_trafficGenerator)
	{
		m_trafficGenerator = p_trafficGenerator;
	}

	void setPUCCH(std::shared_ptr<PUCCH> p_PUCCH)
	{
		m_PUCCH = p_PUCCH;
	}

	void setPUSCH(std::shared_ptr<PUSCH> p_PUSCH)
	{
		m_PUSCH = p_PUSCH;
	}

	void setENodeBId(uint64_t m_id)
	{
		m_eNodeBId = m_id;
	}

	uint64_t getId() override
	{
		return m_id;
	}

	void recieve(std::shared_ptr<RadioMessage> p_msg, Runtime::TimePoint p_tm);

	void processUplinkGrant(std::shared_ptr<UplinkGrantMessage> p_msg,
		                      Runtime::TimePoint p_tm);
	void processHarqAck(std::shared_ptr<HarqAckMessage> p_msg,
		                Runtime::TimePoint p_tm);
	void processHarqNack(std::shared_ptr<HarqNackMessage> p_msg,
		                 Runtime::TimePoint p_tm);

	uint64_t bytesInHarqBuffers() const;

private:
	void processTti(Runtime::TimePoint p_tm);
	void getNewData(MsgData p_msgData, Runtime::TimePoint p_tm);
	void sendData(Runtime::TimePoint p_tm);
	void sendSr(Runtime::TimePoint p_tm);

	bool hasResourcesForTransmission() const;

	uint64_t m_id;
	uint64_t m_eNodeBId;

	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;

	// Scheduling Request
	bool m_srConfigured;
	uint32_t m_srPeriodicity;
	uint32_t m_srSubfarmeOffset;
	uint64_t m_waitUntilForResponse;
	bool m_pendingSr;

	// Traffic
	std::shared_ptr<TrafficGenerator> m_trafficGenerator;
	std::list<MsgData> m_dataBuffer;
	size_t m_dataSize;

	std::list<MsgData> m_uplinkHarqBuffer[8];

	//channels
	std::shared_ptr<PUCCH> m_PUCCH;
	std::shared_ptr<PUSCH> m_PUSCH;

	std::list<RadioResource> m_allocatedResources;
};

