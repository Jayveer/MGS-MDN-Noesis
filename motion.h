#pragma once
#include "mgs/motion/mtar/mtar.h"
#include "mgs/motion/mtar/mtcm.h"
#include "mgs/motion/mtar/mtex.h"
#include "noesis/plugin/pluginshare.h"

#include "include/half/half.h"

const double g_mgs4_PI = acos(-1);
const float  g_mgs4_GAME_FRAMERATE = 60.0f;

struct MoveAnimation {
    int keyframe;
    float x;
    float y;
    float z;
};

struct RotAnimation {
    int keyframe;
    float x;
    float y;
    float z;
    float w;
};

inline
float shiftRadix(const float& f, const int& exponent) {
    return f * pow(2, exponent);
}

inline
float short2Half(const uint16_t& n) {
    half x;
    x.setBits(n);
    float r = x;
    return r;
}

inline
float short2HalfExp(const uint16_t& n) {
    half x;
    x.setBits(n);
    float r = x;
    return shiftRadix(r, 7);
}

inline
bool checkMagic(BYTE* motionFile) {
    MtarHeader* header = (MtarHeader*)motionFile;
    return header->magic == 0x7261744D || header->magic == 0x4D746172;
}

inline
BYTE* openMotion(noeRAPI_t* rapi) {
    int len;
    char out[MAX_NOESIS_PATH];
    BYTE* mtarFile = rapi->Noesis_LoadPairedFile("load mtar", ".mtar", len, out);
    if (!mtarFile) return NULL;
    return checkMagic(mtarFile) ? mtarFile : NULL;
}

inline
RichQuat angleAxisToQuat(const float& angle, RichVec3 axis) {
    RichQuat quat;

    axis.Normalize();
    double sinAngle = sin(angle / 2.0f);

    quat[0] = axis[0] * sinAngle;
    quat[1] = axis[1] * sinAngle;
    quat[2] = axis[2] * sinAngle;
    quat[3] = cos(angle / 2.0f);

    return quat;
}

inline
std::vector<RotAnimation> readRotBitstream(uint16_t* rotBitStream, const int& size, const int& quatLength, const uint32_t& numFrames) {
    int keyFrame = 0;
    std::vector<RotAnimation> ra;
    RichBitStream bs = RichBitStream(rotBitStream, size * 2);

    while (keyFrame < numFrames) {
        keyFrame += bs.ReadBits(8);

        float theta = shiftRadix(bs.ReadBits(quatLength), -quatLength) * g_mgs4_PI;
        float axisX = shiftRadix(bs.ReadBits(quatLength), -quatLength);
        float axisY = shiftRadix(bs.ReadBits(quatLength), -quatLength);

        bool negCheck = bs.ReadBits(1);
        bool negCheck2 = bs.ReadBits(1);
        bool negCheck3 = bs.ReadBits(1);

        float axisZ = 1.0f - axisX - axisY;

        if (negCheck) { axisX = -axisX; }
        if (negCheck2) { axisY = -axisY; }
        if (negCheck3) { axisZ = -axisZ; }

        RichQuat qt = angleAxisToQuat(theta, { axisX, axisY, axisZ });

        ra.push_back({ keyFrame, qt[0], qt[1], qt[2], qt[3] });
    }
   
    return ra;
}

inline
std::vector<MoveAnimation> readMoveBitstream(uint16_t* moveBitStream, const int& size, const uint32_t& numFrames) {
    int keyFrame = 0;
    std::vector<MoveAnimation> ma;
    RichBitStream bs = RichBitStream(moveBitStream, size * 2);

    while (keyFrame < numFrames) {
        float x = short2HalfExp(bs.ReadBits(16));
        float y = short2HalfExp(bs.ReadBits(16));
        float z = short2HalfExp(bs.ReadBits(16));
        keyFrame += bs.ReadBits(8);
        ma.push_back({ keyFrame, x , y, z });
    }
    
    return ma;
}

inline
noeKeyFrameData_t createTransKFData(const MoveAnimation& trans, std::vector<float>& aniData) {
    noeKeyFrameData_t data = {};
    data.dataIndex = aniData.size();
    data.time = trans.keyframe / g_mgs4_GAME_FRAMERATE;

    aniData.push_back(trans.x);
    aniData.push_back(trans.y);
    aniData.push_back(trans.z);

    return data;
}

inline
noeKeyFrameData_t createRotKFData(const RotAnimation& rot, std::vector<float>& aniData) {
    noeKeyFrameData_t data = {};
    data.dataIndex = aniData.size();
    data.time = rot.keyframe / g_mgs4_GAME_FRAMERATE;

    RichQuat quat = { rot.x, rot.y, rot.z, rot.w };
    quat.Transpose();

    aniData.push_back(quat[0]);
    aniData.push_back(quat[1]);
    aniData.push_back(quat[2]);
    aniData.push_back(quat[3]);

    return data;
}

