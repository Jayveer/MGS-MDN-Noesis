#include "tool.h"

const char* g_pPluginName = "mgs_mdn";
const char* g_pPluginDesc = "Metal Gear Solid 4 MDN handler by Jayveer.";

bool checkMDN(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi) {
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x204E444D;
}

noesisModel_t* loadMDN(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi) {
    void* ctx = rapi->rpgCreateContext();
    rapi->rpgSetTriWinding(1);
    rapi->rpgSetEndian(1);

    MdnHeader*   header       = (MdnHeader*   )fileBuffer;
    MdnBone*     bones        = (MdnBone*    )&fileBuffer[_byteswap_ulong(header->boneOffset)];
    MdnMesh*     mesh         = (MdnMesh*    )&fileBuffer[_byteswap_ulong(header->meshOffset)];
    MdnFace*     face         = (MdnFace*    )&fileBuffer[_byteswap_ulong(header->faceOffset)];
    MdnSkin*     skin         = (MdnSkin*    )&fileBuffer[_byteswap_ulong(header->skinOffset)];
    MdnGroup*    group        = (MdnGroup*   )&fileBuffer[_byteswap_ulong(header->groupOffset)];
    MdnTexture*  texture      = (MdnTexture* )&fileBuffer[_byteswap_ulong(header->textureOffset)];
    MdnMaterial* material     = (MdnMaterial*)&fileBuffer[_byteswap_ulong(header->materialOffset)];
    uint8_t*     faceBuffer   = (uint8_t*    )&fileBuffer[_byteswap_ulong(header->faceBufferOffset)];
    uint8_t*     vertexBuffer = (uint8_t*    )&fileBuffer[_byteswap_ulong(header->vertexBufferOffset)];
    MdnVertexDefinition* vertexDefinition = (MdnVertexDefinition*)&fileBuffer[_byteswap_ulong(header->vertexDefinitionOffset)];

    int32_t numMesh  = _byteswap_ulong(header->numMesh );
    int32_t numBones = _byteswap_ulong(header->numBones);

    modelBone_t* noeBones = (numBones) ? bindBones(bones, numBones, rapi) : NULL;

    std::vector<DldInfo> dldInfo; 
    cacheDelayload(rapi, dldInfo);

    CArrayList<noesisTex_t*>      texList; 
    CArrayList<noesisMaterial_t*> matList; 
    std::vector<uint32_t>         normalMaps;

    buildTextures(texture, _byteswap_ulong(header->numTexture), rapi, header->filename, dldInfo, texList);
    buildMaterials(material, _byteswap_ulong(header->numMaterial), rapi, matList, normalMaps);
    decompNormals(normalMaps, texList);

    for (int i = 0; i < numMesh; i++) {
        std::vector<float> normals;
        std::vector<float> tangents;
        std::vector<float> weights;
        std::vector<uint8_t> bones;

        bindMesh(&mesh[i], skin, group, &vertexDefinition[i], vertexBuffer, rapi, normals, weights, tangents, bones);
        bindFace(&mesh[i], face, faceBuffer, rapi);

        rapi->rpgClearBufferBinds();
    }

    noesisMatData_t* md = rapi->Noesis_GetMatDataFromLists(matList, texList);
    rapi->rpgSetExData_Materials(md);

    if (g_mgs4MtarPrompt && numBones) {
        BYTE* motionFile = openMotion(rapi);
        if (motionFile) loadMotion(rapi, motionFile, noeBones, numBones);
    }

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    for (int i = 0; i < dldInfo.size(); i++) {
        delete[] dldInfo[i].dld;
    }

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