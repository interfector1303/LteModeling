#include <iostream>
#include <stdlib.h>

#include "spdlog/spdlog.h"
#include "LteModel.h"
#include "FtpModel3.h"
#include "StatsCollector.h"

void main()
{
	auto l_logger = spdlog::stdout_color_st("LteModel");
	l_logger->set_level(spdlog::level::debug);

	try
	{
		LteModel l_model(l_logger);
		l_model.printInfo();

		auto l_eNodeB = l_model.createBaseStation();

		// Dynamic scheduling
		/*auto l_ulScheduler = l_eNodeB->createUlScheduler(SchedulerAlgorithm::RoundRobin,
			                                             UlSchedulerMode::DynamicScheduling);
		l_ulScheduler->setSrPeriodicity(SrPeriodicity::_1ms);*/

		auto l_ulScheduler = l_eNodeB->createUlScheduler(SchedulerAlgorithm::RoundRobin,
			                                             UlSchedulerMode::PreScheduling);

		auto l_ue0 = l_eNodeB->addUe(0);
		l_ue0->setTrafficGenerator(std::make_shared<FtpModel3>(50, 0.1));

		auto l_ue1 = l_eNodeB->addUe(0);
		l_ue1->setTrafficGenerator(std::make_shared<FtpModel3>(50, 0.1));

		//auto l_ue2 = l_eNodeB->addUe(0);
		//l_ue2->setTrafficGenerator(std::make_shared<FtpModel3>(500, 0.3));

		l_model.run(30);

		StatsCollector::getInstance().printStats(l_logger);
	}
	catch (std::exception& l_ex)
	{
		l_logger->error("{}", l_ex.what());
	}
}