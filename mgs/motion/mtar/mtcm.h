#pragma once
#include <inttypes.h>
#include "../../common/util.h"

struct MtcmHeader {
	uint32_t name;
	uint32_t flags;
	uint32_t baseTick;
	uint32_t numFrames;
	uint32_t archiveOffset;
	uint32_t archiveSize;
	uint32_t numJoints;
	uint32_t bitCheck0;
	uint32_t bitCheck1;
	uint32_t bitCheck2;
	uint32_t bitCheck3;
	uint8_t  quatLowBit;
	uint8_t  quatHighBit;
	uint16_t indiciesOffset;
	uint32_t rootOffset;	// - starting offset from data location, or count of preceding 16-bit (2-byte) values (flag 0x10)
	uint32_t moveOffset;    // - starting offset from data location, or count of preceding 16 - bit(2 - byte) values(flag 0x20)
	uint32_t turnOffset;
	uint32_t fixOffset;		// - starting offset from data location, or count of preceding 16-bit (2-byte) values (flag 0x04)
	uint32_t quatOffset[];
};

extern void swapEndianMtcm(uint8_t* mtcm);
extern int determineArchiveSize(uint8_t* mtcm, int archiveIdx, int numArchive);