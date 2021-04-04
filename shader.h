#pragma once

bool g_mgs4LayerAsDiffuse = false;

struct shaderParams {
    bool useUV1;
    bool noSpec;
    bool hasEnv;
    bool noNorm;
    bool noDiff;
};

shaderParams setParams(uint32_t flag) {
    shaderParams params;
    params.useUV1 = (flag >> 16) & 1;
    params.noSpec = (flag >> 17) & 1;
    params.hasEnv = (flag >> 20) & 1;
    params.noNorm = false;
    params.noDiff = false;
    return params;
}

inline
void setDiffuse(noesisMaterial_t* noeMat, int texIdx, int channel) {
    noeMat->texIdx = texIdx;
}

inline
void setOpacity(noesisMaterial_t* noeMat, int texIdx, int channel) {
    noeMat->transTexIdx = texIdx;
}

inline
void setNormal(noesisMaterial_t* noeMat, int texIdx, int channel, std::vector<uint32_t>& normalMaps) {
    normalMaps.push_back(texIdx);
    noeMat->normalTexIdx = texIdx;
    if (channel == 1) noeMat->flags |= NMATFLAG_NORMAL_UV1;
}

inline
void setSubNormal(noesisMaterial_t* noeMat, int texIdx, int channel, std::vector<uint32_t>& normalMaps) {
    normalMaps.push_back(texIdx);
    noeMat->bumpTexIdx = texIdx;
}

inline
void setSpec(noesisMaterial_t* noeMat, int texIdx, int channel) {
    noeMat->specularTexIdx = texIdx;
    if (channel == 1) noeMat->flags |= NMATFLAG_SPEC_UV1;
}

inline
void setEnv(noesisMaterial_t* noeMat, int texIdx, int channel) {
    noeMat->envTexIdx = texIdx;
}

inline
void setLayer(noesisMaterial_t* noeMat, int texIdx, int channel) {
    if (g_mgs4LayerAsDiffuse)
        noeMat->texIdx = texIdx;
}

