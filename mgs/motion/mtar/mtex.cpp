#include "mtex.h"

void swapEndianMtex(uint8_t* mtex) {
	MtexHeader* header = (MtexHeader*)mtex;
	genericSwap(header, 8, 4);
	genericSwap(&mtex[header->eventOffset], header->numEvents * 3, 4);
	genericSwap(&header->mtpData, header->numMTP * 4, 4);

	if (header->numMTP) {
		int offset = header->mtpData[0].moveOffset * 2;
		int size   = (header->dataSize - offset) / 2;

		genericSwap(&mtex[offset], size, 2);
	}
}