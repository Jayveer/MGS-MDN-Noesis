#pragma once
#pragma once

inline
unsigned int strcode(const char* string) {
	unsigned char c;
	unsigned char* p = (unsigned char*)string;
	unsigned int id, mask = 0x00FFFFFF;

	for (id = 0; c = *p; p++) {
		id = ((id >> 19) | (id << 5));
		id += c;
		id &= mask;
	}

	return (!id) ? 1 : id;
}