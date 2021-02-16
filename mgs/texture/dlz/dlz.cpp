#include "dlz.h"

Dlz::Dlz(std::string filename) {
	this->filename = filename;
}

Dlz::~Dlz() {
}

void Dlz::setRapi(noeRAPI_t* rapi) {
	this->rapi = rapi;
}

void Dlz::decompressSeg(uint8_t* seg, uint8_t* dst, int& size) {
	SegHeader* header = (SegHeader*)seg;
	uint16_t numChunks = _byteswap_ushort(header->numChunks);
	uint32_t decompressedSize = _byteswap_ulong(header->decompressedSize);

	int pos = 0;

	for (int i = 0; i < numChunks; i++) {
		uint32_t offset = _byteswap_ulong(header->table[i].offset) - 1;
		uint16_t chunkCompressedSize = _byteswap_ushort(header->table[i].compressedSize);
		uint16_t chunkDecompressedSize = _byteswap_ushort(header->table[i].decompressedSize);

		int size;
		rapi->Decomp_Inflate2(&seg[offset], &dst[pos], chunkCompressedSize, chunkDecompressedSize, -15);

		pos += chunkDecompressedSize;
	}

	size = decompressedSize;
}

uint32_t Dlz::peekMagic(std::ifstream& ifs) {
	uint32_t magic;
	ifs.read((char*)&magic, 4);
	ifs.seekg(-4, ifs.cur);
	return magic;
}

bool Dlz::isSeg(std::ifstream& ifs) {
	return peekMagic(ifs) == 0x73676573;
}

int Dlz::getMaxDecompressedSize(std::ifstream& ifs, int& numSegs, int fileSize) {
	int size = 0;

	while (isSeg(ifs)) {
		ifs.seekg(8, ifs.cur);

		uint32_t segSize;
		ifs.read((char*)&segSize, 4);

		ifs.seekg(-12, ifs.cur);
		size += _byteswap_ulong(segSize);

		numSegs++;

		int curPos = ifs.tellg();
		if ((curPos + MAXSEG) > fileSize) break;

		ifs.seekg(MAXSEG, ifs.cur);
	}

	ifs.seekg(0, ifs.beg);
	return size;
}

uint8_t* Dlz::decompress(int& size) {
	std::ifstream dlz;
	dlz.open(filename, std::ios::binary);
	int fileSize = getFileSize(filename);

	int pos = 0;
	int numSegs = 0;
	int totalDecompressedSize = getMaxDecompressedSize(dlz, numSegs, fileSize);

	uint8_t* dld = new uint8_t[totalDecompressedSize];

	for (int i = 0; i < numSegs; i++) {
		int remaining = fileSize - dlz.tellg();
		int size = remaining < MAXSEG ? remaining : MAXSEG;

		uint8_t* seg = new uint8_t[size];
		dlz.read((char*)seg, size);

		int decompressedSize = 0;
		decompressSeg(seg, &dld[pos], decompressedSize);
		delete[] seg;

		pos += decompressedSize;
	}

	size = totalDecompressedSize;
	dlz.close();
	return dld;
}