#pragma once
#include "RadioChannel.h"

class PUCCH : public RadioChannel
{
public:
	PUCCH(Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger, const std::string& p_name)
		: RadioChannel(p_runtime, p_logger, p_name, 0)
	{}	

	~PUCCH()
	{}
};

