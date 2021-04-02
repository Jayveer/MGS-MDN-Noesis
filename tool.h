#pragma once
#include "mat.h"
#include "bone.h"
#include "mesh.h"
#include "face.h"
#include "motion.h"

bool g_mgs4MtarPrompt = false;
const char* g_mgs4plugin_name = "Metal Gear Solid 4";

inline
int genericToolSet(bool& setting, int toolIdx) {
    setting = !setting;
    g_nfn->NPAPI_CheckToolMenuItem(toolIdx, setting);
    return 1;
}

int mgs4_anim_prompt(int toolIdx, void* user_data) {
    return genericToolSet(g_mgs4MtarPrompt, toolIdx);
}

int mgs4_highlight_missing(int toolIdx, void* user_data) {
    return genericToolSet(g_mgs4HighlightMissingTex, toolIdx);
}

inline
int mgs4_layer_as_diffuse(int toolIdx, void* user_data) {
    return genericToolSet(g_mgs4LayerAsDiffuse, toolIdx);
}

inline
int mgs4_render_vertex_colour(int toolIdx, void* user_data) {
    return genericToolSet(g_mgs4RenderVertexColour, toolIdx);
}

inline
int makeTool(char* toolDesc, int (*toolMethod)(int toolIdx, void* userData)) {
    int handle = g_nfn->NPAPI_RegisterTool(toolDesc, toolMethod, NULL);
    g_nfn->NPAPI_SetToolSubMenuName(handle, g_mgs4plugin_name);
    return handle;
}

inline
void applyTools() {
    makeTool("Prompt for Motion Archive", mgs4_anim_prompt);
    makeTool("Highlight Missing Textures", mgs4_highlight_missing);
    makeTool("Render Layer Map as Diffuse", mgs4_layer_as_diffuse);
    makeTool("Render Vertex Colour", mgs4_render_vertex_colour);
}