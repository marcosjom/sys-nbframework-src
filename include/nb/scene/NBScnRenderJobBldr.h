#ifndef NB_SCN_RENDER_JOB_BUILDER_H
#define NB_SCN_RENDER_JOB_BUILDER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"

#include "nb/scene/NBScnVertices.h"

#ifdef __cplusplus
extern "C" {
#endif

NB_OBJREF_HEADER(NBScnRenderJobBldr)

//vertices allocation

STNBScnVertex*      NBScnRenderJobBldr_allocVerts(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex*   NBScnRenderJobBldr_allocVertsTex(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex2*  NBScnRenderJobBldr_allocVertsTex2(STNBScnRenderJobBldrRef obj, const UI32 amm);
STNBScnVertexTex3*  NBScnRenderJobBldr_allocVertsTex3(STNBScnRenderJobBldrRef obj, const UI32 amm);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
