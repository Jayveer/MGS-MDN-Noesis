#include "txn.h"

Txn::Txn(std::string filename) {
	std::ifstream fs;
	int size = std::filesystem::file_size(filename);

	fs.open(filename, std::ios::binary);
	uint8_t* p = new uint8_t[size];
	fs.read((char*)p, size);
	this->txnData = p;
	fs.close();

	this->header = (TxnHeader*)txnData;
}

Txn::~Txn() {
	delete[] txnData;
}

void Txn::setRapi(noeRAPI_t* rapi) {
	this->rapi = rapi;
}

bool Txn::containsTexture(uint32_t strcode) {
	return getIdx(strcode) > -1;
}

int Txn::getIdx(uint32_t strcode) {
	int32_t numTexture = _byteswap_ulong(header->numTexture);
	int32_t textureOffset = _byteswap_ulong(header->textureOffset);
	TxnTexture* textureInfo = (TxnTexture*)&txnData[textureOffset];

	for (int i = 0; i < numTexture; i++) {
		if (_byteswap_ulong(textureInfo[i].texStrcode) == strcode)
			return i;
	}

	return -1;
}

uint8_t* getFromCache(uint32_t txnStrcode, int idx, int& size, uint8_t priority, const std::vector<DldInfo>& dldArray) {
	for (int i = 0; i < dldArray.size(); i++) {
		int pos = 0;
		while (pos < dldArray[i].size) {
			DldHeader* header = (DldHeader*)&dldArray[i].dld[pos];
			if ((header->priority == priority) && header->strcode == txnStrcode && (_byteswap_ulong(header->entryNumber) == idx)) {
				size = _byteswap_ulong(header->dataSize);
				return &dldArray[i].dld[pos + 0x20];
			}
			pos += (_byteswap_ulong(header->dataSize) + 0x20);
			pos += getAlignment(pos, 0x10);
		}
	}
	return NULL;
}

uint8_t* Txn::getTexture(uint32_t strcode, int& size, const std::vector<DldInfo>& dldArray) {
	int idx = getIdx(strcode);
	if (idx == -1) return NULL;
	return getTextureIndexed(idx, size, dldArray);
}

noesisTexType_e getFourCC(uint16_t flag) {
	flag = _byteswap_ushort(flag);

	switch (flag) {
	case 0x03:
		return NOESISTEX_RGBA32;
	case 0x09:
		return NOESISTEX_DXT1;
	case 0x0B :
		return NOESISTEX_DXT5;
	default:
		printf("unknown");
	}

	return NOESISTEX_DXT1;
}

bool isExternal(uint16_t flag) {
	return (_byteswap_ushort(flag) & 0xF0) == 0xF0;
}

uint8_t* Txn::tgaFromDXT(uint8_t* dxtData, int width, int height, noesisTexType_e dxtFmt, int& size) {
	int dataSize = width * height * 4;
	uint8_t* rgba = dxtFmt == NOESISTEX_RGBA32 ? dxtData : rapi->Noesis_ConvertDXT(width, height, dxtData, dxtFmt);
	uint8_t* tga = rapi->Image_CreateTGAFromRGBA32(rgba, dataSize, width, height, &size);
	if (dxtFmt != NOESISTEX_RGBA32) rapi->Noesis_UnpooledFree(rgba);
	return tga;
}

uint8_t* Txn::getTextureIndexed(int idx, int& size, const std::vector<DldInfo>& dldArray) {
	int32_t textureOffset = _byteswap_ulong(header->textureOffset);
	TxnTexture* textureInfo = (TxnTexture*)&txnData[textureOffset];

	uint32_t  imageOffset = _byteswap_ulong(textureInfo[idx].imageOffset);
	TxnImage* image = (TxnImage*)&txnData[imageOffset];

	noesisTexType_e dxt = getFourCC(image->fourCC);
	int16_t width  = _byteswap_ushort(textureInfo[idx].width);
	int16_t height = _byteswap_ushort(textureInfo[idx].height);

	if (isExternal(image->flag)) {
		uint8_t*   data = getFromCache(textureInfo[idx].txnStrcode, idx, size, 0, dldArray);
		if (!data) data = getFromCache(textureInfo[idx].txnStrcode, idx, size, 3, dldArray);
		if (!data) return NULL;

		return tgaFromDXT(data, width, height, dxt, size);
	}

	uint32_t offset       = _byteswap_ulong(image->offset);
	uint32_t mipMapOffset = _byteswap_ulong(image->mipMapOffset);

	return tgaFromDXT(&txnData[offset], width, height, dxt, size);
}