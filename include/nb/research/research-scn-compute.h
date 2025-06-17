#pragma once

#ifndef NB_RESEARCH_SCN_COMPUTE
#define NB_RESEARCH_SCN_COMPUTE

#include "nb/scene/NBScnRenderJobTree.h"
#include "nb/scene/NBScnRenderJobFlat.h"

#ifdef __cplusplus
extern "C" {
#endif

//generate
int research_scn_compute_gen_random_arrays(const int count, STNBScnRenderJobTree* dst);
int research_scn_compute_validate_arrays(const STNBScnRenderJobTree* src);
//run
void research_scn_compute_convertTreeToPlainGpuAlgorithmToDst(const BYTE* hdr, const BYTE* tree, BYTE* flat, const UI32 ammNodes);
//compare
float research_scn_compute_compare(const STNBScnRenderJobFlat* src, const STNBScnRenderJobFlat* other, const float maxDiffAbs, const STNBScnRenderJobTree* optSrc);
//print
void research_scn_compute_print_tree_node(const STNBScnTreeNode* src, const char* prefix);
void research_scn_compute_print_flat_node(const STNBScnFlatNode* src, const char* prefix);
void research_scn_compute_print_flat_job(const STNBScnRenderJobFlat* src, const unsigned int spacesPerLvl);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //NB_RESEARCH_SCN_COMPUTE
