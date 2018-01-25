#include "LteModel.h"

#include <iostream>

LteBandParams lteBandParamsTable[static_cast<int>(LteBand::BAND_END)] = {
	{ 1400000, 6 },
	{ 3000000, 15 },
	{ 5000000, 25 },
	{ 10000000, 50 },
	{ 15000000, 75 },
	{ 20000000, 100 },
};

LteModel::LteModel(std::shared_ptr<spdlog::logger> p_logger,
				   LteDuplexType p_duplexType,
	               LteBand p_band)
	: m_nextBaseStationId(0),
	  m_duplexType(p_duplexType),
	  m_band(p_band),
	  m_bandwidthHz(lteBandParamsTable[static_cast<int>(m_band)].bandwidthHz),
	  m_prbNumber(lteBandParamsTable[static_cast<int>(m_band)].prbNumber),
	  m_logger(p_logger),
	  m_runtime(Runtime::Clock::now())
{
	m_runtime.schedule(m_runtime.timePointForTti(0) - 1ns, std::bind(&LteModel::printTti, this, std::placeholders::_1), this);
}

void LteModel::printInfo()
{
	m_logger->info("*********** LTE MODEL INFO ***********");

	if (m_duplexType == LteDuplexType::LTE_FDD)
	{
		m_logger->info("Duplex Division Type: FDD");
	}
	else
	{
		m_logger->info("Duplex Division Type: TDD");
	}

	m_logger->info("Bandwidth: {} Hz", m_bandwidthHz);

	m_logger->info("PRB number: {}", m_prbNumber);
}

std::shared_ptr<eNodeB> LteModel::createBaseStation()
{
	m_eNodeBs.emplace_back(std::make_shared<eNodeB>(m_nextBaseStationId,
		m_runtime, m_logger, m_prbNumber));
	m_logger->info("eNodeB[{}] added", m_nextBaseStationId);
	m_nextBaseStationId++;
	return m_eNodeBs.back();
}

void LteModel::run(uint64_t p_ttiCount)
{
	m_logger->info("========================================");
	m_logger->info("========================================");
	m_logger->info("*********** MODELING STARTED ***********");

	for(uint64_t i = 0; i < p_ttiCount; ++i)
	{
		m_runtime.trigger(1ms);
	}
}