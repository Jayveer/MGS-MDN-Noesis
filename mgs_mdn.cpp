#include "tool.h"

const char* g_pPluginName = "mgs_mdn";
const char* g_pPluginDesc = "Metal Gear Solid 4 MDN handler by Jayveer.";

bool checkMDN(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi) {
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x204E444D;
}

SNoeSceneLight setLight() {
    SNoeSceneLight light;
    light.mPos[0] = 10000;
    light.mPos[1] = 10000;
    light.mPos[2] = 10000;
    light.mPos[3] = 10000;
    return light;
}

noesisModel_t* loadMDN(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi) {
    void* ctx = rapi->rpgCreateContext();
    rapi->rpgSetTriWinding(1);

    MdnHeader*   header       = (MdnHeader*   )fileBuffer;

    bool isBigEndian = header->fileSize != bufferLen;
    if (isBigEndian) swapEndianMDN(fileBuffer);

    MdnBone*     bones        = (MdnBone*    )&fileBuffer[header->boneOffset];
    MdnMesh*     mesh         = (MdnMesh*    )&fileBuffer[header->meshOffset];
    MdnFace*     face         = (MdnFace*    )&fileBuffer[header->faceOffset];
    MdnSkin*     skin         = (MdnSkin*    )&fileBuffer[header->skinOffset];
    MdnGroup*    group        = (MdnGroup*   )&fileBuffer[header->groupOffset];
    MdnTexture*  texture      = (MdnTexture* )&fileBuffer[header->textureOffset];
    MdnMaterial* material     = (MdnMaterial*)&fileBuffer[header->materialOffset];
    uint8_t*     faceBuffer   = (uint8_t*    )&fileBuffer[header->faceBufferOffset];
    uint8_t*     vertexBuffer = (uint8_t*    )&fileBuffer[header->vertexBufferOffset];

    MdnVertexDefinition* vertexDefinition = (MdnVertexDefinition*)&fileBuffer[header->vertexDefinitionOffset];

    modelBone_t* noeBones = header->numBones ? bindBones(bones, header->numBones, rapi) : NULL;

    std::vector<DldInfo> dldInfo; 
    cacheDelayload(rapi, dldInfo, isBigEndian);

    CArrayList<noesisTex_t*>      texList; 
    CArrayList<noesisMaterial_t*> matList; 
    std::vector<uint32_t>         normalMaps;

    buildTextures(texture, header->numTexture, rapi, header->filename, dldInfo, texList, isBigEndian);
    buildMaterials(material, header->numMaterial, rapi, matList, normalMaps);
    decompNormals(normalMaps, texList);

    for (int i = 0; i < header->numMesh; i++) {
        std::vector<float> normals, tangents, weights;
        std::vector<uint8_t> bones;

        bindMesh(&mesh[i], skin, group, &vertexDefinition[i], vertexBuffer, rapi, normals, weights, tangents, bones);
        bindFace(&mesh[i], face, faceBuffer, rapi);
        rapi->rpgUnifyBinormals(true);
        rapi->rpgClearBufferBinds();
    }

   noesisMatData_t* md = rapi->Noesis_GetMatDataFromLists(matList, texList);
   rapi->rpgSetExData_Materials(md);

    if (g_mgs4MtarPrompt && header->numBones) {
        BYTE* motionFile = openMotion(rapi);
        if (motionFile) loadMotion(rapi, motionFile, noeBones, header->numBones);
    }

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    deleteCache(dldInfo);

    rapi->rpgDestroyContext(ctx);
    return mdl;
}

bool NPAPI_InitLocal(void) {
    int fh = g_nfn->NPAPI_Register("Metal Gear Solid 4", ".mdn");
    if (fh < 0) return false;

    g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, checkMDN);
    g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, loadMDN);

    applyTools();

    return true;
}


void NPAPI_ShutdownLocal(void) {

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}