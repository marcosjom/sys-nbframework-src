
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/scene/NBScnRenderJob.h"
#include "nb/2d/NBMatrix.h"

//-----------------------
//-- STNBScnRenderJobTree
//-----------------------

void NBScnRenderJobTree_init(STNBScnRenderJobTree* obj) {
	NBMemory_setZeroSt(*obj, STNBScnRenderJobTree);
	//nodes
	{
		NBArray_initWithSz(&obj->nodes, sizeof(STNBScnTreeNode), NULL, 0, 64, 0.1f);
	}
	//verts (vertices)
	{
		NBArray_initWithSz(&obj->verts.v, sizeof(STNBScnVertex), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v1, sizeof(STNBScnVertexTex), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v2, sizeof(STNBScnVertexTex2), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v3, sizeof(STNBScnVertexTex3), NULL, 0, 64, 0.1f);
	}
}

void NBScnRenderJobTree_release(STNBScnRenderJobTree* obj) {
	//nodes
	{
		NBArray_empty(&obj->nodes);
		NBArray_release(&obj->nodes);
	}
	//verts (vertices)
	{
		NBArray_empty(&obj->verts.v);
		NBArray_empty(&obj->verts.v1);
		NBArray_empty(&obj->verts.v2);
		NBArray_empty(&obj->verts.v3);
		//
		NBArray_release(&obj->verts.v);
		NBArray_release(&obj->verts.v1);
		NBArray_release(&obj->verts.v2);
		NBArray_release(&obj->verts.v3);
	}
}

void NBScnRenderJobTree_empty(STNBScnRenderJobTree* obj) {
	//nodes
	{
		NBArray_empty(&obj->nodes);
	}
	//verts (vertices)
	{
		NBArray_empty(&obj->verts.v);
		NBArray_empty(&obj->verts.v1);
		NBArray_empty(&obj->verts.v2);
		NBArray_empty(&obj->verts.v3);
	}
}

//-------------------------
//-- STNBScnRenderJobPlain
//-------------------------

void NBScnRenderJobPlain_init(STNBScnRenderJobPlain* obj) {
	NBMemory_setZeroSt(*obj, STNBScnRenderJobPlain);
	NBASSERT(sizeof(STNBScnVertex) == NBScnVertex_SZ)
	NBASSERT(sizeof(STNBScnVertexF) == NBScnVertexF_SZ)
	NBASSERT(sizeof(STNBScnVertexTex) == NBScnVertexTex_SZ)
	NBASSERT(sizeof(STNBScnVertexTexF) == NBScnVertexTexF_SZ)
	NBASSERT(sizeof(STNBScnVertexTex2) == NBScnVertexTex2_SZ)
	NBASSERT(sizeof(STNBScnVertexTex2F) == NBScnVertexTex2F_SZ)
	NBASSERT(sizeof(STNBScnVertexTex3) == NBScnVertexTex3_SZ)
	NBASSERT(sizeof(STNBScnVertexTex3F) == NBScnVertexTex3F_SZ)
	NBASSERT(sizeof(STNBScnTreeNode) == NBScnTreeNode_SZ)
	NBASSERT(sizeof(STNBScnFlatNode) == NBScnFlatNode_SZ)
	//nodes
	{
		NBArray_initWithSz(&obj->nodes, sizeof(STNBScnFlatNode), NULL, 0, 64, 0.1f);
	}
	//verts (vertices)
	{
		NBArray_initWithSz(&obj->verts.v, sizeof(STNBScnVertexF), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v1, sizeof(STNBScnVertexTexF), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v2, sizeof(STNBScnVertexTex2F), NULL, 0, 64, 0.1f);
		NBArray_initWithSz(&obj->verts.v3, sizeof(STNBScnVertexTex3F), NULL, 0, 64, 0.1f);
	}
}

void NBScnRenderJobPlain_release(STNBScnRenderJobPlain* obj) {
	NBScnRenderJobPlain_empty(obj);
	//nodes
	{
		NBArray_release(&obj->nodes);
	}
	//verts (vertices)
	{
		NBArray_release(&obj->verts.v);
		NBArray_release(&obj->verts.v1);
		NBArray_release(&obj->verts.v2);
		NBArray_release(&obj->verts.v3);
	}
}

void NBScnRenderJobPlain_empty(STNBScnRenderJobPlain* obj) {
	//nodes
	{
		NBArray_empty(&obj->nodes);
	}
	//verts (vertices)
	{
		NBArray_empty(&obj->verts.v);
		NBArray_empty(&obj->verts.v1);
		NBArray_empty(&obj->verts.v2);
		NBArray_empty(&obj->verts.v3);
	}
}

