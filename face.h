#pragma once

#include "mgs/model/mdn/mdn.h"
#include "noesis/plugin/pluginshare.h"

void bindFace(MdnMesh* mesh, MdnFace* face, uint8_t* faceBuffer, noeRAPI_t* rapi) {
    int32_t numFaceIdx = mesh->numFaceIdx;
    int32_t faceIdx = mesh->faceIdx;
    int32_t length = faceIdx + numFaceIdx;

    for (int i = faceIdx; i < length; i++) {
        int32_t pos = face[i].offset;
        int16_t fCount = face[i].count;
        int32_t matGroup = face[i].matGroup;

        rapi->rpgSetMaterialIndex(matGroup);
        rapi->rpgCommitTrianglesSafe(&faceBuffer[pos], RPGEODATA_SHORT, fCount, RPGEO_TRIANGLE, false);
    }
}