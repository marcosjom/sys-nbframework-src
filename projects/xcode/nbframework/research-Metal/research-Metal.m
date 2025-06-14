
#include "nb/NBFrameworkPch.h"

#include <stdio.h>
//
#include <assert.h>    //assert
//
#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <Metal/MTLBuffer.h>
//
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBThread.h"
#include "nb/research/research-scn-compute.h"
//

//#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

/*typedef struct CUSTOMVERTEX_ {
    FLOAT x, y, z, rhw;    // from the D3DFVF_XYZRHW flag
    DWORD color;    // from the D3DFVF_DIFFUSE flag
} CUSTOMVERTEX;*/

//

typedef struct STD3DCompute_ {
    //dev
    struct {
        int dummy;
    } dev;
    //shader
    struct {
        int dummy;
    } shader;
} STD3DCompute;

typedef struct STApp_ {
    //win
    struct {
        int         width;
        int         height;
        //curSec
        struct {
            int        msgsCountPaint;
        } curSec;
        //refresh
        struct {
            int        msAccum;
        } refresh;
    } win;
    //mtl
    struct {
        //dev
        struct {
            id<MTLDevice> obj;
            //lib
            struct {
                id<MTLLibrary> def;     //default metal library
                id<MTLFunction> func;   //shader
            } lib;
            //pipe
            struct {
                id<MTLComputePipelineState> obj;
            } pipeln;
            //cmds (queue)
            struct {
                id<MTLCommandQueue> obj;
            } cmds;
        } dev;
        STD3DCompute compute;
    } mtl;
} STApp;

//app
void App_init(STApp* obj);
void App_release(STApp* obj);
//
void App_metal_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs);
//
bool App_metal_compute_create(STApp* app);

#define BYTE_TO_BINARY_CHARS(byte)  \
    ((byte) & 0x80 ? '1' : '0'), \
    ((byte) & 0x40 ? '1' : '0'), \
    ((byte) & 0x20 ? '1' : '0'), \
    ((byte) & 0x10 ? '1' : '0'), \
    ((byte) & 0x08 ? '1' : '0'), \
    ((byte) & 0x04 ? '1' : '0'), \
    ((byte) & 0x02 ? '1' : '0'), \
    ((byte) & 0x01 ? '1' : '0')

