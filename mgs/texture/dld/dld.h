#pragma once
#include <inttypes.h>

struct DldHeader {
	uint8_t type;
	uint8_t priority;
	uint8_t alignment;
	uint8_t pad0;
	uint32_t pad1;
	uint32_t strcode;
	uint32_t parentDataSize;
	uint32_t dataSize;
	uint32_t mipMapCount;
	uint32_t entryNumber;
	uint32_t pad2;
};