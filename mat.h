#pragma once
#include "mgs/model/mdn/mdn.h"
#include "mgs/common/strcode.h"
#include "mgs/texture/dlz/dlz.h"
#include "mgs/texture/txn/txn.h"
#include "noesis/plugin/pluginshare.h"

bool g_mgs4SkipNormalMaps = false;
bool g_mgs4HighlightMissingTex = false;

inline
void cacheDelayload(noeRAPI_t* rapi, std::vector<DldInfo>& dldInfo) {
    std::filesystem::path p{ rapi->Noesis_GetInputName() };
    p = p.parent_path();

    for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(p)) {
        if (file.path().extension() == ".dlz") {
            std::string dlzFile = file.path().u8string();
            Dlz dlz(dlzFile);
            dlz.setRapi(rapi);

            int size;
            uint8_t* dld = dlz.decompress(size);
            dldInfo.push_back({ dld, size });
        }
    }
}

inline
std::string findTxn(noeRAPI_t* rapi, uint32_t& strcode) {
    std::filesystem::path p{ rapi->Noesis_GetInputName() };
    p = p.parent_path();

    for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(p)) {
        if (file.path().extension() == ".txn") {
            Txn txn = Txn(file.path().u8string());
            if (txn.containsTexture(strcode))
                return file.path().u8string();
        }
    }

    return "";
}

inline
void swapChannels(uint8_t* src, int size) {
    for (int i = 0; i < size; i += 4) {
        src[i + 0] = src[i + 3];
        src[i + 2] = 0xFF;
        src[i + 3] = 0xFF;
    }
}

inline
void decompNormals(std::vector<uint32_t>& normalMaps, CArrayList<noesisTex_t*>& texList) {
    sort(normalMaps.begin(), normalMaps.end());
    normalMaps.erase(unique(normalMaps.begin(), normalMaps.end()), normalMaps.end());

    for (int i = 0; i < normalMaps.size(); i++) {
        noesisTex_t* tex = texList[normalMaps[i]];
        swapChannels(tex->data, tex->dataLen);
    }
}

inline
noesisTex_t* loadReplacementTex(noeRAPI_t* rapi) {
    uint8_t tgaFusch[50] = { 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x10, 
                             0x01, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 
                             0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C, 0x7B, 0x7C };

    uint8_t tgaWhite[50] = { 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x20,
                             0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


    uint8_t* replacementImage = g_mgs4HighlightMissingTex ? tgaFusch : tgaWhite;

    noesisTex_t* noeTexture = rapi->Noesis_LoadTexByHandler(replacementImage, 50, ".tga");
    return noeTexture;
}

inline
noesisTex_t* loadTexture(noeRAPI_t* rapi, uint32_t strcode, uint32_t txnStrcode, const std::vector<DldInfo>& dldInfo) {
    std::string txnFile = findTxn(rapi, strcode);
    if (txnFile == "") return NULL;

    Txn txn(txnFile);
    txn.setRapi(rapi);
    int size;
    uint8_t* tga = txn.getTexture(strcode, size, dldInfo);
    if (!tga) return NULL;
    noesisTex_t* tex = rapi->Noesis_LoadTexByHandler(tga, size, ".tga");
    rapi->Noesis_UnpooledFree(tga);
    return tex;
}

inline
void buildTextures(MdnTexture* texture, int32_t numTexture, noeRAPI_t* rapi, uint32_t txnStrcode, const std::vector<DldInfo>& dldInfo, CArrayList<noesisTex_t*>& texList) {

    for (int i = 0; i < numTexture; i++) {
        uint32_t strcode = _byteswap_ulong(texture[i].strcode);
        std::string texStr = intToHexString(strcode);
        char texName[7];
        strcpy_s(texName, texStr.c_str());

        noesisTex_t* noeTexture = loadTexture(rapi, strcode, txnStrcode, dldInfo);
        if (!noeTexture) noeTexture = loadReplacementTex(rapi);
        noeTexture->filename = rapi->Noesis_PooledString(texName);
        texList.Append(noeTexture);
    }
}

inline
void setMaterialIdx(MdnMaterial* material, noesisMaterial_t* noeMat, int idx, std::vector<uint32_t>& normalMaps) {
    uint32_t texIdx = _byteswap_ulong(material->texture[idx]);

    switch (idx) {
    case eTexture::DIFFUSETEX:
        noeMat->texIdx = texIdx;
        break;
    case eTexture::NORMALTEX:
        if (g_mgs4SkipNormalMaps) break;
        normalMaps.push_back(texIdx);
        noeMat->normalTexIdx = texIdx;
        break;
    case eTexture::SPECULARTEX:
        noeMat->specularTexIdx = texIdx;
        break;
    case eTexture::SPECULAR_GRADIENTTEX:
        break;
    case eTexture::INCIDENCETEX:
        break;
    case eTexture::REFLECTTEX:
        break;
    case eTexture::WRINKLETEX:
        break;
    case eTexture::UNKNOWNTEX:
        break;
    }
}

inline
void buildMaterials(MdnMaterial* material, int32_t numMaterial, noeRAPI_t* rapi, CArrayList<noesisMaterial_t*>& matList, std::vector<uint32_t>& normalMaps) {

    for (int i = 0; i < numMaterial; i++) {
        std::string matStr = intToHexString(_byteswap_ulong(material[i].strcode));
        char matName[7];
        strcpy_s(matName, matStr.c_str());

        noesisMaterial_t* noeMat = rapi->Noesis_GetMaterialList(1, false);
        noeMat->name = rapi->Noesis_PooledString(matName);

        int32_t numTexture = _byteswap_ulong(material[i].numTexture);
        for (int j = 0; j < numTexture; j++) {
            setMaterialIdx(&material[i], noeMat, j, normalMaps);
        }

        matList.Append(noeMat);
    }
}