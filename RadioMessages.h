#pragma once

#include "RadioResource.h"

enum RadioMessageType
{
	SchedulingRequest,
	UplinkGrant,
	UplinkData,
	HarqAck,
	HarqNack
};

#define TO_ALL_RECIEVER_ID 0xFFFFFFFF

struct RadioMessage
{
	RadioMessage(uint64_t p_recieverId, uint64_t p_senderId, RadioMessageType p_type)
		: recieverId(p_recieverId),
		  senderId(p_senderId),
		  type(p_type),
		  crcValid(true)
	{}

	uint64_t recieverId;
	uint64_t senderId;
	RadioMessageType type;
	bool crcValid;
};

struct SchedulingRequestMessage : public RadioMessage
{
	SchedulingRequestMessage(uint64_t p_senderId)
		: RadioMessage(TO_ALL_RECIEVER_ID, p_senderId, SchedulingRequest)
	{}
};

struct UplinkGrantMessage : public RadioMessage
{
	UplinkGrantMessage(uint64_t p_recieverId, uint64_t p_senderId, uint64_t p_tti)
		: RadioMessage(p_recieverId, p_senderId, UplinkGrant),
		  radResource(p_tti)
	{}

	RadioResource radResource;
};

struct UplinkDataMessage : public RadioMessage
{
	UplinkDataMessage(uint64_t p_recieverId, uint64_t p_senderId, size_t p_dataSize)
		: RadioMessage(p_recieverId, p_senderId, UplinkData),
		  dataSize(p_dataSize)
	{}

	size_t dataSize;
	size_t bsr;
};

struct HarqAckMessage : public RadioMessage
{
	HarqAckMessage(uint64_t p_recieverId, uint64_t p_senderId)
		: RadioMessage(p_recieverId, p_senderId, HarqAck)
	{}
};

struct HarqNackMessage : public RadioMessage
{
	HarqNackMessage(uint64_t p_recieverId, uint64_t p_senderId)
		: RadioMessage(p_recieverId, p_senderId, HarqNack)
	{}
};