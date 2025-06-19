
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnRenderJobFlat.h"
//
#include "nb/core/NBMemory.h"
#include "nb/2d/NBMatrix.h"

//-------------------------
//-- STNBScnRenderJobFlatMap
//-------------------------

void NBScnRenderJobFlatMap_init(STNBScnRenderJobFlatMap* obj) {
	//nothing
}

void NBScnRenderJobFlatMap_release(STNBScnRenderJobFlatMap* obj) {
	//nothing
}

//

void NBScnRenderJobFlatMap_reset(STNBScnRenderJobFlatMap* obj) {
	NBMemory_setZeroSt(*obj, STNBScnRenderJobFlatMap);
}

void NBScnRenderJobFlatMap_build(STNBScnRenderJobFlatMap* obj, void* ptr, const STNBScnRenderBuffRngs* rngs) {
	//map dst locations
	obj->nodes.arr = (STNBScnFlatNode*)&((BYTE*)ptr)[rngs->nodes.offset];
	obj->nodes.use = rngs->nodes.use / sizeof(STNBScnFlatNode); NBASSERT((rngs->nodes.use % sizeof(STNBScnFlatNode)) == 0)
	//
	obj->verts.v.arr = (STNBScnVertexF*)&((BYTE*)ptr)[rngs->verts.v.offset];
	obj->verts.v.use = rngs->verts.v.use / sizeof(STNBScnVertexF); NBASSERT((rngs->verts.v.use % sizeof(STNBScnVertexF)) == 0)
	//
	obj->verts.v1.arr = (STNBScnVertexTexF*)&((BYTE*)ptr)[rngs->verts.v1.offset];
	obj->verts.v1.use = rngs->verts.v1.use / sizeof(STNBScnVertexTexF); NBASSERT((rngs->verts.v1.use % sizeof(STNBScnVertexTexF)) == 0)
	//
	obj->verts.v2.arr = (STNBScnVertexTex2F*)&((BYTE*)ptr)[rngs->verts.v2.offset];
	obj->verts.v2.use = rngs->verts.v2.use / sizeof(STNBScnVertexTex2F); NBASSERT((rngs->verts.v2.use % sizeof(STNBScnVertexTex2F)) == 0)
	//
	obj->verts.v3.arr = (STNBScnVertexTex3F*)&((BYTE*)ptr)[rngs->verts.v3.offset];
	obj->verts.v3.use = rngs->verts.v3.use / sizeof(STNBScnVertexTex3F); NBASSERT((rngs->verts.v3.use % sizeof(STNBScnVertexTex3F)) == 0)
}

//-------------------------
//-- STNBScnRenderJobFlat
//-------------------------

void NBScnRenderJobFlat_init(STNBScnRenderJobFlat* obj) {
	NBMemory_setZeroSt(*obj, STNBScnRenderJobFlat);
	//
	NBASSERT(sizeof(STNBScnRenderBuffRng) == NBScnRenderBuffRng_SZ)
	NBASSERT(sizeof(STNBScnRenderBuffRngs) == NBScnRenderBuffRngs_SZ)
	NBASSERT(sizeof(STNBScnRenderDispatchHeader) == NBScnRenderDispatchHeader_SZ)
	//
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
	NBScnRenderJobFlatMap_init(&obj->map);
}

void NBScnRenderJobFlat_release(STNBScnRenderJobFlat* obj) {
	if (obj->buff.data != NULL) {
        NBMemory_free(obj->buff.data);
		obj->buff.data = NULL;
	}
	obj->buff.use = obj->buff.sz = 0;
	//
	{
		NBMemory_setZeroSt(obj->map, STNBScnRenderJobFlatMap);
	}
	NBScnRenderJobFlatMap_release(&obj->map);
}

UI32 NBScnRenderJobFlat_prepare(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs) {
	UI32 r = 0;
	STNBScnRenderBuffRngs rDst;
	const UI32 buffSz = NBScnRenderJobFlat_getDispatchBufferRngs(src, limits, offset, &rDst);
	NBMemory_setZeroSt(obj->map, STNBScnRenderJobFlatMap);
	if (buffSz <= 0) {
		obj->buff.use = 0;
	} else {
		//resize buffer
		if (obj->buff.sz < buffSz) {
			if (obj->buff.data != NULL) {
				NBMemory_free(obj->buff.data);
				obj->buff.data = NULL;
			}
			obj->buff.use = obj->buff.sz = 0;
			obj->buff.data = NBMemory_alloc(buffSz);
			if (obj->buff.data != NULL) {
				obj->buff.sz = buffSz;
			}
		}
		if (obj->buff.data != NULL) {
			//map dst locations
			NBScnRenderJobFlatMap_build(&obj->map, obj->buff.data, &rDst);
			r = obj->buff.use = buffSz;
		}
	}
	return r;
}

