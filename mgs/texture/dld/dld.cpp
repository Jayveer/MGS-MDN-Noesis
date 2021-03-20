#include "dld.h"

void swapDldHeader(DldHeader* header) {
	genericSwap(&header->pad1, 7, 4);
}

void swapDld(uint8_t* dld, int size) {
	int pos = 0;
	while (pos < size) {
		DldHeader* header = (DldHeader*)&dld[pos];
		swapDldHeader(header);
		pos += header->dataSize + 0x20;
		pos += getAlignment(pos, 0x10);
	}
}