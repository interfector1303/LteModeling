#pragma once

#include <inttypes.h>
#include <list>

struct Prb
{
	Prb(uint32_t p_num = 0, uint8_t p_slot = 0, size_t p_capacity = 0)
		: num(p_num),
		  slot(p_slot),
		  capacity(p_capacity)
	{}
	uint32_t num;
	uint8_t slot;
	size_t capacity; // in bytes

	friend bool operator < (const Prb& p_left, const Prb& p_right);
};

struct RadioResource
{
	RadioResource(uint64_t p_tti = 0)
		: tti(p_tti)
	{}

	size_t capacity() const
	{
		size_t l_res = 0;

		for (auto& l_prb : prbs)
		{
			l_res += l_prb.capacity;
		}

		return l_res;
	}

	uint64_t tti;
	std::list<Prb> prbs;
};