#pragma once
#include "util.h"
#include "mgs/model/mdn/mdn.h"
#include "noesis/plugin/pluginshare.h"

inline
int findBoneIdx(char* boneName, modelBone_t* noeBones, int numBones) {
    for (int i = 0; i < numBones; i++) {
        if (!strcmp(noeBones[i].name, boneName))
            return i;
    }
    return -1;
}

inline
modelBone_t* bindBones(MdnBone* bones, int numBones, noeRAPI_t* rapi) {
    modelBone_t* noeBones = rapi->Noesis_AllocBones(numBones);

    for (int i = 0; i < numBones; i++) {
        std::string boneName = intToHexString(bones[i].strcode);
        strcpy_s(noeBones[i].name, boneName.c_str());

        RichVec4 bonePos = { bones[i].worldPos.x, bones[i].worldPos.y, bones[i].worldPos.z, bones[i].worldPos.w };
        RichVec3 bonePosV3 = bonePos.ToVec3();
        memcpy_s(&noeBones[i].mat.o, 12, &bonePosV3, 12);

        int32_t parent = bones[i].parent;
        if (parent > -1)
            noeBones[i].eData.parent = &noeBones[parent];
    }

    rapi->rpgSetExData_Bones(noeBones, numBones);
    return noeBones;
}