#pragma once

#include <memory>
#include <list>
#include <map>
#include <string>

#include "Runtime.h"
#include "spdlog/spdlog.h"
#include "RadioMessages.h"
#include "RadioChannelReciever.h"
#include "RadioResource.h"

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
		m_messages.push_back(std::make_pair(p_radResource, p_msg));
	}

	void pushToRecievers(Runtime::TimePoint p_tm);

protected:
	std::shared_ptr<spdlog::logger> m_logger;
	Runtime& m_runtime;
	std::string m_name;

private:
	double m_errorPr;

	std::list<std::pair<RadioResource, std::shared_ptr<RadioMessage>>> m_messages;
	std::map<uint64_t, RadioChannelReciever*> m_recievers;
};

