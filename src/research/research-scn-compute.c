
#include "nb/NBFrameworkPch.h"
#include "nb/research/research-scn-compute.h"
#include <stdio.h>
#include <stdlib.h> //defines NULL and rand()
#include <memory.h> //memset
#include <time.h>   //time()
#include <assert.h>	//assert

//generate

void research_scn_compute_gen_random_src_itm_children(STNBScnRenderJobTree* dst, int* nxtNodeIdx, int* nxtV0, int* nxtV1, int* nxtV2, int* nxtV3, STNBScnTreeNode* n, const unsigned int nIdx, const unsigned int nLvl);
void research_scn_compute_gen_random_src_itm(STNBScnRenderJobTree* dst, STNBScnTreeNode* n, int* nxtV0, int* nxtV1, int* nxtV2, int* nxtV3);

int research_scn_compute_gen_random_arrays(const int count, STNBScnRenderJobTree* dst) {
	int r = 0;
	//empty
	NBScnRenderJobTree_empty(dst);
	//seed randizer
	srand((UI32)time(0));
	//populate
	if (count > 0) {
		const int v0Len = (rand() % (count * 3));
		const int v1Len = (rand() % (count * 3));
		const int v2Len = (rand() % (count * 3));
		const int v3Len = (rand() % (count * 3));
		//allocate spaces
		{
			NBArray_addItems(&dst->nodes, NULL, sizeof(STNBScnTreeNode), count);
			memset(NBArray_dataPtr(&dst->nodes, STNBScnTreeNode), 0, sizeof(STNBScnTreeNode) * count);
			//
			NBArray_addItems(&dst->verts.v, NULL, sizeof(STNBScnVertex), v0Len);
			memset(NBArray_dataPtr(&dst->verts.v, STNBScnVertex), 0, sizeof(STNBScnVertex) * v0Len);
			//
			NBArray_addItems(&dst->verts.v1, NULL, sizeof(STNBScnVertexTex), v1Len);
			memset(NBArray_dataPtr(&dst->verts.v1, STNBScnVertexTex), 0, sizeof(STNBScnVertexTex) * v1Len);
			//
			NBArray_addItems(&dst->verts.v2, NULL, sizeof(STNBScnVertexTex2), v2Len);
			memset(NBArray_dataPtr(&dst->verts.v2, STNBScnVertexTex2), 0, sizeof(STNBScnVertexTex2) * v2Len);
			//
			NBArray_addItems(&dst->verts.v3, NULL, sizeof(STNBScnVertexTex3), v3Len);
			memset(NBArray_dataPtr(&dst->verts.v3, STNBScnVertexTex3), 0, sizeof(STNBScnVertexTex3) * v3Len);
		}
		//generate
		{
			int i = 0, iV0 = 0, iV1 = 0, iV2 = 0, iV3 = 0;
			while (i < count) {
				const int nIdx = i;
				STNBScnTreeNode* n = NBArray_itmPtrAtIndex(&dst->nodes, STNBScnTreeNode, nIdx);
				//next
				i++;
				//populate
				n->iParent = nIdx; //root nodes point to themself
				research_scn_compute_gen_random_src_itm(dst, n, &iV0, &iV1, &iV2, &iV3);
				//action
				if ((rand() % 100) < 75) {
					//add children
					research_scn_compute_gen_random_src_itm_children(dst, &i, &iV0, &iV1, &iV2, &iV3, n, nIdx, 0);
				}
			}
			//truncate unsued vertices
			if (iV0 < dst->verts.v.use) {
				printf("Truncating verts-0 from %d to %d (-%d).\n", dst->verts.v.use, iV0, dst->verts.v.use - iV0);
				NBArray_truncateBuffSize(&dst->verts.v, iV0);
			}
			if (iV1 < dst->verts.v1.use) {
				printf("Truncating verts-1 from %d to %d (-%d).\n", dst->verts.v1.use, iV1, dst->verts.v1.use - iV1);
				NBArray_truncateBuffSize(&dst->verts.v1, iV1);
			}
			if (iV2 < dst->verts.v2.use) {
				printf("Truncating verts-2 from %d to %d (-%d).\n", dst->verts.v2.use, iV2, dst->verts.v2.use - iV2);
				NBArray_truncateBuffSize(&dst->verts.v2, iV2);
			}
			if (iV3 < dst->verts.v3.use) {
				printf("Truncating verts-3 from %d to %d (-%d).\n", dst->verts.v3.use, iV3, dst->verts.v3.use - iV3);
				NBArray_truncateBuffSize(&dst->verts.v3, iV3);
			}
			//return value
			r = i;
		}
	}
	return r;
}

int research_scn_compute_validate_arrays(const STNBScnRenderJobTree* src) {
	const int nodesCount = src->nodes.use;
	int v0Count = 0, v1Count = 0, v2Count = 0, v3Count = 0;
	int i; for (i = 0; i < nodesCount; i++) {
		const STNBScnTreeNode* n = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, i);
		if (n->verts.count > 0) {
			switch (NBScnTreeNode_getVertsType(n)) {
				case 0:
					NBASSERT(n->verts.iFirst < src->verts.v.use && (n->verts.iFirst + n->verts.count) <= src->verts.v.use)
					v0Count += n->verts.count;
					break;
				case 1:
					NBASSERT(n->verts.iFirst < src->verts.v1.use && (n->verts.iFirst + n->verts.count) <= src->verts.v1.use)
					v1Count += n->verts.count;
					break;
				case 2:
					NBASSERT(n->verts.iFirst < src->verts.v2.use && (n->verts.iFirst + n->verts.count) <= src->verts.v2.use)
					v2Count += n->verts.count;
					break;
				case 3:
					NBASSERT(n->verts.iFirst < src->verts.v3.use && (n->verts.iFirst + n->verts.count) <= src->verts.v3.use)
					v3Count += n->verts.count;
					break;
				default:
					NBASSERT(FALSE) //unexpected value
					break;
			}
		}
	}
	NBASSERT(v0Count == src->verts.v.use); //all vertexs must be refered by nodes
	NBASSERT(v1Count == src->verts.v1.use); //all vertexs must be refered by nodes
	NBASSERT(v2Count == src->verts.v2.use); //all vertexs must be refered by nodes
	NBASSERT(v3Count == src->verts.v3.use); //all vertexs must be refered by nodes
	if (v0Count != src->verts.v.use) {
		printf("Vertex-0, error, vertices un-or-over-used by nodes.\n");
		return -1;
	} else if (v1Count != src->verts.v1.use) {
		printf("Vertex-1, error, vertices un-or-over-used by nodes.\n");
		return -1;
	} else if (v2Count != src->verts.v2.use) {
		printf("Vertex-2, error, vertices un-or-over-used by nodes.\n");
		return -1;
	} else if (v3Count != src->verts.v3.use) {
		printf("Vertex-3, error, vertices un-or-over-used by nodes.\n");
		return -1;
	}
	//
	return 0;
}

