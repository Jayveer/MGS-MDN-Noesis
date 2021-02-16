#pragma once

#include "mgs/model/mdn/mdn.h"
#include "noesis/plugin/pluginshare.h"

void bindFace(MdnMesh* mesh, MdnFace* face, uint8_t* faceBuffer, noeRAPI_t* rapi) {
    int32_t numFaceIdx = _byteswap_ulong(mesh->numFaceIdx);
    int32_t faceIdx = _byteswap_ulong(mesh->faceIdx);
    int32_t length = faceIdx + numFaceIdx;

    for (int i = faceIdx; i < length; i++) {
        int32_t pos = _byteswap_ulong(face[i].offset);
        int16_t fCount = _byteswap_ushort(face[i].count);
        int32_t matGroup = _byteswap_ulong(face[i].matGroup);

        rapi->rpgSetMaterialIndex(matGroup);
        rapi->rpgCommitTrianglesSafe(&faceBuffer[pos], RPGEODATA_SHORT, fCount, RPGEO_TRIANGLE, false);
    }
}