void NBScnRenderJobPlain_prepare(STNBScnRenderJobPlain* obj, const STNBScnRenderJobTree* src) {
	//empty
	NBScnRenderJobPlain_empty(obj);
	//allocate
	if (src != NULL) {
		//nodes
		{
			NBArray_addItems(&obj->nodes, NULL, sizeof(STNBScnFlatNode), src->nodes.use);
		}
		//verts (vertices)
		{
			NBArray_addItems(&obj->verts.v, NULL, sizeof(STNBScnVertexF), src->verts.v.use);
			NBArray_addItems(&obj->verts.v1, NULL, sizeof(STNBScnVertexTexF), src->verts.v1.use);
			NBArray_addItems(&obj->verts.v2, NULL, sizeof(STNBScnVertexTex2F), src->verts.v2.use);
			NBArray_addItems(&obj->verts.v3, NULL, sizeof(STNBScnVertexTex3F), src->verts.v3.use);
		}
	}
}


//-------------------
//-- STNBScnRenderJob
//-------------------

void NBScnRenderJob_init(STNBScnRenderJob* obj) {
	NBMemory_setZeroSt(*obj, STNBScnRenderJob);
	NBScnRenderJobTree_init(&obj->tree);
	NBScnRenderJobPlain_init(&obj->plain);
}

void NBScnRenderJob_release(STNBScnRenderJob* obj) {
	NBScnRenderJobTree_release(&obj->tree);
	NBScnRenderJobPlain_release(&obj->plain);
}

void research_scn_compute_run_cpu_forward_node_and_children_(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst, const unsigned int iDeepLvl, const STNBScnTreeNode* n, const int nIdx, const STNBScnFlatNode* nPrnt);

void NBScnRenderJob_convertTreeToPlainForwardToDst(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst) {
	//prepare destination (empty-it and allocate the buffers if necesary)
	NBScnRenderJobPlain_prepare(dst, src);
	//process
	{
		const int nodesCount = src->nodes.use;
		int i; for (i = 0; i < nodesCount; i++) {
			const STNBScnTreeNode* n = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, i);
			//root nodes point to themself
			if (n->iParent == i) {
				STNBScnFlatNode rr;
				NBMemory_setZeroSt(rr, STNBScnFlatNode);
				NBMatrix_setIdentity(&rr.matrix);
				rr.color.r = rr.color.g = rr.color.b = rr.color.a = 1.0f;
				//printf("Root Matrix:\n"); NBMATRIZ_IMPRIMIR_PRINTF(rr.matrix);
				research_scn_compute_run_cpu_forward_node_and_children_(src, dst, 0, n, i, &rr);
			}
		}
	}
}

void research_scn_compute_run_cpu_transform_vertices_(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst, const STNBScnTreeNode* n, const STNBScnFlatNode* nDst);

void research_scn_compute_run_cpu_forward_node_and_children_(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst, const unsigned int iDeepLvl, const STNBScnTreeNode* n, const int nIdx, const STNBScnFlatNode* nPrnt) {
	STNBScnFlatNode rr;
	NBMemory_setZeroSt(rr, STNBScnFlatNode);
	NBScnFlatNode_setDeepLvl(&rr, iDeepLvl);
	//
	if (NBScnFlatNode_getIsDisabled(nPrnt) || NBScnTreeNode_getIsDisabled(n)) {
		NBScnFlatNode_setIsDisabled(&rr, 1);
	}
	if (NBScnFlatNode_getIsHidden(nPrnt) || NBScnTreeNode_getIsHidden(n)) {
		NBScnFlatNode_setIsHidden(&rr, 1);
	}
	{
		STNBMatrix2D myMatrix = NBMatrix_fromTransforms((const STNBPoint) { n->transform.x, n->transform.y }, DEG_2_RAD(n->transform.deg), (const STNBSize) { n->transform.sX, n->transform.sY });
		rr.matrix = NBMatrix_multiply(&nPrnt->matrix, &myMatrix);
	}
	//
	rr.color.r = ((float)n->color8.r / 255.f) * nPrnt->color.r;
	rr.color.g = ((float)n->color8.g / 255.f)* nPrnt->color.g;
	rr.color.b = ((float)n->color8.b / 255.f)* nPrnt->color.b;
	rr.color.a = ((float)n->color8.a / 255.f)* nPrnt->color.a;
	//process children
	{
		int i = nIdx + 1;
		unsigned int chlPend = NBScnTreeNode_getChildCount(n);
		const int nodesCount = src->nodes.use;
		while (i < nodesCount && chlPend > 0) {
			const STNBScnTreeNode* n2 = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, i);
			if (n2->iParent == nIdx) {
				research_scn_compute_run_cpu_forward_node_and_children_(src, dst, (iDeepLvl + 1), n2, i, &rr);
				chlPend--;
			}
			//next
			i++;
		}
	}
	//transform vertices
	if (n->verts.count > 0) {
		research_scn_compute_run_cpu_transform_vertices_(src, dst, n, &rr);
	}
	//
	rr.matrixInv = NBMatrix_inverse(&rr.matrix);
	//set value
	NBArray_setItemAt(&dst->nodes, nIdx, &rr, sizeof(rr));
}