inline
std::vector<noeKeyFrameData_t> createKFData(const std::vector<MoveAnimation>& trans, const std::vector<RotAnimation>& rot, const std::vector<MoveAnimation>& scale, std::vector<float>& aniData) {
    std::vector<noeKeyFrameData_t> kfData;

    for (int i = 0; i < trans.size(); i++) {
        noeKeyFrameData_t transData = createTransKFData(trans[i], aniData);
        kfData.push_back(transData);
    }

    for (int i = 0; i < rot.size(); i++) {
        noeKeyFrameData_t rotData = createRotKFData(rot[i], aniData);
        kfData.push_back(rotData);
    }

    for (int i = 0; i < scale.size(); i++) {
        noeKeyFrameData_t scaleData = createTransKFData(scale[i], aniData);
        kfData.push_back(scaleData);
    }

    return kfData;
}

inline
noeKeyFramedBone_t createKFBone(uint32_t boneID, modelBone_t* noeBones, int numBones, int numFrames, const std::vector<MoveAnimation>& trans, const std::vector<RotAnimation>& rot, const std::vector<MoveAnimation>& scale, std::vector<float>& aniData, std::vector<std::vector<noeKeyFrameData_t>>& kfData) {
    noeKeyFramedBone_t kfBone = {};

    std::string boneStrcode = intToHexString(boneID);
    int boneIdx = findBoneIdx((char*)boneStrcode.c_str(), noeBones, numBones);

    if (boneIdx == -1) return kfBone;

    kfBone.boneIndex = boneIdx;

    kfBone.scaleInterpolation = NOEKF_INTERPOLATE_LINEAR;
    kfBone.rotationInterpolation = NOEKF_INTERPOLATE_LINEAR;
    kfBone.translationInterpolation = NOEKF_INTERPOLATE_LINEAR;

    kfBone.scaleType = NOEKF_SCALE_VECTOR_3;
    kfBone.rotationType = NOEKF_ROTATION_QUATERNION_4;
    kfBone.translationType = NOEKF_TRANSLATION_VECTOR_3;

    kfBone.numScaleKeys = scale.size();
    kfBone.numRotationKeys = rot.size();
    kfBone.numTranslationKeys = trans.size();

    kfBone.maxTime = numFrames / g_mgs4_GAME_FRAMERATE;

    std::vector<noeKeyFrameData_t> nkfData = createKFData(trans, rot, scale, aniData);
    kfData.push_back(nkfData);

    int transPos = 0;
    int rotPos = (trans.size());
    int scalePos = (trans.size() + rot.size());

    if (kfBone.numScaleKeys)        kfBone.scaleKeys = &kfData[kfData.size() - 1][scalePos];
    if (kfBone.numRotationKeys)     kfBone.rotationKeys = &kfData[kfData.size() - 1][rotPos];
    if (kfBone.numTranslationKeys)  kfBone.translationKeys = &kfData[kfData.size() - 1][transPos];

    return kfBone;
}

inline
noeKeyFramedAnim_t createKFAnim(char* animName, modelBone_t* noeBones, int numBones, std::vector<noeKeyFramedBone_t>& kfBones, std::vector<float>& data) {
    noeKeyFramedAnim_t kfAnim = {};

    kfAnim.name = animName;
    kfAnim.numKfBones = kfBones.size();
    kfAnim.kfBones = kfBones.data();
    kfAnim.numBones = numBones;
    kfAnim.framesPerSecond = g_mgs4_GAME_FRAMERATE;

    kfAnim.data = data.data();
    kfAnim.numDataFloats = data.size();

    return kfAnim;
}


inline
void bindMtex(uint8_t* mtex, modelBone_t* noeBones, int numBones, std::vector <std::vector<noeKeyFrameData_t>>& kfData, std::vector<noeKeyFramedBone_t>& kfBones, std::vector<float>& aniData) {
    MtexHeader* mtexHeader = (MtexHeader*)mtex;

    for (int i = 0; i < mtexHeader->numMTP; i++) {
        MtpData* mtp = (MtpData*)&mtexHeader->mtpData;

        std::vector<RotAnimation>  rot;
        std::vector<MoveAnimation> trans;
        std::vector<MoveAnimation> scale;

        int movSize = mtp[i].rotOffset - mtp[i].moveOffset;
        int moveOffset = mtp[i].moveOffset * 2;
        uint16_t* moveBitstream = (uint16_t*)&mtex[moveOffset];
        trans = readMoveBitstream(moveBitstream, movSize, mtexHeader->numFrames);

        int rotSize = (i == mtexHeader->numMTP - 1) ? mtexHeader->dataSize - mtp[i].rotOffset : mtp[i + 1].moveOffset - mtp[i].rotOffset;
        int rotOffset = mtp[i].rotOffset * 2;
        uint16_t* rotBitstream = (uint16_t*)&mtex[rotOffset];
        rot = readRotBitstream(rotBitstream, rotSize, mtexHeader->quatBits, mtexHeader->numFrames);

        noeKeyFramedBone_t kfBone = createKFBone(mtp[i].boneName, noeBones, numBones, mtexHeader->numFrames, trans, rot, scale, aniData, kfData);
        kfBones.push_back(kfBone);
    }
}


