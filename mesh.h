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
    case 0x07:
        return RPGEODATA_HALFFLOAT;
    case 0x08:
        return RPGEODATA_UBYTE;
    case 0x09:
        return RPGEODATA_BYTE;
    case 0x0A:
        return RPGEODATA_FLOAT;
    default:
        printf("unknown");
    }
}

inline
void readDec(std::vector<float>& dec, uint8_t* vertexBuffer, int32_t stride, int32_t numVertex) {
    for (int i = 0; i < numVertex; i++) {
        uint32_t bitstream = *(uint32_t*)vertexBuffer;
        bitstream = _byteswap_ulong(bitstream);
        
        RichBitStream bs = RichBitStream(&bitstream, 4);

        float vx = bs.ReadBits(11) / 1023.0f;
        float vy = bs.ReadBits(11) / 1023.0f;
        float vz = bs.ReadBits(10) / 511.0f;

        vx = byteswap_float(vx);
        vy = byteswap_float(vy);
        vz = byteswap_float(vz);
        float vw = byteswap_float(01);

        dec.push_back(vx);
        dec.push_back(vy);
        dec.push_back(vz);
        dec.push_back(vw);

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
            float w = weight / 255.0f;
            w = byteswap_float(w);
            weights.push_back(w);
        }
        vertexBuffer += stride;
    }
}



inline
void bindMesh(MdnMesh* mesh, MdnSkin* skin, MdnGroup* group, MdnVertexDefinition* vertexDefinition, uint8_t* vertexBuffer, noeRAPI_t* rapi, std::vector<float>& normals, std::vector<float>& weights, std::vector<float>& tangents, std::vector<uint8_t>& bones) {
    int32_t numVertex     = _byteswap_ulong(mesh->numVertex);
    int32_t stride        = _byteswap_ulong(vertexDefinition->stride);
    int32_t offset        = _byteswap_ulong(vertexDefinition->offset);
    int32_t numDefinition = _byteswap_ulong(vertexDefinition->defintionCount);
    int64_t size          = numVertex * stride;
        
    skin  = &skin[_byteswap_ulong(mesh->skinIdx)];
    group = &group[_byteswap_ulong(mesh->groupIdx)];

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
            readWeight(&vertexBuffer[pos], weights, stride, numVertex);
            rapi->rpgBindBoneWeightBufferSafe(&weights[0], RPGEODATA_FLOAT, 16, 4, weights.size() * 4);
            break;
        case eDefinition::NORMAL:
            if ((daf & 0xF0) >> 4 == 0x0A) {
                readDec(normals, &vertexBuffer[pos], stride, numVertex);
                rapi->rpgBindNormalBufferSafe(&normals[0], type, 16, normals.size() * 4);
            }  else {
                rapi->rpgBindNormalBufferSafe(&vertexBuffer[pos], type, stride, size);
            }            
            break;
        case eDefinition::COLOUR:
            if (!g_mgs4RenderVertexColour) break;
            rapi->rpgBindColorBufferSafe(&vertexBuffer[pos], type, stride, 4, size);
            break;
        case eDefinition::BONEIDX:
            readSkin(skin, &vertexBuffer[pos], bones, stride, numVertex);
            rapi->rpgBindBoneIndexBufferSafe(&bones[0], RPGEODATA_UBYTE, 4, 4, bones.size());
            break;
        case eDefinition::TEXTURE00:
        case eDefinition::TEXTURE02:
        case eDefinition::TEXTURE03:
        case eDefinition::TEXTURE04:
        case eDefinition::TEXTURE05:
            rapi->rpgBindUVXBufferSafe(&vertexBuffer[pos], type, stride, def - 8, 2, size);
            break;
        case eDefinition::TANGENT:
            if ((daf & 0xF0) >> 4 == 0x0A) {
                readDec(tangents, &vertexBuffer[pos], stride, numVertex);
                rapi->rpgBindTangentBufferSafe(&tangents[0], type, 16, tangents.size() * 4);
            } else {
                rapi->rpgBindTangentBufferSafe(&vertexBuffer[pos], type, stride, size);
            }          
           
            break;
        }
    }
}