void NBScnRenderJob_convertTreeToPlainBackwardToDst(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst) {
	//prepare destination (empty-it and allocate the buffers if necesary)
	NBScnRenderJobPlain_prepare(dst, src);
	//process
	{
		const int nodesCount = src->nodes.use;
		int i; for (i = 0; i < nodesCount; i++) {
			const STNBScnTreeNode* n = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, i);
			unsigned int iDeepLvl = 0;
			unsigned int isDisabled = NBScnTreeNode_getIsDisabled(n);
			unsigned int isHidden = NBScnTreeNode_getIsHidden(n);
			STNBScnFlatNode rr;
			NBMemory_setZeroSt(rr, STNBScnFlatNode);
			rr.matrix = NBMatrix_fromTransforms((const STNBPoint) { n->transform.x, n->transform.y }, DEG_2_RAD(n->transform.deg), (const STNBSize) { n->transform.sX, n->transform.sY });
			//
			rr.color.r = (float)n->color8.r / 255.0f;
			rr.color.g = (float)n->color8.g / 255.0f;
			rr.color.b = (float)n->color8.b / 255.0f;
			rr.color.a = (float)n->color8.a / 255.0f;
			//move backwards
			{
				const STNBScnTreeNode* n2 = n;
				const STNBScnTreeNode* nPrnt = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, n2->iParent);
				while (n2 != nPrnt) { //root nodes point to themself
					n2 = nPrnt;
					nPrnt = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, n2->iParent);
					//
					iDeepLvl++;
					isDisabled = (isDisabled || NBScnTreeNode_getIsDisabled(n2)) ? 1 : 0;
					isHidden = (isHidden || NBScnTreeNode_getIsHidden(n2)) ? 1 : 0;
					//
					STNBMatrix2D cpy = rr.matrix, prntMatrix = NBMatrix_fromTransforms((const STNBPoint) { n2->transform.x, n2->transform.y }, DEG_2_RAD(n2->transform.deg), (const STNBSize) { n2->transform.sX, n2->transform.sY });
					rr.matrix = NBMatrix_multiply(&prntMatrix, &rr.matrix);
					//
					rr.color.r *= (float)n2->color8.r / 255.0f;
					rr.color.g *= (float)n2->color8.g / 255.0f;
					rr.color.b *= (float)n2->color8.b / 255.0f;
					rr.color.a *= (float)n2->color8.a / 255.0f;
				}
			}
			//
			NBScnFlatNode_setDeepLvl(&rr, iDeepLvl);
			NBScnFlatNode_setIsDisabled(&rr, isDisabled);
			NBScnFlatNode_setIsHidden(&rr, isHidden);
			//transform vertices
			if (n->verts.count > 0) {
				research_scn_compute_run_cpu_transform_vertices_(src, dst, n, &rr);
			}
			//
			rr.matrixInv = NBMatrix_inverse(&rr.matrix);
			//set value
			NBArray_setItemAt(&dst->nodes, i, &rr, sizeof(rr));
		}
	}
}