inline
noesisAnim_t* bindMtcm(uint8_t* mtcm, uint8_t* mtex, uint32_t* boneTable, noeRAPI_t* rapi, modelBone_t* noeBones, int numBones) {
    MtcmHeader* mtcmHeader = (MtcmHeader*)mtcm;
    
    uint8_t* mtcmArchive    = &mtcm[mtcmHeader->archiveOffset];
    uint8_t* jointIndicies  = &mtcm[mtcmHeader->indiciesOffset];
    uint8_t* jointIndicies2 = &mtcm[mtcmHeader->indiciesOffset + mtcmHeader->numJoints];

    std::vector<float> aniData;
    std::vector<noeKeyFramedBone_t> kfBones;
    std::vector <std::vector<noeKeyFrameData_t>> kfData;

    RichBitStream bs = RichBitStream(&mtcmHeader->bitCheck0, 128);

    for (int j = 0; j < mtcmHeader->numJoints; j++) {
        int check = bs.ReadBits(1);
        std::vector<RotAnimation>  rot;
        std::vector<MoveAnimation> trans;
        std::vector<MoveAnimation> scale;

        if (j == 0) {
            uint16_t* moveBitstream = (uint16_t*)mtcmArchive;
            trans = readMoveBitstream(moveBitstream, mtcmHeader->quatOffset[0], mtcmHeader->numFrames);
        }

        int boneID = boneTable[jointIndicies[j]];
        int size = determineArchiveSize(mtcm, j, mtcmHeader->numJoints);
        int rotOffset = mtcmHeader->quatOffset[j];
        uint16_t* rotBitstream = (uint16_t*)&mtcmArchive[rotOffset * 2];

        int quatLength = check ? mtcmHeader->quatHighBit : mtcmHeader->quatLowBit;
        rot = readRotBitstream(rotBitstream, size, quatLength, mtcmHeader->numFrames);

        noeKeyFramedBone_t kfBone = createKFBone(boneID, noeBones, numBones, mtcmHeader->numFrames, trans, rot, scale, aniData, kfData);
        kfBones.push_back(kfBone);
    }

    //if (mtex) bindMtex(mtex, noeBones, numBones, kfData, kfBones, aniData);

    if (aniData.empty()) return NULL;

    std::string animName = intToHexString(mtcmHeader->name);
    noeKeyFramedAnim_t kfAnim = createKFAnim((char*)animName.c_str(), noeBones, numBones, kfBones, aniData);

    noesisAnim_t* anim = rapi->Noesis_AnimFromBonesAndKeyFramedAnim(noeBones, numBones, &kfAnim, true);
    return anim;
}

inline
void loadMotion(noeRAPI_t* rapi, BYTE* motionFile, modelBone_t* noeBones, int numBones) {
    MtarHeader* mtarHeader = (MtarHeader*)motionFile;

    if (mtarHeader->magic == 0x4D746172) swapEndianMtar(motionFile);

    uint8_t* mtcm = (uint8_t*)&motionFile[mtarHeader->mtcmOffset];
    uint8_t* mtex = (uint8_t*)&motionFile[mtarHeader->mtexOffset];
    MtarData* mtcmTable = (MtarData*)&motionFile[mtarHeader->dataTableOffset];
    uint32_t* boneTable = (uint32_t*)&motionFile[mtarHeader->boneNameTableOffset];

    CArrayList<noesisAnim_t*> animList;

    for (int i = 0; i < mtarHeader->numMotion; i++) {
        uint8_t* thisMtcm = &mtcm[mtcmTable[i].mtcmOffset];
        uint8_t* thisMtex = mtcmTable[i].mtexSize ? &mtex[mtcmTable[i].mtexOffset] : NULL;

        noesisAnim_t* anim = bindMtcm(thisMtcm, thisMtex, boneTable, rapi, noeBones, numBones);
        if (anim) animList.Append(anim);
    }

    noesisAnim_t* anims = rapi->Noesis_AnimFromAnimsList(animList, animList.Num());
    rapi->rpgSetExData_AnimsNum(anims, 1);
}