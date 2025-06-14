#pragma once

#ifndef NB_RESEARCH_SCN_COMPUTE
#define NB_RESEARCH_SCN_COMPUTE

#include "nb/scene/NBScnRenderJob.h"

#ifdef __cplusplus
extern "C" {
#endif

//generate
int research_scn_compute_gen_random_arrays(const int count, STNBScnRenderJobTree* dst);
int research_scn_compute_validate_arrays(const STNBScnRenderJobTree* src);
//run
void research_scn_compute_convertTreeToPlainGpuAlgorithmToDst(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst);
//compare
float research_scn_compute_compare(const STNBScnRenderJobPlain* src, const STNBScnRenderJobPlain* other, const float maxDiffAbs, const STNBScnRenderJobTree* optSrc);
//print
void research_scn_compute_print_tree_node(const STNBScnTreeNode* src, const char* prefix);
void research_scn_compute_print_flat_node(const STNBScnFlatNode* src, const char* prefix);
void research_scn_compute_print_flat_job(const STNBScnRenderJobPlain* src, const unsigned int spacesPerLvl);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //NB_RESEARCH_SCN_COMPUTE
