#pragma once

#include "TrafficGenerator.h"
#include "spdlog/spdlog.h"

#include <map>
#include <list>

struct UeStats
{
	uint64_t packetForTx;
	uint64_t bytesForTx;
	uint64_t bytesTx;
	uint64_t bytesInBuffer;
	uint64_t bytesInHarqBuffers;

	std::list<uint64_t> uplinkPacketDalays;
};

class StatsCollector
{
public:
	static StatsCollector& getInstance()
	{
		static StatsCollector l_instance;
		return l_instance;
	}

	void ttiCounterIncr()
	{
		m_tti++;
	}

	void newDataPacketForENodeBUe(const MsgData& p_msgData, uint64_t p_eNodeBId, uint64_t p_ueId)
	{
		/*if (m_statsPerENodeB[p_eNodeBId].count(p_ueId) == 0)
		{
			m_statsPerENodeB[p_eNodeBId].insert(std::make_pair(p_ueId, UeStats()));
		}*/

		m_statsPerENodeB[p_eNodeBId][p_ueId].packetForTx++;
		m_statsPerENodeB[p_eNodeBId][p_ueId].bytesForTx += p_msgData.size;
	}

	void txDataForENodeBUe(const MsgData& p_msgData, uint64_t p_eNodeBId, uint64_t p_ueId)
	{
		m_statsPerENodeB[p_eNodeBId][p_ueId].bytesTx += p_msgData.size;
	}

	void setBytesInBufferForENodeBUe(uint64_t p_bytes, uint64_t p_eNodeBId, uint64_t p_ueId)
	{
		m_statsPerENodeB[p_eNodeBId][p_ueId].bytesInBuffer = p_bytes;
	}

	void setBytesInHarqBuffersForENodeBUe(uint64_t p_bytes, uint64_t p_eNodeBId, uint64_t p_ueId)
	{
		m_statsPerENodeB[p_eNodeBId][p_ueId].bytesInHarqBuffers = p_bytes;
	}

	void addUplinkPacketDelayForENodeBUe(uint64_t p_delayInMicrosecs, uint64_t p_eNodeBId, uint64_t p_ueId)
	{
		m_statsPerENodeB[p_eNodeBId][p_ueId].uplinkPacketDalays.push_back(p_delayInMicrosecs);
	}

	void printStats(std::shared_ptr<spdlog::logger> p_logger)
	{
		p_logger->info("");
		p_logger->info("*********** PRINT STATISTICS ***********");
		p_logger->info("");

		p_logger->info("TTIs simulated: {}", m_tti);
		p_logger->info("seconds simulated: {}", (double)m_tti / 1000);
		p_logger->info("");

		for (auto& l_eNodeBStats : m_statsPerENodeB)
		{
			p_logger->info("Statistics for eNodeB[{}]", l_eNodeBStats.first);

			for (auto& l_ueStats : l_eNodeBStats.second)
			{
				p_logger->info("############ UE[{}] #############", l_ueStats.first);

				p_logger->info("PAR: {}", (double)l_ueStats.second.packetForTx / m_tti);
				p_logger->info("");
				p_logger->info("Packets for transmission: {}", l_ueStats.second.packetForTx);
				p_logger->info("Bytes for transmission: {}", l_ueStats.second.bytesForTx);
				p_logger->info("");
				p_logger->info("Bytes transmitted: {}", l_ueStats.second.bytesTx);
				p_logger->info("");
				p_logger->info("Bytes in new data buffer: {}", l_ueStats.second.bytesInBuffer);
				p_logger->info("");
				p_logger->info("Bytes in HARQ buffers: {}", l_ueStats.second.bytesInHarqBuffers);
				p_logger->info("{} + {} + {} = {}", l_ueStats.second.bytesTx,
					l_ueStats.second.bytesInBuffer, l_ueStats.second.bytesInHarqBuffers,
					l_ueStats.second.bytesTx + l_ueStats.second.bytesInBuffer + l_ueStats.second.bytesInHarqBuffers);
				p_logger->info("");

				uint64_t l_avgUplinkPacketDelay = 0;

				for (auto& l_del : l_ueStats.second.uplinkPacketDalays)
				{
					l_avgUplinkPacketDelay += l_del;
				}

				if (!l_ueStats.second.uplinkPacketDalays.empty())
				{
					l_avgUplinkPacketDelay /= l_ueStats.second.uplinkPacketDalays.size();
				}

				p_logger->info("Avg uplink packet delay: {}", (double)l_avgUplinkPacketDelay / 1000);

				p_logger->info("################################");
				p_logger->info("");
			}
		}
	}

private:
	StatsCollector()
		: m_tti(0)
	{}

	~StatsCollector() = default;

	using UesStats = std::map<uint64_t, UeStats>;
	std::map<uint64_t, UesStats> m_statsPerENodeB;

	size_t m_tti;
};