void research_scn_compute_gen_random_src_itm_children(STNBScnRenderJobTree* dst, int* nxtNodeIdx, int* nxtV0, int* nxtV1, int* nxtV2, int* nxtV3, STNBScnTreeNode* n, const unsigned int nIdx, const unsigned int nLvl) {
	//randomize action
	if(*nxtNodeIdx < dst->nodes.use) {
		unsigned int chldCount = 0;
		int action = 0;
		do {
			action = (rand() % 100);
			//add child
			const int cIdx = *nxtNodeIdx;
			STNBScnTreeNode* c = NBArray_itmPtrAtIndex(&dst->nodes, STNBScnTreeNode, cIdx);
			//next
			*nxtNodeIdx = *nxtNodeIdx + 1;
			//populate
			c->iParent = nIdx;
			research_scn_compute_gen_random_src_itm(dst, c, nxtV0, nxtV1, nxtV2, nxtV3);
			//add grandchilren
			if (action < 25) {
				research_scn_compute_gen_random_src_itm_children(dst, nxtNodeIdx, nxtV0, nxtV1, nxtV2, nxtV3, c, cIdx, nLvl + 1);
			}
			chldCount++;
		} while (*nxtNodeIdx < dst->nodes.use && action < 50);
		NBScnTreeNode_setChildCount(n, chldCount);
	}
}

