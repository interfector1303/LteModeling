#include "RadioChannel.h"

void RadioChannel::pushToRecievers(Runtime::TimePoint p_tm)
{
	std::map<Prb, std::list<std::shared_ptr<RadioMessage>>> l_prbMap;

	for (auto& l_msgDescriptor : m_messages)
	{
		for (auto& l_prb : l_msgDescriptor.first.prbs)
		{
			l_prbMap[l_prb].push_back(l_msgDescriptor.second);
		}
	}

	for (auto& l_msgList : l_prbMap)
	{
		bool l_crcValid = true;
		if (l_msgList.second.size() != 1)
		{
			l_crcValid = false;
		}

		for (auto& l_msg : l_msgList.second)
		{
			l_msg->crcValid = l_crcValid;
		}
	}

	for (auto& l_msgDescriptor : m_messages)
	{
		auto& l_msg = l_msgDescriptor.second;

		double l_p = (double)rand() / RAND_MAX;

		if (l_p < m_errorPr)
		{
			l_msg->crcValid = false;
		}

		if (l_msg->recieverId != TO_ALL_RECIEVER_ID)
		{
			m_recievers[l_msg->recieverId]->recieve(l_msg, p_tm);
		}
		else
		{
			for (auto& l_recv : m_recievers)
			{
				l_recv.second->recieve(l_msg, p_tm);
			}
		}
	}

	m_messages.clear();
	m_runtime.schedule(m_runtime.nextTtiPoint(p_tm) + 1ms - 2ns, std::bind(&RadioChannel::pushToRecievers, this, std::placeholders::_1), this);
}
