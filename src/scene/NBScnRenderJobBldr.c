
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnRenderJobBldr.h"
//
#include "nb/core/NBMemory.h"
#include "nb/scene/NBScnRenderJobTree.h"


typedef struct STNBScnRenderJobBldrOpq_ {
    STNBObject              prnt;
    STNBScnRenderJobTree    tree;
} STNBScnRenderJobBldrOpq;

NB_OBJREF_BODY(NBScnRenderJobBldr, STNBScnRenderJobBldrOpq, NBObject)

void NBScnRenderJobBldr_initZeroed(STNBObject* obj) {
    STNBScnRenderJobBldrOpq* opq    = (STNBScnRenderJobBldrOpq*)obj;
    NBScnRenderJobTree_init(&opq->tree);
}

void NBScnRenderJobBldr_uninitLocked(STNBObject* obj){
    STNBScnRenderJobBldrOpq* opq = (STNBScnRenderJobBldrOpq*)obj;
    NBScnRenderJobTree_release(&opq->tree);
}

//tree node
BOOL NBScnRenderJobBldr_nodePush(STNBScnRenderJobBldrRef obj);
BOOL NBScnRenderJobBldr_nodePop(STNBScnRenderJobBldrRef obj);

//vertices allocation

STNBScnVertex*      NBScnRenderJobBldr_allocVerts(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex*   NBScnRenderJobBldr_allocVertsTex(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex2*  NBScnRenderJobBldr_allocVertsTex2(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex3*  NBScnRenderJobBldr_allocVertsTex3(STNBScnRenderJobBldrRef obj, const UI32 amm);
