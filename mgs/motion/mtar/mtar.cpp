#include "mtar.h"

bool isSwapped(std::vector<uint32_t>& swappedMt, uint32_t offset) {
	for (int i = 0; i < swappedMt.size(); i++) {
		if (offset == swappedMt[i])
			return true;
	}
	return false;
}

void swapEndianMtar(uint8_t* mtar) {
	MtarHeader* header = (MtarHeader*)mtar;
	genericSwap(mtar, 1, 4);
	genericSwap(&header->maxJoint, 4, 2);
	genericSwap(&header->flags, 5, 4);

	uint32_t* boneTable = (uint32_t*)&mtar[header->boneNameTableOffset];
	genericSwap(boneTable, header->numBoneName, 4);

	MtarData* mtcmTable = (MtarData*)&mtar[header->dataTableOffset];
	genericSwap(mtcmTable, header->numMotion * 4, 4);

	std::vector<uint32_t> swappedMtcm;
	uint8_t* mtcm = (uint8_t*)&mtar[header->mtcmOffset];

	for (int i = 0; i < header->numMotion; i++) {
		if (!isSwapped(swappedMtcm, mtcmTable[i].mtcmOffset)) {
			uint8_t* thisMtcm = &mtcm[mtcmTable[i].mtcmOffset];
			swapEndianMtcm(thisMtcm);
			swappedMtcm.push_back(mtcmTable[i].mtcmOffset);
		}
	}

	std::vector<uint32_t> swappedMtex;
	uint8_t* mtex = (uint8_t*)&mtar[header->mtexOffset];

	for (int i = 0; i < header->numMotion; i++) {
		if (mtcmTable[i].mtexSize && !isSwapped(swappedMtex, mtcmTable[i].mtexOffset)) {
			uint8_t* thisMtex = &mtex[mtcmTable[i].mtexOffset];
			swapEndianMtex(thisMtex);
			swappedMtex.push_back(mtcmTable[i].mtexOffset);
		}
	}
}