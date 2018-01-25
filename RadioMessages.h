#pragma once

enum RadioMessageType
{
	SchedulingRequest
};

#define TO_ALL_RECIEVER_ID 0xFFFFFFFF

struct RadioMessage
{
	RadioMessage(uint64_t p_recieverId, RadioMessageType p_type)
		: recieverId(p_recieverId),
		  type(p_type),
		  crcValid(true)
	{}

	uint64_t recieverId;
	RadioMessageType type;
	bool crcValid;
};

struct SchedulingRequestMessage : public RadioMessage
{
	SchedulingRequestMessage()
		: RadioMessage(TO_ALL_RECIEVER_ID, SchedulingRequest)
	{}
};