int main() {
#    ifdef _DEBUG
    printf("_DEBUG is defined.\n");
#    else
    printf("_DEBUG is undefined.\n");
#    endif
    NBMngrProcess_init();
    NBMngrStructMaps_init();
    //
    {
        //unsigned int dval = 0;
        //unsigned int rrBase = 0;
        //unsigned int rr = rrBase;
        //unsigned char* rrb = (unsigned char*) &rr;
        //BIT
        /*
        dval = 0;
        rrBase = ~NB_SCN_TREE_NODE_ISDISABLED_BIT;
        rr = rrBase;
        printf("Initial value: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = (((rrBase) / (NB_SCN_TREE_NODE_ISDISABLED_BIT * 2) * (NB_SCN_TREE_NODE_ISDISABLED_BIT * 2)));
        printf("         Left: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = (((1) % 2) * NB_SCN_TREE_NODE_ISDISABLED_BIT);
        printf("         Bit: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = ((rrBase) % NB_SCN_TREE_NODE_ISDISABLED_BIT);
        printf("         Rigth: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = NBScnTreeNode_withIsDisabled(~NB_SCN_TREE_NODE_ISDISABLED_BIT, 1);
        printf("         Final: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = NB_SCN_TREE_NODE_ISDISABLED_BIT;
        printf("         Compr: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        //MASKED-VALUE
        dval = 1;
        rrBase = 0xFFFFFFFF;
        rr = rrBase;
        printf("Initial value: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = (((rrBase / 2) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST) * 2);
        printf("         Left: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = (dval % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1)) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST;
        printf("          Bit: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        rr = ((rrBase) % NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST);
        printf("        Rigth: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        //rr = NBScnTreeNode_withIsDisabled(~NB_SCN_TREE_NODE_ISDISABLED_BIT, 1);
        //printf("         Final: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        //rr = NB_SCN_TREE_NODE_ISDISABLED_BIT;
        //printf("         Compr: %c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c-%c%c%c%c%c%c%c%c.\n", BYTE_TO_BINARY_CHARS(rrb[3]), BYTE_TO_BINARY_CHARS(rrb[2]), BYTE_TO_BINARY_CHARS(rrb[1]), BYTE_TO_BINARY_CHARS(rrb[0]));
        //(((((B) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1)) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) + (V % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1)) + ((B) % NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST))
        //
        NBASSERT((rr = NBScnTreeNode_withIsHidden(~NB_SCN_TREE_NODE_ISHIDDEN_BIT, 0)) == ~NB_SCN_TREE_NODE_ISHIDDEN_BIT);
        NBASSERT((rr = NBScnTreeNode_withIsHidden(~NB_SCN_TREE_NODE_ISHIDDEN_BIT, 1)) == 0xFFFFFFFF);
        NBASSERT((rr = NBScnTreeNode_withIsDisabled(~NB_SCN_TREE_NODE_ISDISABLED_BIT, 0)) == ~NB_SCN_TREE_NODE_ISDISABLED_BIT);
        NBASSERT((rr = NBScnTreeNode_withIsDisabled(~NB_SCN_TREE_NODE_ISDISABLED_BIT, 1)) == 0xFFFFFFFF);
        //
        dval = 0;
        NBASSERT((rr = NBScnTreeNode_withVertsType(0xFFFFFFFF, dval)) == ((0xFFFFFFFF & ~NB_SCN_TREE_NODE_VERTS_TYPE_MSK) | ((dval & NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST)));
        dval = 1;
        NBASSERT((rr = NBScnTreeNode_withVertsType(0xFFFFFFFF, dval)) == ((0xFFFFFFFF & ~NB_SCN_TREE_NODE_VERTS_TYPE_MSK) | ((dval & NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST)));
        dval = 2;
        NBASSERT((rr = NBScnTreeNode_withVertsType(0xFFFFFFFF, dval)) == ((0xFFFFFFFF & ~NB_SCN_TREE_NODE_VERTS_TYPE_MSK) | ((dval & NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST)));
        dval = 3;
        NBASSERT((rr = NBScnTreeNode_withVertsType(0xFFFFFFFF, dval)) == ((0xFFFFFFFF & ~NB_SCN_TREE_NODE_VERTS_TYPE_MSK) | ((dval & NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX) * NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST)));
        */
    }
    //
    STApp app;
    App_init(&app);
    if(!App_metal_compute_create(&app)){
        printf("App, error, App_metal_compute_create failed.\n");
    } else {
        const float compareMaxDiffAbs = 0.1f;
        //static samples
        {
            static STNBScnTreeNode __testTree[] = {
                //root
                {
                    0 /*iParent*/
                    , NBScnTreeNode_withChildCount(0, 2) /*pck*/
                    , { 0.0f /*x*/, 0.0f /*y*/, 0.0f /*deg*/, 2.0f /*scale*/, 3.0f /*scale*/}
                    , { (BYTE)(255.0f * 1.0f) /*r*/, (BYTE)(255.0f * 1.0f) /*g*/, (BYTE)(255.0f * 1.0f) /*b*/, (BYTE)(255.0f * 1.0f) /*a*/}
                    , { 0 /*iFirst*/, 0 /*count*/}
                }
                    //child-0
                    ,{
                        0 /*iParent*/
                        , NBScnTreeNode_withChildCount(NBScnTreeNode_withIsHidden(0, 1), 1) /*pck*/
                        , { 10.0f /*x*/, 20.0f /*y*/, 0.0f /*deg*/, 1.0f /*scale*/, 1.0f /*scale*/}
                        , { (BYTE)(255.0f * 0.5f) /*r*/, (BYTE)(255.0f * 0.75f) /*g*/, (BYTE)(255.0f * 0.9f) /*b*/, (BYTE)(255.0f * 0.1f) /*a*/}
                        , { 0 /*iFirst*/, 0 /*count*/}
                    }
                        //grand-child-0
                        ,{
                            1 /*iParent*/
                            , NBScnTreeNode_withIsDisabled(0, 1) /*pck*/
                            , { 0.0f /*x*/, 0.0f /*y*/, 90.0f /*deg*/, 1.0f /*scale*/, 1.0f /*scale*/}
                            , { (BYTE)(255.0f * 0.9f) /*r*/, (BYTE)(255.0f * 0.25f) /*g*/, (BYTE)(255.0f * 0.3f) /*b*/, (BYTE)(255.0f * 0.5f) /*a*/}
                            , { 0 /*iFirst*/, 0 /*count*/}
                        }
                    //child-1
                    ,{
                        0 /*iParent*/
                        , 0 /*pck*/
                        , { 1.0f /*x*/, 2.0f /*y*/, 45.0f /*deg*/, 10.0f /*scale*/, 20.0f /*scale*/}
                        , { (BYTE)(255.0f * 0.75f) /*r*/, (BYTE)(255.0f * 0.75f) /*g*/, (BYTE)(255.0f * 0.75f) /*b*/, (BYTE)(255.0f * 1.0f) /*a*/}
                        , { 0 /*iFirst*/, 0 /*count*/}
                    }
            };
            const int srcCount = (sizeof(__testTree) / sizeof(__testTree[0]));
            STNBScnRenderJobTree tree;
            NBScnRenderJobTree_init(&tree);
            {
                NBArray_addItems(&tree.nodes, __testTree, sizeof(__testTree[0]), srcCount);
                //compute
                printf("\n\n");
                printf("Testing %d samples.\n", srcCount);
                App_metal_compute_run_in_samples(&app, &tree, compareMaxDiffAbs);
            }
            NBScnRenderJobTree_release(&tree);
        }
        //generated samples
        {
            const int amm[] = { /*100,*/ 1000, 10000, 100000, 100000, 1000000 };
            int i; for (i = 0; i < (sizeof(amm) / sizeof(amm[0])); i++) {
                STNBScnRenderJobTree tree;
                NBScnRenderJobTree_init(&tree);
                {
                    research_scn_compute_gen_random_arrays(amm[i], &tree);
                    printf("\n\n");
                    printf("Testing %d samples (%d vertices, %d vTex1, %d vTex2, %d vTex3).\n", tree.nodes.use, tree.verts.v.use, tree.verts.v1.use, tree.verts.v2.use, tree.verts.v3.use);
                    //validate
                    research_scn_compute_validate_arrays(&tree);
                    //compute
                    App_metal_compute_run_in_samples(&app, &tree, compareMaxDiffAbs);
                }
                NBScnRenderJobTree_release(&tree);
            }
        }
    }
    App_release(&app);
    //sleep few seconds before exit
    //Sleep(1000 * 3);
    //
    NBMngrStructMaps_release();
    NBMngrProcess_release();
}

