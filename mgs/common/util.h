#pragma once
#include <sstream>

inline
std::string formatStrcode(std::string strcode) {
	uint8_t length = strcode.size();
	uint8_t pad = 6 - length;
	std::string prefix;

	for (int i = 0; i < pad; i++) {
		prefix += "0";
	}

	return (prefix + strcode);

}

inline
std::string intToHexString(int value) {
	std::stringstream ss;
	ss << std::hex << value;
	return formatStrcode(ss.str());
}

inline
void genericSwap(void* data, int numData, int length) {

	for (int i = 0; i < numData; i++) {

		switch (length) {
		case 2: {
			uint16_t* pData = (uint16_t*)data;
			pData[i] = _byteswap_ushort(pData[i]);
			break;
		}
		case 4: {
			uint32_t* pData = (uint32_t*)data;
			pData[i] = _byteswap_ulong(pData[i]);
			break;
		}
		case 8: {
			uint64_t* pData = (uint64_t*)data;
			pData[i] = _byteswap_uint64(pData[i]);
			break;
		}
		}
	}

}