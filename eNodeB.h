#pragma once

#include "Runtime.h"
#include "spdlog/spdlog.h"
#include "UE.h"
#include "UlScheduler.h"
#include "RadioChannelReciever.h"
#include "PUCCH.h"
#include "PDCCH.h"
#include "PUSCH.h"
#include "PHICH.h"

#include <map>

class eNodeB : public RadioChannelReciever
{
public:
	eNodeB(uint64_t p_id, Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger,
		size_t p_prbNumber)
		: m_id(p_id),
		  m_nextUeId(0),
		  m_runtime(p_runtime),
		  m_logger(p_logger),
		  m_prbNumber(p_prbNumber),
		  m_PUCCH(std::make_shared<PUCCH>(p_runtime, p_logger, "PUCCH")),
		  m_PDCCH(std::make_shared<PDCCH>(p_runtime, p_logger, "PDCCH")),
		  m_PUSCH(std::make_shared<PUSCH>(p_runtime, p_logger, "PUSCH", 0.1)),
		  m_PHICH(std::make_shared<PHICH>(p_runtime, p_logger, "PHICH"))
	{
		m_PUCCH->addReciever(this);
		m_PUSCH->addReciever(this);

		m_runtime.schedule(m_runtime.start() - 3ns, std::bind(&eNodeB::schedule, this, std::placeholders::_1), this);
	}

	~eNodeB();

	std::shared_ptr<UlScheduler> createUlScheduler(SchedulerAlgorithm p_algo = SchedulerAlgorithm::RoundRobin,
										           UlSchedulerMode p_mode = UlSchedulerMode::DynamicScheduling);
	std::shared_ptr<UE> addUe(uint64_t p_tiiToAttach);

	uint64_t getId() override
	{
		return m_id;
	}

	void recieve(std::shared_ptr<RadioMessage> p_msg, Runtime::TimePoint p_tm) override;

	void processSchedulingRequest(std::shared_ptr<SchedulingRequestMessage> p_msg, Runtime::TimePoint p_tm);
	void processUplinkData(std::shared_ptr<UplinkDataMessage> p_msg, Runtime::TimePoint p_tm);

	void sendAcknowledgement(bool p_isNegative, uint64_t p_ueId, Runtime::TimePoint p_tm);


private:
	void attachUe(uint64_t p_ueId, Runtime::TimePoint p_tm);
	void schedule(Runtime::TimePoint p_tm);

	uint64_t m_id;

	uint64_t m_nextUeId;

	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;

	size_t m_prbNumber;

	std::map<uint64_t, std::shared_ptr<UE>> m_connectedUes;

	std::shared_ptr<UlScheduler> m_ulScheduler;

	// channels
	std::shared_ptr<PUCCH> m_PUCCH;
	std::shared_ptr<PDCCH> m_PDCCH;

	std::shared_ptr<PUSCH> m_PUSCH;

	std::shared_ptr<PHICH> m_PHICH;
};

