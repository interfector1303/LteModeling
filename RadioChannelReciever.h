#pragma once

#include <memory>
#include "RadioMessages.h"

class RadioChannelReciever
{
public:
	virtual uint64_t getId() = 0;
	virtual void recieve(std::shared_ptr<RadioMessage> p_msg, Runtime::TimePoint p_tm) = 0;
};