inline
int basicShader(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps, shaderParams& params) {
    int i = 0;

    if (i < maxTex && !params.noDiff) setDiffuse(noeMat, material->texture[i++], i);
    if (i < maxTex && !params.noNorm)  setNormal(noeMat, material->texture[i++], i, normalMaps);
    if (i < maxTex && !params.noSpec)    setSpec(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader000(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    if (params.noSpec && params.hasEnv) {        
        int i = 0;
        if (i < maxTex) setDiffuse(noeMat, material->texture[i++], i);
        if (i < maxTex) i++; // sometimes it isnt a normal
        return i;
    }  

    params.useUV1 = false;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader001(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader002(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader003(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    //params.noNorm = true; //sometimes has normal but not always
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setLayer(noeMat, material->texture[i++], i); //layer texture goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader005(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader006(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    i++; //layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader007(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader010(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setLayer(noeMat, material->texture[i++], i);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    return i;
}

inline
int useShader012(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //layer texture goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader016(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //cube map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader019(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //layer map goes here
    i++; //unknown map goes here
    i++; //unknown map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader020(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //cube map goes here
    i++; //layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader033(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader036(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    i++; //layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader042(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //incidence map goes here
    i++; //layer map goes here
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader048(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    return i;
}

inline
int useShader049(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    return i;
}


inline
int useShader065(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //incidence map goes here
    i++; //unknown map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader080(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //specular gradient goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

//helicopter
inline
int useShader082(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //specular gradient goes here
    i++; //cube map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader096(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //specular gradient goes here
    i++; //incidence map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader100(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader160(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //unknown map goes here
    i++; //unknown map goes here
    i++; //unknown map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader162(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; //unknown map goes here, env
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader163(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader164(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    i++; // layer map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader165(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // cube map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader166(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = 0;
    setDiffuse(noeMat, -1, i);
    setOpacity(noeMat, material->texture[i], i);
    if (i < maxTex && !params.noSpec)  setSpec(noeMat, material->texture[i++], i);
    if (i < maxTex && !params.noNorm) setNormal(noeMat, material->texture[i++], i, normalMaps);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}


inline
int useShader168(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader171(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // cube map goes here
    i++; // specular gradient goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader224(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noNorm = params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader240(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // specular gradient goes here
    i++; // layer
    i++; // layer norm
    i++; // incidence map goes here
    i++; // unknown map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}


inline
int useShader241(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // specular gradient goes here
    i++; // incidence map goes here
    i++; // unknown map goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader243(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    params.noSpec = true;
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
int useShader245(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {
    shaderParams params = setParams(material->flag);
    int i = basicShader(material, noeMat, maxTex, normalMaps, params);
    i++; // incidence map goes here
    i++; // layer map goes here
    if (i < maxTex) setSubNormal(noeMat, material->texture[i++], i, normalMaps);
    i++; // 3rd normal goes here
    if (i < maxTex && params.hasEnv) setEnv(noeMat, material->texture[i++], i);
    return i;
}

inline
void shaderFactory(MdnMaterial* material, noesisMaterial_t* noeMat, int maxTex, std::vector<uint32_t>& normalMaps) {

    switch (material->flag & 0xFF) {
    case 0x00:
        useShader000(material, noeMat, maxTex, normalMaps); break;
    case 0x01:
    case 0x04:
        useShader001(material, noeMat, maxTex, normalMaps); break;
    case 0x02:
        useShader002(material, noeMat, maxTex, normalMaps); break;
    case 0x03:
        useShader003(material, noeMat, maxTex, normalMaps); break;
    case 0x05:
        useShader005(material, noeMat, maxTex, normalMaps); break;
    case 0x06:
        useShader006(material, noeMat, maxTex, normalMaps); break;
    case 0x07:
        useShader007(material, noeMat, maxTex, normalMaps); break;
    case 0x0A:
        useShader010(material, noeMat, maxTex, normalMaps); break;
    case 0x0C:
        useShader012(material, noeMat, maxTex, normalMaps); break;
    case 0x10:
        useShader016(material, noeMat, maxTex, normalMaps); break;
    case 0x13:
        useShader019(material, noeMat, maxTex, normalMaps); break;
    case 0x14:
        useShader020(material, noeMat, maxTex, normalMaps); break;
    case 0x21:
        useShader033(material, noeMat, maxTex, normalMaps); break;
    case 0x24:
        useShader036(material, noeMat, maxTex, normalMaps); break;
    case 0x2A:
        useShader042(material, noeMat, maxTex, normalMaps); break;
    case 0x30:
        useShader048(material, noeMat, maxTex, normalMaps); break;
    case 0x31:
        useShader049(material, noeMat, maxTex, normalMaps); break;
    case 0x41:
        useShader065(material, noeMat, maxTex, normalMaps); break;
    case 0x50:
    case 0x51:
        useShader080(material, noeMat, maxTex, normalMaps); break;
    case 0x52:
        useShader082(material, noeMat, maxTex, normalMaps); break;
    case 0x60:
    case 0x61:
        useShader096(material, noeMat, maxTex, normalMaps); break;
    case 0x64:
        useShader100(material, noeMat, maxTex, normalMaps); break;
    case 0xA0:
        useShader160(material, noeMat, maxTex, normalMaps); break;
    case 0xA2:
        useShader162(material, noeMat, maxTex, normalMaps); break;
    case 0xA3:
        useShader163(material, noeMat, maxTex, normalMaps); break;
    case 0xA4:
        useShader164(material, noeMat, maxTex, normalMaps); break;
    case 0xA5:
        useShader165(material, noeMat, maxTex, normalMaps); break;
    case 0xA6:
        useShader166(material, noeMat, maxTex, normalMaps); break;
    case 0xA8:
        useShader168(material, noeMat, maxTex, normalMaps); break;
    case 0xAB:
        useShader171(material, noeMat, maxTex, normalMaps); break;
    case 0xE0:
    case 0xE1:
        useShader224(material, noeMat, maxTex, normalMaps); break;
    case 0xF0:
        useShader240(material, noeMat, maxTex, normalMaps); break;
    case 0xF1:
        useShader241(material, noeMat, maxTex, normalMaps); break;
    case 0xF3:
        useShader243(material, noeMat, maxTex, normalMaps); break;
    case 0xF5:
        useShader245(material, noeMat, maxTex, normalMaps); break;
    default:
        useShader001(material, noeMat, maxTex, normalMaps);
    }
}