void NBScnRenderJobFlat_transformVertices_(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnTreeNode* n, const STNBScnFlatNode* nDst);

void NBScnRenderJobFlat_dispatchForwardsNodeAndChildren_(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const unsigned int iDeepLvl, const STNBScnTreeNode* n, const int nIdx, const STNBScnFlatNode* nPrnt);

//this algorithm is runs over the tree, and cannot be called in parallel; each node is passed once. 
UI32 NBScnRenderJobFlat_dispatchForwards(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits) {
	UI32 r = NBScnRenderJobFlat_prepare(obj, src, limits, 0, NULL);
	if (r > 0) {
		//process
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
				NBScnRenderJobFlat_dispatchForwardsNodeAndChildren_(obj, src, 0, n, i, &rr);
			}
		}
	}
	return r;
}

void NBScnRenderJobFlat_dispatchForwardsNodeAndChildren_(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const unsigned int iDeepLvl, const STNBScnTreeNode* n, const int nIdx, const STNBScnFlatNode* nPrnt) {
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
	rr.color.g = ((float)n->color8.g / 255.f) * nPrnt->color.g;
	rr.color.b = ((float)n->color8.b / 255.f) * nPrnt->color.b;
	rr.color.a = ((float)n->color8.a / 255.f) * nPrnt->color.a;
	//process children
	{
		int i = nIdx + 1;
		unsigned int chlPend = NBScnTreeNode_getChildCount(n);
		const int nodesCount = src->nodes.use;
		while (i < nodesCount && chlPend > 0) {
			const STNBScnTreeNode* n2 = NBArray_itmPtrAtIndex(&src->nodes, const STNBScnTreeNode, i);
			if (n2->iParent == nIdx) {
				NBScnRenderJobFlat_dispatchForwardsNodeAndChildren_(obj, src, (iDeepLvl + 1), n2, i, &rr);
				chlPend--;
			}
			//next
			i++;
		}
	}
	//transform vertices
	if (n->verts.count > 0) {
		NBScnRenderJobFlat_transformVertices_(obj, src, n, &rr);
	}
	//
	rr.matrixInv = NBMatrix_inverse(&rr.matrix);
	//set value
	obj->map.nodes.arr[nIdx] = rr;
}

//this algorithm can be called in parallel in exchange of passing through the same node multiple times.
UI32 NBScnRenderJobFlat_dispatchBackwards(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 iNode, const UI32 ammNodes) {
	UI32 r = NBScnRenderJobFlat_prepare(obj, src, limits, 0, NULL);
	if (r > 0) {
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
					STNBMatrix2D prntMatrix = NBMatrix_fromTransforms((const STNBPoint) { n2->transform.x, n2->transform.y }, DEG_2_RAD(n2->transform.deg), (const STNBSize) { n2->transform.sX, n2->transform.sY });
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
				NBScnRenderJobFlat_transformVertices_(obj, src, n, &rr);
			}
			//
			rr.matrixInv = NBMatrix_inverse(&rr.matrix);
			//set value
			obj->map.nodes.arr[i] = rr;
		}
	}
	return r;
}