void research_scn_compute_gen_random_src_itm(STNBScnRenderJobTree* dst, STNBScnTreeNode* n, int* nxtV0, int* nxtV1, int* nxtV2, int* nxtV3) {
	const int tMax = 100, tMaxScale = 4;
	const int rMaxScale = 4;
	const int sMax = 2, sMaxScale = 10;
	const int cMaxScale = 10;
	const int texMaxScale = 10;
	//properties
	const unsigned int isHidden = ((rand() % 10) == 0 ? 1 : 0);
	const unsigned int isDisabled = ((rand() % 10) == 0 ? 1 : 0);
	NBScnTreeNode_setIsHidden(n, isHidden);
	NBScnTreeNode_setIsDisabled(n, isDisabled);
	//transform
	{
		n->transform.x = (float)((rand() % (2 * (tMax * tMaxScale))) - (tMax * tMaxScale)) / (float)tMaxScale;
		n->transform.y = (float)((rand() % (2 * (tMax * tMaxScale))) - (tMax * tMaxScale)) / (float)tMaxScale;
		n->transform.deg = (float)((rand() % (2 * (360 * rMaxScale))) - (360 * rMaxScale)) / (float)rMaxScale;
		n->transform.sX = (float)((rand() % (2 * (sMax * sMaxScale))) - (sMax * sMaxScale)) / (float)sMaxScale;
		n->transform.sY = (float)((rand() % (2 * (sMax * sMaxScale))) - (sMax * sMaxScale)) / (float)sMaxScale;
		if (n->transform.sX >= 0) {
			n->transform.sX = 0.2f + (0.8f * n->transform.sX);
		} else {
			n->transform.sX = -0.2f + (0.8f * n->transform.sX);
		}
		if (n->transform.sY >= 0) {
			n->transform.sY = 0.2f + (0.8f * n->transform.sY);
		} else {
			n->transform.sY = -0.2f + (0.8f * n->transform.sY);
		}
	}
	//color
	{
		n->color8.r = (BYTE)(255.0f * ((float)(rand() % cMaxScale) / (float)cMaxScale));
		n->color8.g = (BYTE)(255.0f * ((float)(rand() % cMaxScale) / (float)cMaxScale));
		n->color8.b = (BYTE)(255.0f * ((float)(rand() % cMaxScale) / (float)cMaxScale));
		n->color8.a = (BYTE)(255.0f * ((float)(rand() % cMaxScale) / (float)cMaxScale));
	}
	//vertexs
	{
		const int vSlotsAvail = (dst->verts.v.use - *nxtV0) + (dst->verts.v1.use - *nxtV1) + (dst->verts.v2.use - *nxtV2) + (dst->verts.v3.use - *nxtV3);
		if (vSlotsAvail > 0) {
			int vSlotType = -1;
			char* arrSlotsPtr = 0;
			int arrSlotsItmSz = 0;
			int* arrSlotsNxtIdxPtr = NULL;
			const int vSlotIdx = (rand() % vSlotsAvail);
			int vSlotFirstIdx = 0;
			int vSlotToUse = (rand() % (3 * 10));
			if (vSlotToUse > 0) {
				int vSlotsLmt = (dst->verts.v.use - *nxtV0);
				if (0 && vSlotIdx < vSlotsLmt) { //ToDo: reenable v0
					//use v0
					if (vSlotToUse > (dst->verts.v.use - *nxtV0)) vSlotToUse = (dst->verts.v.use - *nxtV0);
					vSlotType = 0;
					vSlotFirstIdx = *nxtV0;
					arrSlotsPtr = (char*)&NBArray_dataPtr(&dst->verts.v, STNBScnVertex)[vSlotFirstIdx];
					arrSlotsItmSz = sizeof(STNBScnVertex);
					arrSlotsNxtIdxPtr = nxtV0;
				} else {
					vSlotsLmt += (dst->verts.v1.use - *nxtV1);
					if (vSlotIdx < vSlotsLmt) {
						//use v1
						if (vSlotToUse > (dst->verts.v1.use - *nxtV1)) vSlotToUse = (dst->verts.v1.use - *nxtV1);
						vSlotType = 1;
						vSlotFirstIdx = *nxtV1;
						arrSlotsPtr = (char*)&NBArray_dataPtr(&dst->verts.v1, STNBScnVertexTex)[vSlotFirstIdx];
						arrSlotsItmSz = sizeof(STNBScnVertexTex);
						arrSlotsNxtIdxPtr = nxtV1;
					} else {
						vSlotsLmt += (dst->verts.v2.use - *nxtV2);
						if (vSlotIdx < vSlotsLmt) {
							//use v2
							if (vSlotToUse > (dst->verts.v2.use - *nxtV2)) vSlotToUse = (dst->verts.v2.use - *nxtV2);
							vSlotType = 2;
							vSlotFirstIdx = *nxtV2;
							arrSlotsPtr = (char*)&NBArray_dataPtr(&dst->verts.v2, STNBScnVertexTex2)[vSlotFirstIdx];
							arrSlotsItmSz = sizeof(STNBScnVertexTex2);
							arrSlotsNxtIdxPtr = nxtV2;
						} else {
							NBASSERT(vSlotIdx < vSlotsLmt + (dst->verts.v3.use - *nxtV3))
							//use v3
							if (vSlotToUse > (dst->verts.v3.use - *nxtV3)) vSlotToUse = (dst->verts.v3.use - *nxtV3);
							vSlotType = 3;
							vSlotFirstIdx = *nxtV3;
							arrSlotsPtr = (char*)&NBArray_dataPtr(&dst->verts.v3, STNBScnVertexTex3)[vSlotFirstIdx];
							arrSlotsItmSz = sizeof(STNBScnVertexTex3);
							arrSlotsNxtIdxPtr = nxtV3;
						}
					}
				}
			}
			//generate vertexs
			if(arrSlotsPtr != NULL && vSlotToUse > 0){
				NBASSERT(n->verts.iFirst == 0 && n->verts.count == 0)
				n->verts.iFirst = vSlotFirstIdx;
				//printf("Generating vertices type-%d (%d, +%d).\n", vSlotType, vSlotFirstIdx, vSlotToUse);
				int i; for (i = 0; i < vSlotToUse; i++) {
					STNBScnVertex* v = (STNBScnVertex*)arrSlotsPtr;
					//
					assert((vSlotType != 0 || *arrSlotsNxtIdxPtr < dst->verts.v.use) && (vSlotType != 1 || *arrSlotsNxtIdxPtr < dst->verts.v1.use) && (vSlotType != 2 || *arrSlotsNxtIdxPtr < dst->verts.v2.use) && (vSlotType != 3 || *arrSlotsNxtIdxPtr < dst->verts.v3.use));
					//
					v->x = (float)((rand() % (2 * (tMax * tMaxScale))) - (tMax * tMaxScale)) / (float)tMaxScale;
					v->y = (float)((rand() % (2 * (tMax * tMaxScale))) - (tMax * tMaxScale)) / (float)tMaxScale;
					//
					v->color.r = (float)(rand() % cMaxScale) / (float)cMaxScale;
					v->color.g = (float)(rand() % cMaxScale) / (float)cMaxScale;
					v->color.b = (float)(rand() % cMaxScale) / (float)cMaxScale;
					v->color.a = (float)(rand() % cMaxScale) / (float)cMaxScale;
					//
					switch (vSlotType) {
						case 3:
							{
								STNBScnVertexTex3* v = (STNBScnVertexTex3*)arrSlotsPtr;
								v->tex3.x = (float)(rand() % texMaxScale) / (float)texMaxScale;
								v->tex3.y = (float)(rand() % texMaxScale) / (float)texMaxScale;
							}
							//do-not-break;
						case 2:
							{
								STNBScnVertexTex2* v = (STNBScnVertexTex2*)arrSlotsPtr;
								v->tex2.x = (float)(rand() % texMaxScale) / (float)texMaxScale;
								v->tex2.y = (float)(rand() % texMaxScale) / (float)texMaxScale;
							}
							//do-not-break;
						case 1:
							{
								STNBScnVertexTex* v = (STNBScnVertexTex*)arrSlotsPtr;
								v->tex.x = (float)(rand() % texMaxScale) / (float)texMaxScale;
								v->tex.y = (float)(rand() % texMaxScale) / (float)texMaxScale;
							}
							break;
						default:
							break;
					}
					//next
					arrSlotsPtr += arrSlotsItmSz;
					*arrSlotsNxtIdxPtr = *arrSlotsNxtIdxPtr + 1;
					n->verts.count++;
				}
				//
				NBScnTreeNode_setVertsType(n, vSlotType);
			}
		}
	}
}

//run

//compare

static float lclAbs(const float v) {
	return (v < 0 ? -v : v);
}

