
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnRenderJobTree.h"
#include "nb/core/NBMemory.h"
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

//

UI32 NBScnRenderJobTree_getDispatchBufferRngs(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs) {
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
			rDst.nodes.offset = iPos;
			rDst.nodes.use		= sizeof(STNBScnTreeNode) * src->nodes.use;
			rDst.nodes.sz		= (rDst.nodes.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
			iPos				+= rDst.nodes.sz;
			//
			rDst.verts.v.offset	= iPos;
			rDst.verts.v.use	= sizeof(STNBScnVertex) * src->verts.v.use;
			rDst.verts.v.sz		= (rDst.verts.v.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
			iPos				+= rDst.verts.v.sz;
			//
			rDst.verts.v1.offset = iPos;
			rDst.verts.v1.use	= sizeof(STNBScnVertexTex) * src->verts.v1.use;
			rDst.verts.v1.sz	= (rDst.verts.v1.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
			iPos				+= rDst.verts.v1.sz;
			//
			rDst.verts.v2.offset = iPos;
			rDst.verts.v2.use	= sizeof(STNBScnVertexTex2) * src->verts.v2.use;
			rDst.verts.v2.sz	= (rDst.verts.v2.use + limits->buffer.alignment - 1) / limits->buffer.alignment * limits->buffer.alignment;
			iPos				+= rDst.verts.v2.sz;
			//
			rDst.verts.v3.offset = iPos;
			rDst.verts.v3.use	= sizeof(STNBScnVertexTex3) * src->verts.v3.use;
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