bool App_metal_compute_create(STApp* app){
    bool r = false;
    NSArray<id<MTLDevice>> *devs = MTLCopyAllDevices();
    if(devs == nil){
        printf("Metal, error, could not retrieve devices.\n");
    } else {
        printf("Metal, %d devices found:\n", (int)devs.count);
        id<MTLDevice> dev = nil;
        int i; for(i = 0; i < devs.count; i++){
            id<MTLDevice> d = devs[i];
            printf("    ---------\n");
            printf("    Dev#%d/%d: '%s'\n", (i + 1), (int)devs.count, [d.name UTF8String]);
            //Identification
            printf("         Arch: '%s'\n", [d.architecture.name UTF8String]);
            printf("          Loc: '%s' (num: %d)\n", d.location == MTLDeviceLocationBuiltIn ? "BuiltIn" : d.location == MTLDeviceLocationSlot ? "Slot" : d.location == MTLDeviceLocationExternal ? "External" : d.location == MTLDeviceLocationUnspecified ? "Unspecified" :"Unknown", (int)d.locationNumber);
            printf("       LowPwr: %s\n", d.isLowPower ? "yes" : "no");
            printf("    Removable: %s\n", d.isRemovable ? "yes" : "no");
            printf("     Headless: %s\n", d.isHeadless ? "yes" : "no");
            printf("         Peer: grpId(%llu) idx(%d) count(%d)\n", (UI64)d.peerGroupID, d.peerIndex, d.peerCount);
            //GPU's Device Memory
            printf("     CurAlloc: %.2f %s\n", (double)d.currentAllocatedSize / (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.currentAllocatedSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.currentAllocatedSize >= (1024) ? (double)(1024) : 1.0), (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? "GBs" : d.currentAllocatedSize >= (1024 * 1024) ? "MBs" : d.currentAllocatedSize >= (1024) ? "KBs" : "bytes"));
            printf("     MaxAlloc: %.2f %s (recommended)\n", (double)d.recommendedMaxWorkingSetSize / (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024) ? (double)(1024) : 1.0), (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? "GBs" : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? "MBs" : d.recommendedMaxWorkingSetSize >= (1024) ? "KBs" : "bytes"));
            printf("      MaxRate: %.2f %s/s\n", (double)d.maxTransferRate / (d.maxTransferRate >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxTransferRate >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxTransferRate >= (1024) ? (double)(1024) : 1.0), (d.maxTransferRate >= (1024 * 1024 * 1024) ? "GBs" : d.maxTransferRate >= (1024 * 1024) ? "MBs" : d.maxTransferRate >= (1024) ? "KBs" : "bytes"));
            printf("   UnifiedMem: %s\n", d.hasUnifiedMemory ? "yes" : "no");
            //Compute Support
            printf(" ThreadGrpMem: %.2f %s\n", (double)d.maxThreadgroupMemoryLength / (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024) ? (double)(1024) : 1.0), (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? "GBs" : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? "MBs" : d.maxThreadgroupMemoryLength >= (1024) ? "KBs" : "bytes"));
            printf("  ThrdsPerGrp: (%d, %d, %d)\n", (int)d.maxThreadsPerThreadgroup.width, (int)d.maxThreadsPerThreadgroup.height, (int)d.maxThreadsPerThreadgroup.depth);
            //Functions Pointer Support
            printf("     FuncPtrs: %s (compute kernel functions)\n", d.supportsFunctionPointers ? "yes" : "no");
            printf("    FPtrsRndr: %s\n", d.supportsFunctionPointersFromRender ? "yes" : "no");
            //Texture and sampler support
            printf("   32bFltFilt: %s\n", d.supports32BitFloatFiltering ? "yes" : "no");
            printf("   BCTextComp: %s\n", d.supportsBCTextureCompression ? "yes" : "no");
            printf("    Depth24-8: %s\n", d.isDepth24Stencil8PixelFormatSupported ? "yes" : "no");
            printf("  TexLODQuery: %s\n", d.supportsQueryTextureLOD ? "yes" : "no");
            printf("        RWTex: %s\n", d.readWriteTextureSupport ? "yes" : "no");
            //Render support
            printf("   RayTracing: %s\n", d.supportsRaytracing ? "yes" : "no");
            printf("  RayTracRndr: %s\n", d.supportsRaytracingFromRender ? "yes" : "no");
            printf("  PrimMotBlur: %s\n", d.supportsPrimitiveMotionBlur ? "yes" : "no");
            printf("      32bMSAA: %s\n", d.supports32BitMSAA ? "yes" : "no");
            printf("  PullModeInt: %s\n", d.supportsPullModelInterpolation ? "yes" : "no");
            printf("ShadBaryCoord: %s\n", d.supportsShaderBarycentricCoordinates ? "yes" : "no");
            printf("  ProgSmplPos: %s\n", d.areProgrammableSamplePositionsSupported ? "yes" : "no");
            printf("  RstrOrdGrps: %s\n", d.areRasterOrderGroupsSupported ? "yes" : "no");
            //
            if(
               dev == nil //first device is default
               || (dev.isHeadless && !d.isHeadless) //allways prefer non-headless devices
               || (dev.isRemovable && !d.isRemovable) //allways prefer non-removable devices
               || (!dev.hasUnifiedMemory && d.hasUnifiedMemory) //allways prefer unified memory devices
               )
            {
                if(dev != nil){
                    [dev release];
                    dev = nil;
                }
                dev = d;
                [dev retain];
            }
        }
        if(dev == nil){
            printf("Metal, error, could select a device.\n");
        } else {
            printf("Prefered device: '%s'\n", [dev.name UTF8String]);
            id<MTLLibrary> defLib = [dev newDefaultLibrary];
            if (defLib == nil){
                printf("Metal, error, newDefaultLibrary failed.\n");
            } else {
                id<MTLFunction> libFunc = [defLib newFunctionWithName:@"compute_tree"];
                if (libFunc == nil){
                    printf("Metal, error, newFunctionWithName failed.\n");
                } else {
                    NSError* error = nil;
                    //This compiles the 'function' into the pipeline architecture, call outside sensite code.
                    id<MTLComputePipelineState> pipeln = [dev newComputePipelineStateWithFunction: libFunc error:&error];
                    if(pipeln == nil){
                        printf("Metal, error, newComputePipelineStateWithFunction failed: '%s'.\n", error != nil ? [error.description UTF8String] : "[no-error-msg]");
                    } else {
                        id<MTLCommandQueue> cmds = [dev newCommandQueue];
                        if(cmds == nil){
                            printf("Metal, error, newCommandQueue failed.\n");
                        } else {
                            //results
                            app->mtl.dev.cmds.obj = cmds; cmds = nil; //consume
                            app->mtl.dev.pipeln.obj = pipeln; pipeln = nil; //consume
                            app->mtl.dev.lib.def = defLib; defLib = nil; //consume
                            app->mtl.dev.lib.func = libFunc; libFunc = nil; //consume
                            app->mtl.dev.obj = dev; dev = nil; //consume
                            r = true;
                        }
                        //release (if not consumed)
                        if(cmds != nil){
                            [cmds release];
                            cmds = nil;
                        }
                    }
                    //release (if not consumed)
                    if(pipeln != nil){
                        [pipeln release];
                        pipeln = nil;
                    }
                }
                //release (if not consumed)
                if(libFunc != nil){
                    [libFunc release];
                    libFunc = nil;
                }
            }
            //release (if not consumed)
            if(defLib != nil){
                [defLib release];
                defLib = nil;
            }
        }
        //release (if not consumed)
        if(dev != nil){
            [dev release];
            dev = nil;
        }
    }
    if(devs != nil){
        [devs release];
        devs = nil;
    }
    return r;
}

