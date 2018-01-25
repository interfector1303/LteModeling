#pragma once

#include <memory>
#include <list>
#include <map>
#include <string>

#include "Runtime.h"
#include "spdlog/spdlog.h"
#include "RadioMessages.h"
#include "RadioChannelReciever.h"

struct RadioResource
{
	RadioResource(uint32_t p_prbNum = 0, uint64_t p_tti = 0, uint8_t p_slot = 0)
		: prbNum(prbNum),
		  tti(p_tti),
		  slot(p_slot)
	{}
	uint32_t prbNum;
	uint64_t tti;
	uint8_t slot;
};

class RadioChannel
{
public:
	RadioChannel(Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger,
		const std::string& p_name, double p_errorPr)
		: m_runtime(p_runtime),
		  m_logger(p_logger),
		  m_name(p_name),
		  m_errorPr(p_errorPr)
	{
		m_logger->info("RadioChannel[{}] is created", m_name);
		m_runtime.schedule(m_runtime.nextTtiPointRelNow() + 1ms - 2ns, std::bind(&RadioChannel::pushToRecievers, this, std::placeholders::_1), this);
	}

	virtual ~RadioChannel()
	{}


	void addReciever(RadioChannelReciever* p_reciever)
	{
		m_logger->debug("RadioChannel[{}]: reciever with id {} is added", m_name, p_reciever->getId());
		m_recievers[p_reciever->getId()] = p_reciever;
	}

	void removeReciever(RadioChannelReciever* p_reciever)
	{
		m_logger->debug("RadioChannel[{}]: reciever with id {} is removed", m_name, p_reciever->getId());
		m_recievers.erase(p_reciever->getId());
	}

	void send(std::shared_ptr<RadioMessage> p_msg, RadioResource p_radResource = RadioResource())
	{
		m_messages.push_back(std::make_pair(p_msg, p_radResource));
	}

	void pushToRecievers(Runtime::TimePoint p_tm)
	{
		m_logger->debug("RadioChannel[{}]: pushing messages to recievers", m_name);

		m_messages.clear();
		m_runtime.schedule(m_runtime.nextTtiPoint(p_tm) + 1ms - 2ns, std::bind(&RadioChannel::pushToRecievers, this, std::placeholders::_1), this);
	}

protected:
	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;
	std::string m_name;

private:
	double m_errorPr;

	std::list<std::pair<std::shared_ptr<RadioMessage>, RadioResource>> m_messages;
	std::map<uint64_t, RadioChannelReciever*> m_recievers;
};

