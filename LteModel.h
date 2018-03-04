#pragma once

#include <memory>
#include <list>
#include <iostream>

#include "LteParameters.h"
#include "eNodeB.h"
#include "Runtime.h"
#include "spdlog/spdlog.h"

class LteModel
{
public:
	LteModel(std::shared_ptr<spdlog::logger> p_logger,
		     LteDuplexType p_duplexType = LteDuplexType::LTE_FDD,
		     LteBand p_band = LteBand::BAND_20_MHz);

	~LteModel()
	{}

	void printInfo();

	std::shared_ptr<eNodeB> createBaseStation();

	void printTti(Runtime::TimePoint p_tm)
	{
		m_logger->debug("TTI {}", m_runtime.curTti(p_tm) + 1);
		m_runtime.schedule(m_runtime.nextTtiPoint(p_tm) + 1ms - 1ns, std::bind(&LteModel::printTti, this, std::placeholders::_1), this);
	}

	void run(uint64_t p_ttiCount);

private:
	uint64_t m_nextBaseStationId;

	LteDuplexType m_duplexType;
	LteBand m_band;
	size_t m_bandwidthHz;
	size_t m_prbNumber;

	std::list<std::shared_ptr<eNodeB>> m_eNodeBs;

	std::shared_ptr<spdlog::logger> m_logger;
	Runtime m_runtime;
};