void research_scn_compute_run_cpu_transform_vertices_(const STNBScnRenderJobTree* src, STNBScnRenderJobPlain* dst, const STNBScnTreeNode* n, const STNBScnFlatNode* nDst) {
	const unsigned int vFirstIdx = n->verts.iFirst;
	const unsigned int vCount = n->verts.count;
	//printf("Transforming vertices-%d (%d, +%d).\n", n->verts.type, n->verts.iFirst, n->verts.count);
	switch (NBScnTreeNode_getVertsType(n)) {
		case 0:
			{
				const unsigned int vAfterLast = vFirstIdx + vCount;
				int i; for (i = vFirstIdx; i < vAfterLast; i++) {
					const STNBScnVertex* vSrc = NBArray_itmPtrAtIndex(&src->verts.v, const STNBScnVertex, i);
					STNBScnVertexF* vDst = NBArray_itmPtrAtIndex(&dst->verts.v, STNBScnVertexF, i);
					const STNBPoint p = NBMatrix_applyToPoint(&nDst->matrix, (const STNBPoint) { vSrc->x, vSrc->y });
					vDst->x = p.x;
					vDst->y = p.y;
					//printf("Vertex-%d  (%.4f, %.4f) -> (%.4f, %.4f).\n", n->verts.type, vSrc->x, vSrc->y, p.x, p.y);
					vDst->color.r = vSrc->color.r * nDst->color.r;
					vDst->color.g = vSrc->color.g * nDst->color.g;
					vDst->color.b = vSrc->color.b * nDst->color.b;
					vDst->color.a = vSrc->color.a * nDst->color.a;
				}
			}
			break;
		case 1:
			{
				const unsigned int vAfterLast = vFirstIdx + vCount;
				int i; for (i = vFirstIdx; i < vAfterLast; i++) {
					const STNBScnVertexTex* vSrc = NBArray_itmPtrAtIndex(&src->verts.v1, const STNBScnVertexTex, i);
					STNBScnVertexTexF* vDst = NBArray_itmPtrAtIndex(&dst->verts.v1, STNBScnVertexTexF, i);
					const STNBPoint p = NBMatrix_applyToPoint(&nDst->matrix, (const STNBPoint) { vSrc->x, vSrc->y });
					vDst->x = p.x;
					vDst->y = p.y;
					//printf("Vertex-%d  (%.4f, %.4f) -> (%.4f, %.4f).\n", n->verts.type, vSrc->x, vSrc->y, p.x, p.y);
					vDst->color.r = vSrc->color.r * nDst->color.r;
					vDst->color.g = vSrc->color.g * nDst->color.g;
					vDst->color.b = vSrc->color.b * nDst->color.b;
					vDst->color.a = vSrc->color.a * nDst->color.a;
					vDst->tex.x = vSrc->tex.x;
					vDst->tex.y = vSrc->tex.y;
				}
			}
			break;
		case 2:
			{
				const unsigned int vAfterLast = vFirstIdx + vCount;
				int i; for (i = vFirstIdx; i < vAfterLast; i++) {
					const STNBScnVertexTex2* vSrc = NBArray_itmPtrAtIndex(&src->verts.v2, const STNBScnVertexTex2, i);
					STNBScnVertexTex2F* vDst = NBArray_itmPtrAtIndex(&dst->verts.v2, STNBScnVertexTex2F, i);
					const STNBPoint p = NBMatrix_applyToPoint(&nDst->matrix, (const STNBPoint) { vSrc->x, vSrc->y });
					vDst->x = p.x;
					vDst->y = p.y;
					//printf("Vertex-%d  (%.4f, %.4f) -> (%.4f, %.4f).\n", n->verts.type, vSrc->x, vSrc->y, p.x, p.y);
					vDst->color.r = vSrc->color.r * nDst->color.r;
					vDst->color.g = vSrc->color.g * nDst->color.g;
					vDst->color.b = vSrc->color.b * nDst->color.b;
					vDst->color.a = vSrc->color.a * nDst->color.a;
					vDst->tex.x = vSrc->tex.x;
					vDst->tex.y = vSrc->tex.y;
					vDst->tex2.x = vSrc->tex2.x;
					vDst->tex2.y = vSrc->tex2.y;
				}
			}
			break;
		case 3:
			{
				const unsigned int vAfterLast = vFirstIdx + vCount;
				int i; for (i = vFirstIdx; i < vAfterLast; i++) {
					const STNBScnVertexTex3* vSrc = NBArray_itmPtrAtIndex(&src->verts.v3, const STNBScnVertexTex3, i);
					STNBScnVertexTex3F* vDst = NBArray_itmPtrAtIndex(&dst->verts.v3, STNBScnVertexTex3F, i);
					const STNBPoint p = NBMatrix_applyToPoint(&nDst->matrix, (const STNBPoint) { vSrc->x, vSrc->y });
					vDst->x = p.x;
					vDst->y = p.y;
					//printf("Vertex-%d  (%.4f, %.4f) -> (%.4f, %.4f).\n", n->verts.type, vSrc->x, vSrc->y, p.x, p.y);
					vDst->color.r = vSrc->color.r * nDst->color.r;
					vDst->color.g = vSrc->color.g * nDst->color.g;
					vDst->color.b = vSrc->color.b * nDst->color.b;
					vDst->color.a = vSrc->color.a * nDst->color.a;
					vDst->tex.x = vSrc->tex.x;
					vDst->tex.y = vSrc->tex.y;
					vDst->tex2.x = vSrc->tex2.x;
					vDst->tex2.y = vSrc->tex2.y;
					vDst->tex3.x = vSrc->tex3.x;
					vDst->tex3.y = vSrc->tex3.y;
				}
			}
			break;
		default:
			break;
	}
}