float research_scn_compute_compare(const STNBScnRenderJobFlat* src, const STNBScnRenderJobFlat* other, const float maxDiffAbs, const STNBScnRenderJobTree* optSrc) {
    const unsigned int ammPrintMax = 0;
    unsigned int ammEval = 0, ammMissmatch = 0, ammPrinted = 0;
	if (src->map.nodes.use != other->map.nodes.use) {
		printf("Nodes-count missmatch (%d vs %d)\n", src->map.nodes.use, other->map.nodes.use);
        return 0.0f;
	} else {
		//compare nodes
		{
			int i; for (i = 0; i < src->map.nodes.use; i++) {
				const STNBScnFlatNode* s0 = &src->map.nodes.arr[i];
				const STNBScnFlatNode* s1 = &other->map.nodes.arr[i];
                //
                ammEval++;
				//
				if (s0->pck != s1->pck) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Header missmatch (#%d/%d):\n", (i + 1), src->map.nodes.use);
                        printf("  src0 iDeepLvl(%d) %s %s.\n", NBScnFlatNode_getDeepLvl(s0), NBScnFlatNode_getIsHidden(s0) ? "hidden" : "visible", NBScnFlatNode_getIsDisabled(s0) ? "disabled" : "enabled");
                        printf("  src1 iDeepLvl(%d) %s %s.\n", NBScnFlatNode_getDeepLvl(s1), NBScnFlatNode_getIsHidden(s1) ? "hidden" : "visible", NBScnFlatNode_getIsDisabled(s1) ? "disabled" : "enabled");
                        if (optSrc != NULL && i < optSrc->nodes.use) {
                            const STNBScnTreeNode* org = NBArray_itmPtrAtIndex(&optSrc->nodes, const STNBScnTreeNode, i);
                            printf("Original:\n");
                            research_scn_compute_print_tree_node(org, "  ");
                        }
                    }
                    ammMissmatch++;
                    continue;
				}
				//
				if (lclAbs(s0->matrix.e[0] - s1->matrix.e[0]) > maxDiffAbs || lclAbs(s0->matrix.e[1] - s1->matrix.e[1]) > maxDiffAbs || lclAbs(s0->matrix.e[2] - s1->matrix.e[2]) > maxDiffAbs
					|| lclAbs(s0->matrix.e[3] - s1->matrix.e[3]) > maxDiffAbs || lclAbs(s0->matrix.e[4] - s1->matrix.e[4]) > maxDiffAbs || lclAbs(s0->matrix.e[5] - s1->matrix.e[5]) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Matrix missmatch (#%d/%d):\n", (i + 1), src->map.nodes.use);
                        printf("  matrix0_row0[%.6f, %.6f, %.6f].\n", s0->matrix.e[0], s0->matrix.e[1], s0->matrix.e[2]);
                        printf("  matrix1_row0[%.6f, %.6f, %.6f].\n", s1->matrix.e[0], s1->matrix.e[1], s1->matrix.e[2]);
                        printf("  matrix0_row1[%.6f, %.6f, %.6f].\n", s0->matrix.e[3], s0->matrix.e[4], s0->matrix.e[5]);
                        printf("  matrix1_row1[%.6f, %.6f, %.6f].\n", s1->matrix.e[3], s1->matrix.e[4], s1->matrix.e[5]);
                        //
                        printf("  diff_row0[%.6f, %.6f, %.6f].\n", s1->matrix.e[0] - s0->matrix.e[0], s1->matrix.e[1] - s0->matrix.e[1], s1->matrix.e[2] - s0->matrix.e[2]);
                        printf("  diff_row1[%.6f, %.6f, %.6f].\n", s1->matrix.e[3] - s0->matrix.e[3], s1->matrix.e[4] - s0->matrix.e[4], s1->matrix.e[5] - s0->matrix.e[5]);
                        //
                        if (optSrc != NULL && i < optSrc->nodes.use) {
                            const STNBScnTreeNode* org = NBArray_itmPtrAtIndex(&optSrc->nodes, const STNBScnTreeNode, i);
                            printf("Original:\n");
                            research_scn_compute_print_tree_node(org, "  ");
                        }
                    }
                    ammMissmatch++;
                    continue;
				}
				//
				if (lclAbs(s0->matrixInv.e[0] - s1->matrixInv.e[0]) > maxDiffAbs || lclAbs(s0->matrixInv.e[1] - s1->matrixInv.e[1]) > maxDiffAbs || lclAbs(s0->matrixInv.e[2] - s1->matrixInv.e[2]) > maxDiffAbs
					|| lclAbs(s0->matrixInv.e[3] - s1->matrixInv.e[3]) > maxDiffAbs || lclAbs(s0->matrixInv.e[4] - s1->matrixInv.e[4]) > maxDiffAbs || lclAbs(s0->matrixInv.e[5] - s1->matrixInv.e[5]) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("MatrixInv missmatch (#%d/%d):\n", (i + 1), src->map.nodes.use);
                        printf("  matrixInv0_row0[%.6f, %.6f, %.6f].\n", s0->matrixInv.e[0], s0->matrixInv.e[1], s0->matrixInv.e[2]);
                        printf("  matrixInv1_row0[%.6f, %.6f, %.6f].\n", s1->matrixInv.e[0], s1->matrixInv.e[1], s1->matrixInv.e[2]);
                        printf("  matrixInv0_row1[%.6f, %.6f, %.6f].\n", s0->matrixInv.e[3], s0->matrixInv.e[4], s0->matrixInv.e[5]);
                        printf("  matrixInv1_row1[%.6f, %.6f, %.6f].\n", s1->matrixInv.e[3], s1->matrixInv.e[4], s1->matrixInv.e[5]);
                        //
                        printf("  diff_row0[%.6f, %.6f, %.6f].\n", s1->matrixInv.e[0] - s0->matrixInv.e[0], s1->matrixInv.e[1] - s0->matrixInv.e[1], s1->matrixInv.e[2] - s0->matrixInv.e[2]);
                        printf("  diff_row1[%.6f, %.6f, %.6f].\n", s1->matrixInv.e[3] - s0->matrixInv.e[3], s1->matrixInv.e[4] - s0->matrixInv.e[4], s1->matrixInv.e[5] - s0->matrixInv.e[5]);
                        if (optSrc != NULL && i < optSrc->nodes.use) {
                            const STNBScnTreeNode* org = NBArray_itmPtrAtIndex(&optSrc->nodes, const STNBScnTreeNode, i);
                            printf("Original:\n");
                            research_scn_compute_print_tree_node(org, "  ");
                        }
                    }
                    ammMissmatch++;
                    continue;
				}
				//
				if (lclAbs(s0->color.r - s0->color.r) > maxDiffAbs || lclAbs(s0->color.g - s0->color.g) > maxDiffAbs || lclAbs(s0->color.b - s0->color.b) > maxDiffAbs || lclAbs(s0->color.a - s0->color.a) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Color missmatch (#%d/%d):\n", (i + 1), src->map.nodes.use);
                        printf("   color0[%.2f, %.2f, %.2f, %.2f].\n", s0->color.r, s0->color.g, s0->color.b, s0->color.a);
                        printf("   color1[%.2f, %.2f, %.2f, %.2f].\n", s1->color.r, s1->color.g, s1->color.b, s1->color.a);
                        if (optSrc != NULL && i < optSrc->nodes.use) {
                            const STNBScnTreeNode* org = NBArray_itmPtrAtIndex(&optSrc->nodes, const STNBScnTreeNode, i);
                            printf("Original:\n");
                            research_scn_compute_print_tree_node(org, "  ");
                        }
                    }
                    ammMissmatch++;
                    continue;
				}
			}
		}
		//compare vertices0
		if (src->map.verts.v.use != other->map.verts.v.use) {
			printf("Vertices0-count missmatch (%d vs %d)\n", src->map.verts.v.use, other->map.verts.v.use);
			return 0.0f;
		} else {
			int i; for (i = 0; i < src->map.verts.v.use; i++) {
				const STNBScnVertexF* s0 = &src->map.verts.v.arr[i];
				const STNBScnVertexF* s1 = &other->map.verts.v.arr[i];
                //
                ammEval++;
                //
				if (lclAbs(s0->x - s1->x) > maxDiffAbs || lclAbs(s0->y - s1->y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices0-coords missmatch (#%d/%d):\n", (i + 1), src->map.verts.v.use);
                        printf("    pos0[%.6f, %.6f]\n", s0->x, s0->y);
                        printf("    pos1[%.6f, %.6f]\n", s1->x, s1->y);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->color.r - s1->color.r) > maxDiffAbs || lclAbs(s0->color.g - s1->color.g) > maxDiffAbs || lclAbs(s0->color.b - s1->color.b) > maxDiffAbs || lclAbs(s0->color.a - s1->color.a) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices0-color missmatch (#%d/%d):\n", (i + 1), src->map.verts.v.use);
                        printf("   color0[%.4f, %.4f, %.4f, %.4f].\n", s0->color.r, s0->color.g, s0->color.b, s0->color.a);
                        printf("   color1[%.4f, %.4f, %.4f, %.4f].\n", s1->color.r, s1->color.g, s1->color.b, s1->color.a);
                    }
                    ammMissmatch++;
                    continue;
				}
			}
		}
		//compare vertices1
		if (src->map.verts.v1.use != other->map.verts.v1.use) {
			printf("Vertices1-count missmatch (%d vs %d)\n", src->map.verts.v1.use, other->map.verts.v1.use);
			return 0.0f;
		} else {
			int i; for (i = 0; i < src->map.verts.v1.use; i++) {
				const STNBScnVertexTexF* s0 = &src->map.verts.v1.arr[i];
				const STNBScnVertexTexF* s1 = &other->map.verts.v1.arr[i];
                //
                ammEval++;
                //
				if (lclAbs(s0->x - s1->x) > maxDiffAbs || lclAbs(s0->y - s1->y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices1-coords missmatch (#%d/%d):\n", (i + 1), src->map.verts.v1.use);
                        printf("    pos0[%.6f, %.6f]\n", s0->x, s0->y);
                        printf("    pos1[%.6f, %.6f]\n", s1->x, s1->y);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->color.r - s1->color.r) > maxDiffAbs || lclAbs(s0->color.g - s1->color.g) > maxDiffAbs || lclAbs(s0->color.b - s1->color.b) > maxDiffAbs || lclAbs(s0->color.a - s1->color.a) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices1-color missmatch (#%d/%d):\n", (i + 1), src->map.verts.v1.use);
                        printf("   color0[%.4f, %.4f, %.4f, %.4f].\n", s0->color.r, s0->color.g, s0->color.b, s0->color.a);
                        printf("   color1[%.4f, %.4f, %.4f, %.4f].\n", s1->color.r, s1->color.g, s1->color.b, s1->color.a);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->tex.x - s1->tex.x) > maxDiffAbs || lclAbs(s0->tex.y - s1->tex.y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices1-tex1 missmatch (#%d/%d):\n", (i + 1), src->map.verts.v1.use);
                        printf("   tex1-0[%.2f, %.2f].\n", s0->tex.x, s0->tex.y);
                        printf("   tex1-1[%.2f, %.2f].\n", s1->tex.x, s1->tex.y);
                    }
                    ammMissmatch++;
                    continue;
				}
			}
		}
		//compare vertices2
		if (src->map.verts.v2.use != other->map.verts.v2.use) {
			printf("Vertices1-count missmatch (%d vs %d)\n", src->map.verts.v2.use, other->map.verts.v2.use);
			return 0.0f;
		} else {
			int i; for (i = 0; i < src->map.verts.v2.use; i++) {
				const STNBScnVertexTex2F* s0 = &src->map.verts.v2.arr[i];
				const STNBScnVertexTex2F* s1 = &other->map.verts.v2.arr[i];
                //
                ammEval++;
                //
				if (lclAbs(s0->x - s1->x) > maxDiffAbs || lclAbs(s0->y - s1->y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices2-coords missmatch (#%d/%d):\n", (i + 1), src->map.verts.v2.use);
                        printf("    pos0[%.6f, %.6f]\n", s0->x, s0->y);
                        printf("    pos1[%.6f, %.6f]\n", s1->x, s1->y);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->color.r - s1->color.r) > maxDiffAbs || lclAbs(s0->color.g - s1->color.g) > maxDiffAbs || lclAbs(s0->color.b - s1->color.b) > maxDiffAbs || lclAbs(s0->color.a - s1->color.a) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices2-color missmatch (#%d/%d):\n", (i + 1), src->map.verts.v2.use);
                        printf("   color0[%.4f, %.4f, %.4f, %.4f].\n", s0->color.r, s0->color.g, s0->color.b, s0->color.a);
                        printf("   color1[%.4f, %.4f, %.4f, %.4f].\n", s1->color.r, s1->color.g, s1->color.b, s1->color.a);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->tex.x - s1->tex.x) > maxDiffAbs || lclAbs(s0->tex.y - s1->tex.y) > maxDiffAbs || lclAbs(s0->tex2.x - s1->tex2.x) > maxDiffAbs || lclAbs(s0->tex2.y - s1->tex2.y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices1-tex1 missmatch (#%d/%d):\n", (i + 1), src->map.verts.v2.use);
                        printf("   tex1-0[%.2f, %.2f] tex2-0[%.2f, %.2f].\n", s0->tex.x, s0->tex2.y, s0->tex2.x, s0->tex2.y);
                        printf("   tex1-1[%.2f, %.2f] tex2-1[%.2f, %.2f].\n", s1->tex.x, s1->tex2.y, s1->tex2.x, s1->tex2.y);
                    }
                    ammMissmatch++;
                    continue;
				}
			}
		}
		//compare vertices3
		if (src->map.verts.v3.use != other->map.verts.v3.use) {
			printf("Vertices1-count missmatch (%d vs %d)\n", src->map.verts.v3.use, other->map.verts.v3.use);
			return 0.0f;
		} else {
			int i; for (i = 0; i < src->map.verts.v3.use; i++) {
				const STNBScnVertexTex3F* s0 = &src->map.verts.v3.arr[i];
				const STNBScnVertexTex3F* s1 = &other->map.verts.v3.arr[i];
                //
                ammEval++;
                //
				if (lclAbs(s0->x - s1->x) > maxDiffAbs || lclAbs(s0->y - s1->y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices3-coords missmatch (#%d/%d):\n", (i + 1), src->map.verts.v3.use);
                        printf("    pos0[%.6f, %.6f]\n", s0->x, s0->y);
                        printf("    pos1[%.6f, %.6f]\n", s1->x, s1->y);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->color.r - s1->color.r) > maxDiffAbs || lclAbs(s0->color.g - s1->color.g) > maxDiffAbs || lclAbs(s0->color.b - s1->color.b) > maxDiffAbs || lclAbs(s0->color.a - s1->color.a) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices3-color missmatch (#%d/%d):\n", (i + 1), src->map.verts.v3.use);
                        printf("   color0[%.4f, %.4f, %.4f, %.4f].\n", s0->color.r, s0->color.g, s0->color.b, s0->color.a);
                        printf("   color1[%.4f, %.4f, %.4f, %.4f].\n", s1->color.r, s1->color.g, s1->color.b, s1->color.a);
                    }
                    ammMissmatch++;
                    continue;
				}
				if (lclAbs(s0->tex.x - s1->tex.x) > maxDiffAbs || lclAbs(s0->tex.y - s1->tex.y) > maxDiffAbs || lclAbs(s0->tex2.x - s1->tex2.x) > maxDiffAbs || lclAbs(s0->tex2.y - s1->tex2.y) > maxDiffAbs || lclAbs(s0->tex3.x - s1->tex3.x) > maxDiffAbs || lclAbs(s0->tex3.y - s1->tex3.y) > maxDiffAbs) {
                    if(ammPrinted < ammPrintMax){
                        ammPrinted++;
                        printf("Vertices1-tex1 missmatch (#%d/%d):\n", (i + 1), src->map.verts.v3.use);
                        printf("   tex1-0[%.2f, %.2f] tex2-0[%.2f, %.2f] tex3-0[%.2f, %.2f].\n", s0->tex.x, s0->tex2.y, s0->tex2.x, s0->tex2.y, s0->tex3.x, s0->tex3.y);
                        printf("   tex1-1[%.2f, %.2f] tex2-1[%.2f, %.2f] tex3-1[%.2f, %.2f].\n", s1->tex.x, s1->tex2.y, s1->tex2.x, s1->tex2.y, s1->tex3.x, s1->tex3.y);
                    }
                    ammMissmatch++;
                    continue;
				}
			}
		}
	}
    NBASSERT(ammMissmatch <= ammEval)
    return (ammEval == 0 ? 1.0f : (float)(ammEval - ammMissmatch) / (float)ammEval);
}


//print

void research_scn_compute_print_tree_node(const STNBScnTreeNode* n, const char* prefix) {
	printf("%s  iParent(%d) childCount(%d) %s %s.\n", prefix, n->iParent, NBScnTreeNode_getChildCount(n), NBScnTreeNode_getIsHidden(n) ? "hidden" : "visible", NBScnTreeNode_getIsDisabled(n) ? "disabled" : "enabled");
	printf("%s  transform t(%.6f, %.6f) deg(%.6f) s(%.6f, %.6f).\n", prefix, n->transform.x, n->transform.y, n->transform.deg, n->transform.sX, n->transform.sY);
	printf("%s  color[%u, %u, %u, %u].\n", prefix, n->color8.r, n->color8.g, n->color8.b, n->color8.a);
	printf("%s  vertices(type-%d, %d, +%d).\n", prefix, NBScnTreeNode_getVertsType(n), n->verts.iFirst, n->verts.count);
}

void research_scn_compute_print_flat_node(const STNBScnFlatNode* n, const char* prefix) {
	printf("%s  iDeepLvl(%d) %s %s.\n", prefix, NBScnFlatNode_getDeepLvl(n), NBScnFlatNode_getIsHidden(n) ? "hidden" : "visible", NBScnFlatNode_getIsDisabled(n) ? "disabled" : "enabled");
	printf("%s  matrix[%.6f, %.6f, %.6f].\n", prefix, n->matrix.e[0], n->matrix.e[1], n->matrix.e[2]);
	printf("%s        [%.6f, %.6f, %.6f].\n", prefix, n->matrix.e[3], n->matrix.e[4], n->matrix.e[5]);
	printf("%s  matInv[%.6f, %.6f, %.6f].\n", prefix, n->matrixInv.e[0], n->matrixInv.e[1], n->matrixInv.e[2]);
	printf("%s        [%.6f, %.6f, %.6f].\n", prefix, n->matrixInv.e[3], n->matrixInv.e[4], n->matrixInv.e[5]);
	printf("%s   color[%.2f, %.2f, %.2f, %.2f].\n", prefix, n->color.r, n->color.g, n->color.b, n->color.a);
}

void research_scn_compute_print_flat_job(const STNBScnRenderJobFlat* src, const unsigned int spacesPerLvl) {
	const STNBScnFlatNode* nodes = src->map.nodes.arr;
	const int nodesCount = src->map.nodes.use;
	char* prefixBuff = NULL; unsigned int prefixBuffUse = 0, prefixBuffSz = 0;
	int i; for (i = 0; i < nodesCount; i++) {
		const STNBScnFlatNode* n = &nodes[i];
		const unsigned int iDeepLvl = NBScnFlatNode_getDeepLvl(n);
		//resize prefix
		if (prefixBuffSz < ((iDeepLvl * spacesPerLvl) + 1)) {
			const unsigned int pnSz = ((iDeepLvl * spacesPerLvl) + 1);
			char* pN = (char*)malloc(sizeof(char) * pnSz);
			if (pN != NULL) {
				if (prefixBuff != NULL) {
					if (prefixBuffSz > 0) {
						memcpy(pN, prefixBuff, prefixBuffSz);
					}
					free(prefixBuff);
				}
				prefixBuff = pN;
				prefixBuffSz = pnSz;
			}
		}
		//contract or expand prefix
		if (prefixBuff != NULL) {
			if (prefixBuffSz > 0) {
				while (prefixBuffUse < prefixBuffSz && prefixBuffUse < (iDeepLvl * spacesPerLvl)) {
					prefixBuff[prefixBuffUse++] = ' ';
				}
				prefixBuffUse = (iDeepLvl * spacesPerLvl);
				prefixBuff[prefixBuffUse] = '\0';
			}
			//print
			if (i != 0) printf("\n");
			printf("%sNode(#%d/%d).\n", prefixBuff, (i + 1), nodesCount);
			research_scn_compute_print_flat_node(n, prefixBuff);
		}
		if (prefixBuff != NULL) {
			free(prefixBuff);
			prefixBuff = NULL;
		}
		prefixBuffUse = prefixBuffSz = 0;
	}
}


void research_scn_compute_convertTreeToPlainGpuAlgorithmToDst(const BYTE* hdr, const BYTE* tree, BYTE* flat, const UI32 ammNodes) {
	//Execute
	UI32 iNodeSrc = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_iNodeOffset];
	//
	const UI32 trNdsOff = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_nodes + NBScnRenderBuffRng_IDX_offset];
	const UI32 trV0Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v + NBScnRenderBuffRng_IDX_offset];
	const UI32 trV1Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v1 + NBScnRenderBuffRng_IDX_offset];
	const UI32 trV2Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v2 + NBScnRenderBuffRng_IDX_offset];
	const UI32 trV3Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v3 + NBScnRenderBuffRng_IDX_offset];
	//
	const UI32 flNdsOff = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_nodes + NBScnRenderBuffRng_IDX_offset];
	const UI32 flV0Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v + NBScnRenderBuffRng_IDX_offset];
	const UI32 flV1Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v1 + NBScnRenderBuffRng_IDX_offset];
	const UI32 flV2Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v2 + NBScnRenderBuffRng_IDX_offset];
	const UI32 flV3Off = *(UI32*)&hdr[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v3 + NBScnRenderBuffRng_IDX_offset];
	//
	const UI32 iNodeNextLast = iNodeSrc + ammNodes;
	while (iNodeSrc < iNodeNextLast) {
		UI32 iNode = iNodeSrc;
		UI32 iDeepLvl = 0u;
		UI32 iParent = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent]);
		UI32 pck = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck]);
		UI32 chldCountSrc = NBScnTreeNode_getChildCountV(pck);
		UI32 isHidden = NBScnTreeNode_getIsHiddenV(pck);
		UI32 isDisabled = NBScnTreeNode_getIsDisabledV(pck);
		float rad = DEG_2_RAD(*(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg]));
		float radSin = sin(rad);
		float radCos = cos(rad);
		float sx = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX]);
		float sy = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY]);
		UI32 c8 = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c]);
		float r = (float)NBColor8_getRV(c8) / 255.f;
		float g = (float)NBColor8_getGV(c8) / 255.f;
		float b = (float)NBColor8_getBV(c8) / 255.f;
		float a = (float)NBColor8_getAV(c8) / 255.f;
		UI32 vsType = NBScnTreeNode_getVertsTypeV(pck);
		UI32 vsFirstIdx = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_iFirst]);
		UI32 vsCount = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_count]);
		float mDet;
		STNBMatrix mP, mC, m;
		m._m00 = radCos * sx;
		m._m01 = -radSin * sy;
		m._m02 = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x]);
		m._m10 = radSin * sx;
		m._m11 = radCos * sy;
		m._m12 = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y]);
		while (iNode != iParent) {
			iNode = iParent;
			iParent = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent]);
			iDeepLvl++;
			pck = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck]);
			if (NBScnTreeNode_getIsHiddenV(pck) != 0u) {
				isHidden = 1u;
			}
			if (NBScnTreeNode_getIsDisabledV(pck) != 0u) {
				isDisabled = 1u;
			}
			rad = DEG_2_RAD(*(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg]));
			radSin = sin(rad);
			radCos = cos(rad);
			sx = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX]);
			sy = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY]);
			mC = m;
			mP._m00 = radCos * sx;
			mP._m01 = -radSin * sy;
			mP._m02 = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x]);
			mP._m10 = radSin * sx;
			mP._m11 = radCos * sy;
			mP._m12 = *(float*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y]);
			m._m00 = (mP._m00 * mC._m00) + (mP._m01 * mC._m10) /*always-zero: + ( mP._m02 * mC._m20)*/;
			m._m01 = (mP._m00 * mC._m01) + (mP._m01 * mC._m11) /*always-zero: + ( mP._m02 * mC._m21)*/;
			m._m02 = (mP._m00 * mC._m02) + (mP._m01 * mC._m12) + (mP._m02 /*always-one: * mC._m22*/);
			m._m10 = (mP._m10 * mC._m00) + (mP._m11 * mC._m10) /*always-zero: + ( mP._m12 * mC._m20)*/;
			m._m11 = (mP._m10 * mC._m01) + (mP._m11 * mC._m11) /*always-zero: + ( mP._m12 * mC._m21)*/;
			m._m12 = (mP._m10 * mC._m02) + (mP._m11 * mC._m12) + (mP._m12 /*always-one: * mC._m22*/);
			c8 = *(UI32*)(&tree[trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c]);
			r *= (float)NBColor8_getRV(c8) / 255.f;
			g *= (float)NBColor8_getGV(c8) / 255.f;
			b *= (float)NBColor8_getBV(c8) / 255.f;
			a *= (float)NBColor8_getAV(c8) / 255.f;
		}
		//transform vertices
		if (vsCount > 0) {
			UI32 vIdx, vIdxAfterLast;
			float vX, vY, vXP, vYP, vCR, vCG, vCB, vCA;
			float t1x, t1y, t2x, t2y, t3x, t3y;
			vIdxAfterLast = vsFirstIdx + vsCount;
			switch (vsType) {
			case 0:
				for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
					vX = *(float*)(&tree[trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_x]);
					vY = *(float*)(&tree[trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_y]);
					//
					vXP = (m._m00 * vX) + (m._m01 * vY) + m._m02;
					vYP = (m._m10 * vX) + (m._m11 * vY) + m._m12;
					//
					c8 = *(UI32*)(&tree[trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_color]);
					vCR = r * ((float)NBColor8_getRV(c8) / 255.f);
					vCG = g * ((float)NBColor8_getGV(c8) / 255.f);
					vCB = b * ((float)NBColor8_getBV(c8) / 255.f);
					vCA = a * ((float)NBColor8_getAV(c8) / 255.f);
					//
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_x] = *(UI32*)(&vXP);
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_y] = *(UI32*)(&vYP);
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_r] = *(UI32*)(&vCR);
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_g] = *(UI32*)(&vCG);
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_b] = *(UI32*)(&vCB);
					*(UI32*)&flat[flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_a] = *(UI32*)(&vCA);
				}
				break;
			case 1:
				for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
					vX = *(float*)(&tree[trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_x]);
					vY = *(float*)(&tree[trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_y]);
					//
					vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
					vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
					//
					c8 = *(UI32*)(&tree[trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_color]);
					vCR = r * ((float)NBColor8_getRV(c8) / 255.f);
					vCG = g * ((float)NBColor8_getGV(c8) / 255.f);
					vCB = b * ((float)NBColor8_getBV(c8) / 255.f);
					vCA = a * ((float)NBColor8_getAV(c8) / 255.f);
					//
					t1x = *(float*)(&tree[trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_x]);
					t1y = *(float*)(&tree[trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_y]);
					//
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_x] = *(UI32*)(&vXP);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_y] = *(UI32*)(&vYP);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_r] = *(UI32*)(&vCR);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_g] = *(UI32*)(&vCG);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_b] = *(UI32*)(&vCB);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_a] = *(UI32*)(&vCA);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_x] = *(UI32*)(&t1x);
					*(UI32*)&flat[flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_y] = *(UI32*)(&t1y);
				}
				break;
			case 2:
				for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
					vX = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_x]);
					vY = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_y]);
					//
					vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
					vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
					//
					c8 = *(UI32*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_color]);
					vCR = r * ((float)NBColor8_getRV(c8) / 255.f);
					vCG = g * ((float)NBColor8_getGV(c8) / 255.f);
					vCB = b * ((float)NBColor8_getBV(c8) / 255.f);
					vCA = a * ((float)NBColor8_getAV(c8) / 255.f);
					//
					t1x = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_x]);
					t1y = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_y]);
					t2x = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_x]);
					t2y = *(float*)(&tree[trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_y]);
					//
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_x] = *(UI32*)(&vXP);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_y] = *(UI32*)(&vYP);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_r] = *(UI32*)(&vCR);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_g] = *(UI32*)(&vCG);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_b] = *(UI32*)(&vCB);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_a] = *(UI32*)(&vCA);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_x] = *(UI32*)(&t1x);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_y] = *(UI32*)(&t1y);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_x] = *(UI32*)(&t2x);
					*(UI32*)&flat[flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_y] = *(UI32*)(&t2y);
				}
				break;
			case 3:
				for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
					vX = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_x]);
					vY = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_y]);
					//
					vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
					vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
					//
					c8 = *(UI32*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_color]);
					vCR = r * ((float)NBColor8_getRV(c8) / 255.f);
					vCG = g * ((float)NBColor8_getGV(c8) / 255.f);
					vCB = b * ((float)NBColor8_getBV(c8) / 255.f);
					vCA = a * ((float)NBColor8_getAV(c8) / 255.f);
					//
					t1x = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_x]);
					t1y = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_y]);
					t2x = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_x]);
					t2y = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_y]);
					t3x = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_x]);
					t3y = *(float*)(&tree[trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_y]);
					//
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_x] = *(UI32*)(&vXP);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_y] = *(UI32*)(&vYP);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_r] = *(UI32*)(&vCR);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_g] = *(UI32*)(&vCG);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_b] = *(UI32*)(&vCB);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_a] = *(UI32*)(&vCA);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_x] = *(UI32*)(&t1x);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_y] = *(UI32*)(&t1y);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_x] = *(UI32*)(&t2x);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_y] = *(UI32*)(&t2y);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_x] = *(UI32*)(&t3x);
					*(UI32*)&flat[flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_y] = *(UI32*)(&t3y);
				}
				break;
			default:
				break;
			}
		}
		//save
		pck = NBScnFlatNode_withDeepLvl(0u, iDeepLvl) + NBScnFlatNode_withIsHidden(0u, isHidden) + NBScnFlatNode_withIsDisabled(0u, isDisabled);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_pck] = *(UI32*)(&pck);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_0] = *(UI32*)(&m._m00);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_1] = *(UI32*)(&m._m01);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_2] = *(UI32*)(&m._m02);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_0] = *(UI32*)(&m._m10);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_1] = *(UI32*)(&m._m11);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_2] = *(UI32*)(&m._m12);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_r] = *(UI32*)(&r);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_g] = *(UI32*)(&g);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_b] = *(UI32*)(&b);
		*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_a] = *(UI32*)(&a);
		//calculate inverse matrix (last action as div-by-zero-err is posible)
		mDet = (m._m00 * m._m11) - (m._m01 * m._m10);
		if (mDet != 0) {
			mC._m00 = ((m._m11 /** mInv._m22*/) /*- (mInv._m21 * m._m12) allways-zero*/) / mDet;
			mC._m01 = (/*(m._m02 * mInv._m21) allways-zero*/0.0f - (/*mInv._m22 **/ m._m01)) / mDet;
			mC._m02 = ((m._m01 * m._m12) - (m._m11 * m._m02)) / mDet;
			mC._m10 = (/*(m._m12 * mInv._m20) allways-zero*/0.0f - (/*mInv._m22 **/ m._m10)) / mDet;
			mC._m11 = ((m._m00 /** mInv._m22*/) /*- (mInv._m20 * m._m02) allways-zero*/) / mDet;
			mC._m12 = ((m._m02 * m._m10) - (m._m12 * m._m00)) / mDet;
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_0] = *(UI32*)(&mC._m00);
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_1] = *(UI32*)(&mC._m01);
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_2] = *(UI32*)(&mC._m02);
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_0] = *(UI32*)(&mC._m10);
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_1] = *(UI32*)(&mC._m11);
			*(UI32*)&flat[flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_2] = *(UI32*)(&mC._m12);
		}
		//next node
		iNodeSrc++;
	}
}
