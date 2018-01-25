#include "FtpModel3.h"

FtpModel3::~FtpModel3()
{
}

MsgData FtpModel3::getNextMsgData()
{
	auto l_nextMsgArrival = getNextMessageArrivalTp();
	l_nextMsgArrival += std::chrono::microseconds((int)(m_expDistribution(m_generator) * pow(10, 3)));
	//l_nextMsgArrival += std::chrono::microseconds(3000);
	setNextMessageArrivalTp(l_nextMsgArrival);
	
	return MsgData(m_dataSizeBytes, l_nextMsgArrival);
}
