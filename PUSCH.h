#pragma once
#include "RadioChannel.h"
class PUSCH :
	public RadioChannel
{
public:
	PUSCH(Runtime& p_runtime, std::shared_ptr<spdlog::logger> p_logger,
		const std::string& p_name, double p_errorPr)
		: RadioChannel(p_runtime, p_logger, p_name, p_errorPr)
	{}

	~PUSCH()
	{}
};

