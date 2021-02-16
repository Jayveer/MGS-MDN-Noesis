#pragma once
#include <inttypes.h>

#include "../../common/util.h"

struct vec4 {
	float x;
	float y;
	float z;
	float w;
};

struct vec3 {
	float x;
	float y;
	float z;
};

struct vec4Half {
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint16_t w;
};

enum eTexture {
	DIFFUSETEX,
	NORMALTEX,
	SPECULARTEX,
	SPECULAR_GRADIENTTEX,
	INCIDENCETEX,
	REFLECTTEX,
	WRINKLETEX,
	UNKNOWNTEX
};

enum eDefinition {
	POSITION,
	WEIGHT,
	NORMAL,
	COLOUR,
	UNKNOWNA,
	UNKNOWNB,
	UNKNOWNC,
	BONEIDX,
	TEXTURE00,
	TEXTURE01,
	TEXTURE02,
	TEXTURE03,
	TEXTURE04,
	TEXTURE05,
	TANGENT
};

struct MdnHeader {
	uint32_t magic;
	uint32_t filename;
	uint32_t numBones;
	uint32_t numGroups;
	uint32_t numMesh;
	uint32_t numFace;
	uint32_t numVertexDefinition;
	uint32_t numMaterial;
	uint32_t numTexture;
	uint32_t numSkin;
	uint32_t boneOffset;
	uint32_t groupOffset;
	uint32_t meshOffset;
	uint32_t faceOffset;
	uint32_t vertexDefinitionOffset;
	uint32_t materialOffset;
	uint32_t textureOffset;
	uint32_t skinOffset;
	uint32_t vertexBufferOffset;
	uint32_t vertexBufferSize;
	uint32_t faceBufferOffset;
	uint32_t faceBufferSize;
	uint32_t pad;
	uint32_t fileSize;
	vec4 max;
	vec4 min;
};

struct MdnBone {
	uint32_t strcode;
	uint32_t flag;
	int32_t parent;
	uint32_t pad;
	vec4 parentPos;
	vec4 worldPos;
	vec4 max;
	vec4 min;
};

struct MdnGroup {
	uint32_t strcode;
	uint32_t flag;
	uint32_t parent;
	uint32_t pad;
};

struct MdnMesh {
	uint32_t groupIdx;
	uint32_t flag;
	uint32_t numFaceIdx;
	uint32_t faceIdx;
	uint32_t vertexDefIdx;
	uint32_t skinIdx;
	uint32_t numVertex;
	uint32_t pad;
	vec4 max;
	vec4 min;
	vec4 pos;
};

struct MdnFace {
	uint16_t type;
	uint16_t count;
	uint32_t offset;
	uint32_t matGroup;
	uint16_t start;
	uint16_t size;
};

struct MdnVertexDefinition {
	uint32_t pad;
	uint32_t defintionCount;
	uint32_t stride;
	uint32_t offset;
	uint8_t definition[16];
	uint8_t position[16];
};

struct MdnMaterial {
	uint32_t flag;
	uint32_t strcode;
	uint32_t numTexture;
	uint32_t numParams;
	uint32_t texture[8];
	vec4Half params[8];
};

struct MdnTexture {
	uint32_t strcode;
	uint32_t flag;
	float scaleU;
	float scaleV;
	float posU;
	float posV;
	uint32_t pad[2];
};

struct MdnSkin {
	uint32_t unknown;
	uint16_t count;
	uint16_t nullBytes;
	uint8_t boneId[32];
};