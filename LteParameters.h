#pragma once

#include <inttypes.h>

enum class LteDuplexType {
	LTE_TDD,
	LTE_FDD
};

enum class LteBand {
	BAND_1_4_MHz = 0,
	BAND_3_MHz,
	BAND_5_MHz,
	BAND_10_MHz,
	BAND_15_MHz,
	BAND_20_MHz,
	BAND_END
};

struct LteBandParams
{
	size_t bandwidthHz;
	size_t prbNumber;
};
