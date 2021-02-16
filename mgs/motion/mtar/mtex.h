#pragma once
#include <inttypes.h>

struct MtpData {
	uint32_t mtpName;
	uint32_t boneName;
	uint32_t moveOffset;
	uint32_t rotOffset;
};

struct MtexHeader {
	uint32_t name;
	uint32_t flags;
	uint32_t numFrames;
	uint32_t numEvents;
	uint32_t numMTP;    // motion point
	uint32_t quatBits;
	uint32_t dataSize;
	uint32_t eventOffset;
	MtpData mtpData[];
};

struct EventHeader {
	uint32_t time;
	uint32_t signal;
	uint32_t param;
};