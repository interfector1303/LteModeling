#pragma once
#include "RadioChannel.h"
class PHICH :
	public RadioChannel
{
public:
	PHICH(Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger, const std::string& p_name)
		: RadioChannel(p_runtime, p_logger, p_name, 0)
	{}

	~PHICH()
	{}
};

