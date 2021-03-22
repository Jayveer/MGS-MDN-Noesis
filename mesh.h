#pragma once
#include <vector>
#include "util.h"
#include "mgs/model/mdn/mdn.h"
#include "noesis/plugin/pluginshare.h"

bool g_mgs4RenderVertexColour = false;

inline
rpgeoDataType_e getType(uint8_t type) {
    switch ((type & 0xF0) >> 4) {
    case 0x01:
        return RPGEODATA_FLOAT;
    case 0x05:
        return RPGEODATA_SHORT;
    case 0x07:
        return RPGEODATA_HALFFLOAT;
    case 0x08:
        return RPGEODATA_UBYTE;
    case 0x09:
        return RPGEODATA_BYTE;
    case 0x0A:
        return RPGEODATA_FLOAT;
    case 0x0B:
        return RPGEODATA_BYTE;
    default:
        printf("unknown");
    }
}

inline
void readDec(std::vector<float>& dec, uint8_t* vertexBuffer, int32_t stride, int32_t numVertex) {
    for (int i = 0; i < numVertex; i++) {
        uint32_t bitstream = *(uint32_t*)vertexBuffer;
        
        RichBitStream bs = RichBitStream(&bitstream, 4);
        
        int32_t ax = bs.ReadBits(11);
        int32_t ay = bs.ReadBits(11);
        int32_t az = bs.ReadBits(10);

        if (ax & 0x400) ax |= -0x100;
        if (ay & 0x400) ay |= -0x100;
        if (az & 0x200) az |= -0x100;

        float vx = ax / 1023.0f;
        float vy = ay / 1023.0f;
        float vz = az / 511.0f;
        
        dec.push_back(vx);
        dec.push_back(vy);
        dec.push_back(vz);

        vertexBuffer += stride;
    }
}

inline
void readSkin(MdnSkin* skin, uint8_t* vertexBuffer, std::vector<uint8_t>& bones, int32_t stride, int32_t numVertex) {
    for (int i = 0; i < numVertex; i++) {
        for (int j = 0; j < 4; j++) {
            uint8_t boneIdx = vertexBuffer[j];
            bones.push_back(skin->boneId[boneIdx]);
        }
        vertexBuffer += stride;
    }
}

inline
void readWeight(uint8_t* vertexBuffer, std::vector<float>& weights, int32_t stride, int32_t numVertex) {
    for (int i = 0; i < numVertex; i++) {
        for (int j = 0; j < 4; j++) {
            uint8_t weight = vertexBuffer[j];
            weights.push_back(weight / 255.0f);
        }
        vertexBuffer += stride;
    }
}

inline
void bindMesh(MdnMesh* mesh, MdnSkin* skin, MdnGroup* group, MdnVertexDefinition* vertexDefinition, uint8_t* vertexBuffer, noeRAPI_t* rapi, std::vector<float>& normals, std::vector<float>& weights, std::vector<float>& tangents, std::vector<uint8_t>& bones) {
    int32_t stride        = vertexDefinition->stride;
    int32_t offset        = vertexDefinition->offset;
    int32_t numDefinition = vertexDefinition->defintionCount;
    int64_t size          = mesh->numVertex * stride;
        
    skin  = &skin[mesh->skinIdx];
    group = &group[mesh->groupIdx];

    vertexBuffer += offset;

    for (int i = 0; i < numDefinition; i++) {
        uint8_t pos = vertexDefinition->position[i];
        uint8_t daf = vertexDefinition->definition[i];
        uint8_t def = vertexDefinition->definition[i] & 0x0F;
        rpgeoDataType_e type = getType(vertexDefinition->definition[i]);

        switch (def) {
        case eDefinition::POSITION:
            rapi->rpgBindPositionBufferSafe(&vertexBuffer[pos], type, stride, size);
            break;
        case eDefinition::WEIGHT:
            readWeight(&vertexBuffer[pos], weights, stride, mesh->numVertex);
            rapi->rpgBindBoneWeightBufferSafe(&weights[0], RPGEODATA_FLOAT, 16, 4, weights.size() * 4);
            break;
        case eDefinition::NORMAL:
            if ((daf & 0xF0) >> 4 == 0x0A) {
                readDec(normals, &vertexBuffer[pos], stride, mesh->numVertex);
                rapi->rpgBindNormalBufferSafe(&normals[0], type, 12, normals.size() * 4);
            }  else {
                rapi->rpgBindNormalBufferSafe(&vertexBuffer[pos], type, stride, size);
            }            
            break;
        case eDefinition::COLOUR:
            if (!g_mgs4RenderVertexColour) break;
            rapi->rpgBindColorBufferSafe(&vertexBuffer[pos], type, stride, 4, size);
            break;
        case eDefinition::TEXTURE3DS:
            rapi->rpgBindUV1BufferSafe(&vertexBuffer[pos], type, stride, size);
            break;
        case eDefinition::BONEIDX:
            readSkin(skin, &vertexBuffer[pos], bones, stride, mesh->numVertex);
            rapi->rpgBindBoneIndexBufferSafe(&bones[0], RPGEODATA_UBYTE, 4, 4, bones.size());
            break;
        case eDefinition::TEXTURE00:
        case eDefinition::TEXTURE01:
        case eDefinition::TEXTURE02:
        case eDefinition::TEXTURE03:
        case eDefinition::TEXTURE04:
        case eDefinition::TEXTURE05:
            rapi->rpgBindUVXBufferSafe(&vertexBuffer[pos], type, stride, def - 8, 2, size);
            break;
        case eDefinition::TANGENT:
            if ((daf & 0xF0) >> 4 == 0x0A) {
                readDec(tangents, &vertexBuffer[pos], stride, mesh->numVertex);
                //rapi->rpgBindTangentBufferSafe(&tangents[0], type, 12, tangents.size() * 4);
            } else {
                rapi->rpgBindTangentBufferSafe(&vertexBuffer[pos], type, stride, size);
            }                     
            break;
        default:
            printf("unknown");
        }
    }
}