void NBScnRenderJobFlat_transformVertices_(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnTreeNode* n, const STNBScnFlatNode* nDst) {
	const unsigned int vFirstIdx = n->verts.iFirst;
	const unsigned int vCount = n->verts.count;
	//printf("Transforming vertices-%d (%d, +%d).\n", n->verts.type, n->verts.iFirst, n->verts.count);
	switch (NBScnTreeNode_getVertsType(n)) {
		case 0:
		{
			const unsigned int vAfterLast = vFirstIdx + vCount;
			int i; for (i = vFirstIdx; i < vAfterLast; i++) {
				const STNBScnVertex* vSrc = NBArray_itmPtrAtIndex(&src->verts.v, const STNBScnVertex, i);
				STNBScnVertexF* vDst = &obj->map.verts.v.arr[i];
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
				STNBScnVertexTexF* vDst = &obj->map.verts.v1.arr[i];
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
				STNBScnVertexTex2F* vDst = &obj->map.verts.v2.arr[i];
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
				STNBScnVertexTex3F* vDst = &obj->map.verts.v3.arr[i];
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

//

UI32 NBScnRenderJobFlat_getDispatchHeaderPaddedSz(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits) {
	NBASSERT(limits->header.alignment != 0 && (limits->header.alignment % 4) == 0)
	//NBASSERT(limits->buffer.alignment != 0 && (limits->buffer.alignment % 4) == 0)
	NBASSERT(limits->dispatch.maxThreads != 0)
	UI32 r = 0;
	if (limits != NULL && limits->header.alignment != 0 && limits->dispatch.maxThreads != 0) {
		r = (sizeof(STNBScnRenderDispatchHeader) + limits->header.alignment - 1) / limits->header.alignment * limits->header.alignment;
	}
	return r;
}

UI32 NBScnRenderJobFlat_getDispatcCallsNeeded(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits) {
	//NBASSERT(limits->header.alignment != 0 && (limits->header.alignment % 4) == 0)
	//NBASSERT(limits->buffer.alignment != 0 && (limits->buffer.alignment % 4) == 0)
	NBASSERT(limits->dispatch.maxThreads != 0)
	UI32 r = 0;
	if (limits != NULL && limits->dispatch.maxThreads != 0) {
		r = ((src->nodes.use + limits->dispatch.maxThreads - 1) / limits->dispatch.maxThreads);
	}
	return r;
}

UI32 NBScnRenderJobFlat_getDispatchHeadersBufferSz(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits) {
	NBASSERT(limits->header.alignment != 0 && (limits->header.alignment % 4) == 0)
	//NBASSERT(limits->buffer.alignment != 0 && (limits->buffer.alignment % 4) == 0)
	NBASSERT(limits->dispatch.maxThreads != 0)
	UI32 r = 0;
	if (limits != NULL && limits->header.alignment != 0 && limits->dispatch.maxThreads != 0) {
		const UI32 ammDispatchCalls = (src->nodes.use + limits->dispatch.maxThreads - 1) / limits->dispatch.maxThreads;
		const UI32 paramsPaddedSz = (sizeof(STNBScnRenderDispatchHeader) + limits->header.alignment - 1) / limits->header.alignment * limits->header.alignment;
		r = (ammDispatchCalls * paramsPaddedSz);
	}
	return r;
}

UI32 NBScnRenderJobFlat_getDispatchBufferRngs(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs) {
	NBASSERT(limits->header.alignment != 0 && (limits->header.alignment % 4) == 0)
	NBASSERT(limits->buffer.alignment != 0 && (limits->buffer.alignment % 4) == 0)
	//NBASSERT(limits->dispatch.maxThreads != 0)
	UI32 r = 0;
	NBASSERT(limits != NULL && limits->buffer.alignment != 0 && (offset % limits->buffer.alignment) == 0)
	if (limits != NULL && limits->buffer.alignment != 0 && (offset % limits->buffer.alignment) == 0) {
		UI32 iPos = offset;
		STNBScnRenderBuffRngs rDst;
		NBMemory_setZeroSt(rDst, STNBScnRenderBuffRngs);
		//
		rDst.nodes.offset	= iPos;
		rDst.nodes.use		= sizeof(STNBScnFlatNode) * src->nodes.use;
		rDst.nodes.sz		= (rDst.nodes.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		iPos				+= rDst.nodes.sz;
		//
		rDst.verts.v.offset	= iPos;
		rDst.verts.v.use	= sizeof(STNBScnVertexF) * src->verts.v.use;
		rDst.verts.v.sz		= (rDst.verts.v.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		iPos				+= rDst.verts.v.sz;
		//
		rDst.verts.v1.offset = iPos;
		rDst.verts.v1.use	= sizeof(STNBScnVertexTexF) * src->verts.v1.use;
		rDst.verts.v1.sz	= (rDst.verts.v1.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		iPos				+= rDst.verts.v1.sz;
		//
		rDst.verts.v2.offset = iPos;
		rDst.verts.v2.use	= sizeof(STNBScnVertexTex2F) * src->verts.v2.use;
		rDst.verts.v2.sz	= (rDst.verts.v2.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		iPos				+= rDst.verts.v2.sz;
		//
		rDst.verts.v3.offset = iPos;
		rDst.verts.v3.use	= sizeof(STNBScnVertexTex3F) * src->verts.v3.use;
		rDst.verts.v3.sz	= (rDst.verts.v3.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		iPos				+= rDst.verts.v3.sz;
		//
		iPos = (iPos + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
		//
		if (dstRngs != NULL) *dstRngs = rDst;
		//
		r = iPos;
	}
	return r;
}

