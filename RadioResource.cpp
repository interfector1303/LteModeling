#include "RadioResource.h"

bool operator < (const Prb& p_left, const Prb& p_right)
{
	if (p_left.slot < p_right.slot)
	{
		return true;
	}
	else if (p_left.slot == p_right.slot)
	{
		if (p_left.num < p_right.num)
		{
			return true;
		}
	}

	return false;
}
