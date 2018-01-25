#pragma once

#include <random>
#include <chrono>

#include "TrafficGenerator.h"

class FtpModel3 : public TrafficGenerator
{
public:
	FtpModel3(size_t p_dataSizeBytes, double p_lambda)
		: m_dataSizeBytes(p_dataSizeBytes),
		  m_generator(std::chrono::system_clock::now().time_since_epoch().count()),
		  m_expDistribution(p_lambda)
	{
	}

	~FtpModel3();

	MsgData getNextMsgData() override;

private:
	size_t m_dataSizeBytes;
	std::default_random_engine m_generator;
	std::exponential_distribution<double> m_expDistribution;
};

