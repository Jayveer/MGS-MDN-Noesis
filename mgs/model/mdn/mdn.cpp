#include "mdn.h"

int getSize(std::vector<uint8_t>& offsets, uint8_t pos, int stride) {
	int last = offsets.size() - 1;
	auto it = std::find(offsets.begin(), offsets.end(), pos);
	int idx = it - offsets.begin();

	return pos == offsets[last] ? stride - offsets[idx] : offsets[idx + 1] - offsets[idx];
}

std::vector<uint8_t> formatOffsets(uint8_t* pos, int numPos) {
	std::vector<uint8_t> offsets(numPos);
	memcpy(&offsets[0], pos, numPos);

	std::sort(offsets.begin(), offsets.end());
	offsets.erase(std::unique(offsets.begin(), offsets.end()), offsets.end());

	return offsets;
}

bool isSwapped(std::vector<uint8_t>& swappedOffsets, uint8_t pos) {
	for (int i = 0; i < swappedOffsets.size(); i++) {
		if (swappedOffsets[i] == pos)
			return 1;
	}

	return 0;
}

void swapMDNVertexBuffer(uint8_t* vertexBuffer, MdnVertexDefinition* vertexDefinition, int numVertex) {	
	int numData = 0;
	vertexBuffer += vertexDefinition->offset;
	uint32_t stride = vertexDefinition->stride;
	uint32_t defCount = vertexDefinition->defintionCount;

	std::vector offsets = formatOffsets(vertexDefinition->position, defCount);

	for (int i = 0; i < numVertex; i++) {
		std::vector<uint8_t> swappedOffsets;
		for (int j = 0; j < defCount; j++) {			
			uint8_t pos = vertexDefinition->position[j];
			uint8_t def = vertexDefinition->definition[j] >> 4;
			int size = getSize(offsets, pos, stride);

			switch (def) {
			case 0x01:
			case 0x0A:
				numData = size / 4;
				if (!isSwapped(swappedOffsets, pos)) {
					genericSwap(&vertexBuffer[pos], numData, 4);
					swappedOffsets.push_back(pos);
				} 
				break;
			case 0x07:
				numData = size / 2;
				if (!isSwapped(swappedOffsets, pos)) {
					genericSwap(&vertexBuffer[pos], numData, 2);
					swappedOffsets.push_back(pos);
				}
			}
		}
		vertexBuffer += vertexDefinition->stride;
	}
}

void swapMDNFaceBuffer(uint8_t* faceBuffer, MdnMesh* mesh, MdnFace* face) {
	int length = mesh->faceIdx + mesh->numFaceIdx;

	for (int i = mesh->faceIdx; i < length; i++) {
		int32_t pos = face[i].offset;
		genericSwap(&faceBuffer[pos], face[i].count, 2);
	}
}

void swapEndianMDN(uint8_t* mdn) {
	MdnHeader* header = (MdnHeader*)mdn;
    genericSwap(&header->filename, 0x19, 4);

	MdnBone* bones = (MdnBone*)&mdn[header->boneOffset];
	for (int i = 0; i < header->numBones; i++) {
		genericSwap(&bones[i], 0x14, 4);
	}

	MdnGroup* groups = (MdnGroup*)&mdn[header->groupOffset];
	for (int i = 0; i < header->numGroups; i++) {
		genericSwap(&groups[i], 0x04, 4);
	}

	MdnMesh* mesh = (MdnMesh*)&mdn[header->meshOffset];
	for (int i = 0; i < header->numMesh; i++) {
		genericSwap(&mesh[i], 0x14, 4);
	}

	MdnFace* face = (MdnFace*)&mdn[header->faceOffset];
	for (int i = 0; i < header->numFace; i++) {
		genericSwap(&face[i],        0x02, 2);
		genericSwap(&face[i].offset, 0x02, 4);
		genericSwap(&face[i].start,  0x02, 2);
	}

	MdnVertexDefinition* vertexDefinition = (MdnVertexDefinition*)&mdn[header->vertexDefinitionOffset];
	for (int i = 0; i < header->numVertexDefinition; i++) {
		genericSwap(&vertexDefinition[i], 0x04, 4);
	}

	MdnMaterial* material = (MdnMaterial*)&mdn[header->materialOffset];
	for (int i = 0; i < header->numMaterial; i++) {
		genericSwap(&material[i],        0x0C, 4);
		genericSwap(&material[i].params, 0x10, 2);
	}

	MdnTexture* texture = (MdnTexture*)&mdn[header->textureOffset];
	for (int i = 0; i < header->numTexture; i++) {
		genericSwap(&texture[i], 0x08, 4);
	}

	MdnSkin* skin = (MdnSkin*)&mdn[header->skinOffset];
	for (int i = 0; i < header->numSkin; i++) {
		genericSwap(&skin[i],       0x01, 4);
		genericSwap(&skin[i].count, 0x02, 2);
	}

	uint8_t* faceBuffer = (uint8_t*)&mdn[header->faceBufferOffset];
	uint8_t* vertexBuffer = (uint8_t*)&mdn[header->vertexBufferOffset];
	for (int i = 0; i < header->numMesh; i++) {
		swapMDNVertexBuffer(vertexBuffer, &vertexDefinition[i], mesh[i].numVertex);
		swapMDNFaceBuffer(faceBuffer, &mesh[i], face);
	}
}