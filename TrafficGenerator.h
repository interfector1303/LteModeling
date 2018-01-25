#pragma once

#include <chrono>
#include <memory>

#include "Runtime.h"

class UE;

struct MsgData
{
	size_t size;
	Runtime::TimePoint arrivalTime;

	MsgData(size_t p_size, Runtime::TimePoint p_arrivalTime)
		: size(p_size),
		  arrivalTime(p_arrivalTime)
	{}
};

class TrafficGenerator
{
public:
	TrafficGenerator()
	{}

	virtual ~TrafficGenerator()
	{}

	void setNextMessageArrivalTp(Runtime::TimePoint p_tp)
	{
		m_nextMessageArrivalTp = p_tp;
	}

	Runtime::TimePoint getNextMessageArrivalTp() const
	{
		return m_nextMessageArrivalTp;
	}

	virtual MsgData getNextMsgData() = 0;

private:
	Runtime::TimePoint m_nextMessageArrivalTp;
};

