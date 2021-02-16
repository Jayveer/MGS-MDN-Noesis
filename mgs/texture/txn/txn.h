#pragma once
#include <vector>
#include "../dld/dld.h"
#include "../../common/fileutil.h"
#include "../../../noesis/plugin/pluginshare.h"

struct DldInfo {
	uint8_t* dld;
	int size;
};

struct TxnImage {
	uint16_t width;
	uint16_t height;
	uint16_t fourCC;
	uint16_t flag;
	uint32_t offset;
	uint32_t mipMapOffset;
};

struct TxnTexture {
	uint32_t flag;
	uint32_t texStrcode;
	uint32_t txnStrcode;
	uint16_t width;
	uint16_t height;
	uint16_t xOffset;
	uint16_t yOffset;
	uint32_t imageOffset;
	uint32_t pad;
	float uScale;
	float vScale;
	float uOffset;
	float vOffset;
	uint32_t pad2;
};

struct TxnHeader {
	uint32_t pad;
	uint32_t flag;
	int32_t numImage;
	int32_t imageOffset;
	int32_t numTexture;
	int32_t textureOffset;
	uint32_t pad1;
	uint32_t pad2;
};

class Txn {
public:
	Txn(std::string filename);
	~Txn();

	noeRAPI_t* rapi;
	int getIdx(uint32_t strcode);
	void setRapi(noeRAPI_t* rapi);
	bool containsTexture(uint32_t strcode);
	uint8_t* getTextureIndexed(int idx, int& size, const std::vector<DldInfo>& dld);
	uint8_t* getTexture(uint32_t strcode, int& size, const std::vector<DldInfo>& dldArray);
private:
	uint8_t* txnData;
	TxnHeader* header;
	std::string filename;

	uint8_t* Txn::tgaFromDXT(uint8_t* dxtData, int width, int height, noesisTexType_e dxtFmt, int& size);
};