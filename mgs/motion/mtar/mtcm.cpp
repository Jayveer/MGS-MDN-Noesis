#include "mtcm.h"

int determineArchiveSize(uint8_t* mtcm, int archiveIdx, int numArchive) {
	MtcmHeader* mtcmHeader = (MtcmHeader*)mtcm;
	int rotOffset = mtcmHeader->quatOffset[archiveIdx];

	int nextOffset = archiveIdx == (numArchive - 1) ? mtcmHeader->rootOffset : mtcmHeader->quatOffset[archiveIdx + 1];
	int size = nextOffset - rotOffset;
	return size <= 0 ? mtcmHeader->rootOffset - rotOffset : size;
}

void swapEndianMtcm(uint8_t* mtcm) {
	MtcmHeader* header = (MtcmHeader*)mtcm;
	genericSwap(header, 11, 4);
	genericSwap(&header->indiciesOffset, 1, 2);
	genericSwap(&header->rootOffset, 4, 4);

	int numArchive = ((header->flags >> 4) == 0x07) ? header->numJoints * 2 : header->numJoints;
	genericSwap(&header->quatOffset, numArchive, 4);

	uint8_t* mtcmArchive = &mtcm[header->archiveOffset];
	genericSwap(mtcmArchive, header->archiveSize, 2);
}