#define METAL_NON_ZERO_BUFF_SZ(SZ, MIN)  ((SZ) == 0 ? (MIN) : ((SZ) + (MIN) - 1) / MIN * MIN)

void App_metal_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs) {
    //const unsigned int spacesPerLvl = 4;
    NBTHREAD_CLOCK osFreq = NBThread_clocksPerSec();
    NBTHREAD_CLOCK cpuFwdTime = 0, cpuBwdTime = 0, cpuBwd2Time = 0, gpuBwdTimeExec = 0, gpuBwdTimeMapping = 0, gpuBwdTimeCpying = 0;
    STNBScnRenderJobPlain cpuFwdRR, cpuBwdRR, cpuBwdRR2, gpuBwdRRMapped, gpuBwdRRCopied;
    //
    //QueryPerformanceFrequency(&osFreq);
    //
    NBScnRenderJobPlain_init(&cpuFwdRR);
    NBScnRenderJobPlain_init(&cpuBwdRR);
    NBScnRenderJobPlain_init(&cpuBwdRR2);    //gpu shader code running in cpu
    NBScnRenderJobPlain_init(&gpuBwdRRMapped);
    NBScnRenderJobPlain_init(&gpuBwdRRCopied);
    //
    NBScnRenderJobPlain_prepare(&cpuFwdRR, src); //preallocate (cpu)
    NBScnRenderJobPlain_prepare(&cpuBwdRR, src); //preallocate (cpu)
    NBScnRenderJobPlain_prepare(&cpuBwdRR2, src); //preallocate (cpu)
    //NBScnRenderJobPlain_prepare(&cpuBwdRRMapped, src); //do not preallocate mapped version (will point to gpu buffers)
    NBScnRenderJobPlain_prepare(&gpuBwdRRCopied, src); //preallocate (gpu)
    //
    //Run in CPU
    {
        {
            NBTHREAD_CLOCK startTime, endTime;
            startTime = NBThread_clock();
            {
                NBScnRenderJob_convertTreeToPlainForwardToDst(src, &cpuFwdRR);
            }
            endTime = NBThread_clock();
            cpuFwdTime = endTime - startTime;
            //printf("RESULTS :: CPU :: forward execution:\n");
            //printf("---------------------->\n");
            //research_scn_compute_print_flat_job(&cpuFwdRR, spacesPerLvl);
            //printf("<----------------------\n");
        }
        {
            NBTHREAD_CLOCK startTime, endTime;
            startTime = NBThread_clock();
            {
                NBScnRenderJob_convertTreeToPlainBackwardToDst(src, &cpuBwdRR);
            }
            endTime = NBThread_clock();
            cpuBwdTime = endTime - startTime;
            //printf("RESULTS :: CPU :: backward execution:\n");
            //printf("---------------------->\n");
            //research_scn_compute_print_flat_job(&cpuBwdRR, spacesPerLvl);
            //printf("<----------------------\n");
        }
        {
            NBTHREAD_CLOCK startTime, endTime;
            startTime = NBThread_clock();
            {
                research_scn_compute_convertTreeToPlainGpuAlgorithmToDst(src, &cpuBwdRR2);
            }
            endTime = NBThread_clock();
            cpuBwd2Time = endTime - startTime;
            //printf("RESULTS :: CPU :: backward execution:\n");
            //printf("---------------------->\n");
            //research_scn_compute_print_flat_job(&cpuBwdRR, spacesPerLvl);
            //printf("<----------------------\n");
        }
        {
            float matchRel = 0.0f;
            NBTHREAD_CLOCK startTime, endTime;
            endTime = startTime = NBThread_clock();
            if ((matchRel = research_scn_compute_compare(&cpuFwdRR, &cpuBwdRR, compareMaxDiffAbs, src)) > 0.9f) {
                endTime = NBThread_clock();
                printf("cpu_forward vs cpu_backwards: MATCH (%.4fms; %.2f%%).\n", (float)(endTime - startTime) / (float)(osFreq / 1000), 100.0f * matchRel);
            } else {
                printf("cpu_forward vs cpu_backwards: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
            }
        }
        {
            float matchRel = 0.0f;
            NBTHREAD_CLOCK startTime, endTime;
            endTime = startTime = NBThread_clock();
            if ((matchRel = research_scn_compute_compare(&cpuFwdRR, &cpuBwdRR2, compareMaxDiffAbs, src)) > 0.9f) {
                endTime = NBThread_clock();
                printf("cpu_forward vs gpu_backwards_on_cpu: MATCH (%.4fms; %.2f%%).\n", (float)(endTime - startTime) / (float)(osFreq / 1000), 100.0f * matchRel);
            } else {
                printf("cpu_forward vs gpu_backwards_on_cpu: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
            }
        }
    }
    //Run in GPU
    {
        typedef struct STParams_ {
            UI32 iNodeOffset; //nodes already executed on previous Dispatch() calls
        } STParams;
        id<MTLBuffer> paramsBuff = nil;
        id<MTLBuffer> treNodesBuff = nil, treV0Buff = nil, treV1Buff = nil, treV2Buff = nil, treV3Buff = nil;
        id<MTLBuffer> fltNodesBuff = nil, fltV0Buff = nil, fltV1Buff = nil, fltV2Buff = nil, fltV3Buff = nil;
        const unsigned long maxValPerDispatch = app->mtl.dev.pipeln.obj.maxTotalThreadsPerThreadgroup;
        const unsigned long ammDispatchCalls = (src->nodes.use + maxValPerDispatch - 1) / maxValPerDispatch;
        if(nil == (paramsBuff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STParams) * ammDispatchCalls, 32) options:MTLResourceStorageModeShared])){
            printf("Metal, error, paramsBuff allocation failed.\n");
        } else if(nil == (treNodesBuff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnTreeNode) * src->nodes.use, 32) options:MTLResourceStorageModeShared])){
            printf("Metal, error, treNodesBuff allocation failed.\n");
        } else if(nil == (treV0Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertex) * src->verts.v.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV0Buff allocation failed.\n");
        } else if(nil == (treV1Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTex) * src->verts.v1.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV1Buff allocation failed.\n");
        } else if(nil == (treV2Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTex2) * src->verts.v2.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV2Buff allocation failed.\n");
        } else if(nil == (treV3Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTex3) * src->verts.v3.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV3Buff allocation failed.\n");
            //
            //
        } else if(nil == (fltNodesBuff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnFlatNode) * src->nodes.use, 32) options:MTLResourceStorageModeShared])){
            printf("Metal, error, treNodesBuff allocation failed.\n");
        } else if(nil == (fltV0Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexF) * src->verts.v.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV0Buff allocation failed.\n");
        } else if(nil == (fltV1Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTexF) * src->verts.v1.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV1Buff allocation failed.\n");
        } else if(nil == (fltV2Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTex2F) * src->verts.v2.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV2Buff allocation failed.\n");
        } else if(nil == (fltV3Buff = [app->mtl.dev.obj newBufferWithLength:METAL_NON_ZERO_BUFF_SZ(sizeof(STNBScnVertexTex3F) * src->verts.v3.use, 32) options:MTLResourceStorageModeShared])){
            printf("Vulkan, error, treV3Buff allocation failed.\n");
            //
            //
        } else {
            //populate exec params
            {
                unsigned int execCount = 0;
                STParams execPrms;
                NBMemory_setZeroSt(execPrms, STParams);
                while (execPrms.iNodeOffset < src->nodes.use) {
                    execCount = src->nodes.use - execPrms.iNodeOffset;
                    if (execCount > maxValPerDispatch) {
                        execCount = (UI32)maxValPerDispatch;
                    }
                    //populate step param
                    memcpy(&paramsBuff.contents[sizeof(execPrms) * (execPrms.iNodeOffset / maxValPerDispatch)], (void*)&execPrms, sizeof(execPrms));
                    //next
                    execPrms.iNodeOffset += execCount;
                }
            }
            //populate data
            memcpy(treNodesBuff.contents, (void*)NBArray_dataPtr(&src->nodes, STNBScnTreeNode*), sizeof(STNBScnTreeNode) * src->nodes.use);
            memcpy(treV0Buff.contents, (void*)NBArray_dataPtr(&src->verts.v, STNBScnVertex*), sizeof(STNBScnVertex) * src->verts.v.use);
            memcpy(treV1Buff.contents, (void*)NBArray_dataPtr(&src->verts.v1, STNBScnVertexTex*), sizeof(STNBScnVertexTex) * src->verts.v1.use);
            memcpy(treV2Buff.contents, (void*)NBArray_dataPtr(&src->verts.v2, STNBScnVertexTex2*), sizeof(STNBScnVertexTex2) * src->verts.v2.use);
            memcpy(treV3Buff.contents, (void*)NBArray_dataPtr(&src->verts.v3, STNBScnVertexTex3*), sizeof(STNBScnVertexTex3) * src->verts.v3.use);
            //
            NBTHREAD_CLOCK startTime, midTime, midTime2, endTime;
            midTime = midTime2 = endTime = startTime = NBThread_clock();
            {
                id<MTLCommandBuffer> cmdBuff = nil;
                id<MTLComputeCommandEncoder> cmdEnc = nil;
                if(nil == (cmdBuff = [app->mtl.dev.cmds.obj commandBuffer])){
                    printf("Vulkan, error, cmdBuff allocation failed.\n");
                } else if(nil == (cmdEnc = [cmdBuff computeCommandEncoder])){
                    printf("Vulkan, error, cmdEnc allocation failed.\n");
                } else {
                    [cmdEnc setComputePipelineState:app->mtl.dev.pipeln.obj];
                    //
                    [cmdEnc setBuffer:treNodesBuff offset:0 atIndex:1];
                    [cmdEnc setBuffer:treV0Buff offset:0 atIndex:2];
                    [cmdEnc setBuffer:treV1Buff offset:0 atIndex:3];
                    [cmdEnc setBuffer:treV2Buff offset:0 atIndex:4];
                    [cmdEnc setBuffer:treV3Buff offset:0 atIndex:5];
                    //
                    [cmdEnc setBuffer:fltNodesBuff offset:0 atIndex:6];
                    [cmdEnc setBuffer:fltV0Buff offset:0 atIndex:7];
                    [cmdEnc setBuffer:fltV1Buff offset:0 atIndex:8];
                    [cmdEnc setBuffer:fltV2Buff offset:0 atIndex:9];
                    [cmdEnc setBuffer:fltV3Buff offset:0 atIndex:10];
                    //configure steps
                    {
                        unsigned int execCount = 0;
                        STParams execPrms;
                        NBMemory_setZeroSt(execPrms, STParams);
                        while (execPrms.iNodeOffset < src->nodes.use) {
                            execCount = src->nodes.use - execPrms.iNodeOffset;
                            if (execCount > maxValPerDispatch) {
                                execCount = (UI32)maxValPerDispatch;
                            }
                            //set step param
                            {
                                MTLSize gridSize = MTLSizeMake(execCount, 1, 1);
                                MTLSize threadgroupSize = MTLSizeMake(execCount, 1, 1);
                                [cmdEnc setBuffer:paramsBuff offset:(sizeof(execPrms) * (execPrms.iNodeOffset / maxValPerDispatch)) atIndex:0];
                                [cmdEnc dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
                            }
                            //next
                            execPrms.iNodeOffset += execCount;
                        }
                    }
                    [cmdEnc endEncoding];
                    [cmdBuff commit];
                    [cmdBuff waitUntilCompleted];
                }
                if(cmdEnc != nil) [cmdEnc release];
                if(cmdBuff != nil) [cmdBuff release];
            }
            midTime2 = endTime = midTime = NBThread_clock();
            gpuBwdTimeExec = midTime - startTime;
            //map
            {
                NBArray_release(&gpuBwdRRMapped.nodes);
                NBArray_release(&gpuBwdRRMapped.verts.v);
                NBArray_release(&gpuBwdRRMapped.verts.v1);
                NBArray_release(&gpuBwdRRMapped.verts.v2);
                NBArray_release(&gpuBwdRRMapped.verts.v3);
                //
                NBArray_initWithExternalBuffer(&gpuBwdRRMapped.nodes, sizeof(STNBScnFlatNode), NULL, (BYTE*)fltNodesBuff.contents);
                gpuBwdRRMapped.nodes.use = src->nodes.use;
                NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v, sizeof(STNBScnVertexF), NULL, (BYTE*)fltV0Buff.contents);
                gpuBwdRRMapped.verts.v.use = src->verts.v.use;
                NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v1, sizeof(STNBScnVertexTexF), NULL, (BYTE*)fltV1Buff.contents);
                gpuBwdRRMapped.verts.v1.use = src->verts.v1.use;
                NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v2, sizeof(STNBScnVertexTex2F), NULL, (BYTE*)fltV2Buff.contents);
                gpuBwdRRMapped.verts.v2.use = src->verts.v2.use;
                NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v3, sizeof(STNBScnVertexTex3F), NULL, (BYTE*)fltV3Buff.contents);
                gpuBwdRRMapped.verts.v3.use = src->verts.v3.use;
            }
            endTime = midTime2 = NBThread_clock();
            gpuBwdTimeMapping = midTime2 - midTime;
            //copy
            {
                NBArray_empty(&gpuBwdRRCopied.nodes);
                NBArray_empty(&gpuBwdRRCopied.verts.v);
                NBArray_empty(&gpuBwdRRCopied.verts.v1);
                NBArray_empty(&gpuBwdRRCopied.verts.v2);
                NBArray_empty(&gpuBwdRRCopied.verts.v3);
                NBArray_addItems(&gpuBwdRRCopied.nodes, NBArray_dataPtr(&gpuBwdRRMapped.nodes, const STNBScnFlatNode), sizeof(STNBScnFlatNode), gpuBwdRRMapped.nodes.use);
                NBArray_addItems(&gpuBwdRRCopied.verts.v, NBArray_dataPtr(&gpuBwdRRMapped.verts.v, const STNBScnVertexF), sizeof(STNBScnVertexF), gpuBwdRRMapped.verts.v.use);
                NBArray_addItems(&gpuBwdRRCopied.verts.v1, NBArray_dataPtr(&gpuBwdRRMapped.verts.v1, const STNBScnVertexTexF), sizeof(STNBScnVertexTexF), gpuBwdRRMapped.verts.v1.use);
                NBArray_addItems(&gpuBwdRRCopied.verts.v2, NBArray_dataPtr(&gpuBwdRRMapped.verts.v2, const STNBScnVertexTex2F), sizeof(STNBScnVertexTex2F), gpuBwdRRMapped.verts.v2.use);
                NBArray_addItems(&gpuBwdRRCopied.verts.v3, NBArray_dataPtr(&gpuBwdRRMapped.verts.v3, const STNBScnVertexTex3F), sizeof(STNBScnVertexTex3F), gpuBwdRRMapped.verts.v3.use);
            }
            endTime = NBThread_clock();
            gpuBwdTimeCpying = endTime - midTime2;
            //compare
            {
                float matchRel = 0.0f;
                NBTHREAD_CLOCK startTime, endTime;
                endTime = startTime  = NBThread_clock();
                if ((matchRel = research_scn_compute_compare(&gpuBwdRRMapped, &cpuBwdRR, compareMaxDiffAbs, src)) > 0.9f) {
                    endTime = NBThread_clock();
                    printf("gpu_backwards_maped vs cpu_backwards: MATCH (%.4fms; %.2f%%).\n", (float)(endTime - startTime) / (float)(osFreq / 1000), 100.0f * matchRel);
                    //compare with processing time in copied
                    {
                        float matchRel = 0.0f;
                        NBTHREAD_CLOCK startTime, endTime;
                        endTime = startTime = NBThread_clock();
                        if ((matchRel = research_scn_compute_compare(&gpuBwdRRCopied, &cpuBwdRR, compareMaxDiffAbs, src)) > 0.9f) {
                            endTime = NBThread_clock();
                            printf("gpu_backwards_copied vs cpu_backwards: MATCH (%.4fms; %.2f%%).\n", (float)(endTime - startTime) / (float)(osFreq / 1000), 100.0f *matchRel);
                        } else {
                            printf("gpu_backwards_copied vs cpu_backwards: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
                        }
                    }
                } else {
                    printf("gpu_backwards_maped vs cpu_backwards: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
                }
            }
        }
        if(paramsBuff != nil) [paramsBuff release];
        //
        if(treNodesBuff != nil) [treNodesBuff release];
        if(treV0Buff != nil) [treV0Buff release];
        if(treV1Buff != nil) [treV1Buff release];
        if(treV2Buff != nil) [treV2Buff release];
        if(treV3Buff != nil) [treV3Buff release];
        //
        if(fltNodesBuff != nil) [fltNodesBuff release];
        if(fltV0Buff != nil) [fltV0Buff release];
        if(fltV1Buff != nil) [fltV1Buff release];
        if(fltV2Buff != nil) [fltV2Buff release];
        if(fltV3Buff != nil) [fltV3Buff release];
    }
    printf("Execution times (%d elems): cpu_fwd(%.4fms), cpu_bwd(%.4fms), gpu_bwd_on_cpu(%.4fms), gpu_bwd(%.4fms exc, +%.4fms map, +%.4fms cpy).\n\n\n"
        , src->nodes.use
        , (float)(cpuFwdTime) / (float)(osFreq / 1000)
        , (float)(cpuBwdTime) / (float)(osFreq / 1000)
        , (float)(cpuBwd2Time) / (float)(osFreq / 1000)
        , (float)(gpuBwdTimeExec) / (float)(osFreq / 1000)
        , (float)(gpuBwdTimeMapping) / (float)(osFreq / 1000)
        , (float)(gpuBwdTimeCpying) / (float)(osFreq / 1000)
    );
    //    printf("Info, OS-Window created.\n");
    NBScnRenderJobPlain_release(&cpuFwdRR);
    NBScnRenderJobPlain_release(&cpuBwdRR);
    NBScnRenderJobPlain_release(&gpuBwdRRMapped);
    NBScnRenderJobPlain_release(&gpuBwdRRCopied);
}

//app

void App_init(STApp* obj) {
    NBMemory_setZeroSt(*obj, STApp);
}

void App_release(STApp* obj) {
    //mtl
    {
        //cmds (queue)
        {
            if(obj->mtl.dev.cmds.obj != nil){
                [obj->mtl.dev.cmds.obj release];
                obj->mtl.dev.cmds.obj = nil;
            }
        }
        //pipeln
        {
            if(obj->mtl.dev.pipeln.obj != nil){
                [obj->mtl.dev.pipeln.obj release];
                obj->mtl.dev.pipeln.obj = nil;
            }
        }
        //lib
        {
            if(obj->mtl.dev.lib.func != nil){
                [obj->mtl.dev.lib.func release];
                obj->mtl.dev.lib.func = nil;
            }
            if(obj->mtl.dev.lib.def != nil){
                [obj->mtl.dev.lib.def release];
                obj->mtl.dev.lib.def = nil;
            }
        }
        //dev
        {
            if(obj->mtl.dev.obj != nil){
                [obj->mtl.dev.obj release];
                obj->mtl.dev.obj = nil;
            }
        }
    }
    //win
    {
        //
    }
}

