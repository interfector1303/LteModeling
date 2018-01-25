#pragma once

#include <list>

#include "Runtime.h"
#include "spdlog/spdlog.h"
#include "TrafficGenerator.h"
#include "RadioChannelReciever.h"
#include "PUCCH.h"

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

	uint64_t getId() override
	{
		return m_id;
	}

	void recieve(std::shared_ptr<RadioMessage> p_msg) override
	{

	}

private:
	void processTti(Runtime::TimePoint p_tm);
	void getNewData(MsgData p_msgData, Runtime::TimePoint p_tm);
	void sendData(Runtime::TimePoint p_tm);
	void sendSr(Runtime::TimePoint p_tm);

	bool hasResourcesForTransmission() const;

	uint64_t m_id;
	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;

	// Scheduling Request
	bool m_srConfigured;
	uint32_t m_srPeriodicity;
	uint32_t m_srSubfarmeOffset;

	// Traffic
	std::shared_ptr<TrafficGenerator> m_trafficGenerator;
	std::list<MsgData> m_dataBuffer;
	size_t m_dataSize;

	//channels
	std::shared_ptr<PUCCH> m_PUCCH;
};

