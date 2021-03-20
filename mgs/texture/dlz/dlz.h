#pragma once
#include <inttypes.h>
#include "../../common/util.h"
#include "../../common/fileutil.h"
#include "../../../noesis/plugin/pluginshare.h"

struct SegIndex {
	uint16_t compressedSize;
	uint16_t decompressedSize;
	uint32_t offset;
};

struct SegHeader {
	uint32_t magic;
	uint16_t flag;
	uint16_t numChunks;
	uint32_t decompressedSize;
	uint32_t compressedSize; //little endian
	SegIndex table[];
};

class Dlz {

public:
	Dlz(std::string filename);
	~Dlz();
	void setRapi(noeRAPI_t* rapi);
	uint8_t* decompress(int& size);
private:
	std::string filename;
	const int MAXSEG = 0x20000;

	noeRAPI_t* rapi;
	bool isBigEndian = false;

	void setEndian();
	bool isSeg(std::ifstream& ifs);
	void swapDlzHeader(SegHeader* header);
	uint32_t peekMagic(std::ifstream& ifs);
	void decompressSeg(uint8_t* seg, uint8_t* dst, int& size);
	int getMaxDecompressedSize(std::ifstream& ifs, int& numSegs, int fileSize);
};