// research-DirectX.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//Direct Compute sample:
//  https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create
//
// https://github.com/walbourn/directx-sdk-samples/blob/main/BasicCompute11/BasicCompute11.cpp
// https://github.com/walbourn/directx-sdk-samples/blob/main/BasicCompute11/BasicCompute11.hlsl
//

#include "nb/NBFrameworkPch.h"

#define _WIN32_WINNT 0x600
#include <stdio.h>
//
#include <d3d11.h>
//
#include <windows.h>
#include <windowsx.h>
#include <assert.h>	//assert
//
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBThread.h"
#include "nb/scene/shaders/NBScnRenderJob_hlsl.h"
#include "nb/scene/shaders/NBScnRenderJob_cs_5_0.h"
#include "nb/research/research-scn-compute.h"
//
#pragma comment(lib,"d3d11.lib")

//#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

/*typedef struct CUSTOMVERTEX_ {
	FLOAT x, y, z, rhw;    // from the D3DFVF_XYZRHW flag
	DWORD color;    // from the D3DFVF_DIFFUSE flag
} CUSTOMVERTEX;*/

//

typedef struct STD3DCompute_ {
	//dev
	struct {
		ID3D11Device* obj;
		ID3D11DeviceContext* ctx;
		ID3D11InfoQueue* infoQueue;
	} dev;
	//shader
	struct {
		ID3D11ComputeShader* obj;
	} shader;
} STD3DCompute;

typedef struct STApp_ {
    //win
    struct {
        HWND		hWnd;	//window handle
        int         width;
        int         height;
        //curSec
        struct {
            int		msgsCountPaint;
        } curSec;
        //refresh
        struct {
            int		msAccum;
        } refresh;
    } win;
    //d3d
    struct {
		STD3DCompute compute;
    } d3d;
} STApp;

//app
void App_init(STApp* obj);
void App_release(STApp* obj);

//app::win
bool App_win_create(STApp* obj, const char* title, const int width, const int height);
void App_win_loop(STApp* obj);
//app::d3d
void App_d3d_compute_print_msgs_queue(STApp* app, const char* opHint);
bool App_d3d_compute_create(STApp* app);
bool App_d3d_compute_create_raw_buff(ID3D11Device* pDevice, UINT uSize, void* pInitData, ID3D11Buffer** ppBufOut);
bool App_d3d_compute_create_buff_shad_res_view(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut); //input buffer
bool App_d3d_compute_create_buff_unord_accs_view(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut); //output buffer
void App_d3d_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs);
//void App_d3d_renderFrame(STApp* app);

//

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
#	ifdef _DEBUG
	printf("_DEBUG is defined.\n");
#	else
	printf("_DEBUG is undefined.\n");
#	endif
#	ifdef NB_MEM_LEAK_DETECT_ENABLED
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtMemCheckpoint(&sOld); //take a snapshot
#	endif
	NBMngrProcess_init();
	NBMngrStructMaps_init();
	//
	{
		unsigned int dval = 0;
		unsigned int rrBase = 0;
		unsigned int rr = rrBase;
		unsigned char* rrb = (unsigned char*) &rr;
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
	HRESULT hr;
	STApp app;
	App_init(&app);
	if (!App_win_create(&app, "NBFramework - OpenGL ES 2.0 research", 800, 600)) {
		printf("App, error, App_win_create failed.\n");
	} else if (!App_d3d_compute_create(&app)) {
		printf("App, error, App_d3d_compute_create failed.\n");
	} else {
		const float maxDiffInSamples = 1.0f;
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
				App_d3d_compute_run_in_samples(&app, &tree, maxDiffInSamples);
			}
			NBScnRenderJobTree_release(&tree);
		}
		//generated samples
		{
			const int amm[] = { 100, 1000, 10000, 100000, 1000000 };
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
					App_d3d_compute_run_in_samples(&app, &tree, maxDiffInSamples);
				}
				NBScnRenderJobTree_release(&tree);
			}
		}
		//
		App_win_loop(&app);
	}
	App_release(&app);
	//sleep few seconds before exit
	Sleep(1000 * 3);
	//
	NBMngrStructMaps_release();
	NBMngrProcess_release();
#	ifdef NB_MEM_LEAK_DETECT_ENABLED
	_CrtMemCheckpoint(&sNew); //take a snapshot 
	if (_CrtMemDifference(&sDiff, &sOld, &sNew) || 1) {
		printf("-----------_CrtMemDumpStatistics ---------\n");
		_CrtMemDumpStatistics(&sDiff);
		printf("-----------_CrtMemDumpAllObjectsSince ---------\n");
		_CrtMemDumpAllObjectsSince(&sOld);
		printf("-----------_CrtDumpMemoryLeaks ---------\n");
		_CrtDumpMemoryLeaks();
	}
#	endif
}

bool App_d3d_compute_map_buffer_data(STApp* app, ID3D11Buffer* buff, ID3D11Buffer** dstBuffMapped, void** dstPtr, const int dstDataSz);

void App_d3d_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs) {
	HRESULT hr;
	const unsigned int spacesPerLvl = 4;
	NBTHREAD_CLOCK osFreq = NBThread_clocksPerSec();
	NBTHREAD_CLOCK cpuFwdTime = 0, cpuBwdTime = 0, cpuBwd2Time = 0, gpuBwdTimeExec = 0, gpuBwdTimeMapping = 0, gpuBwdTimeCpying = 0;
	STNBScnRenderJobFlat cpuFwdRR, cpuBwdRR, cpuBwdRR2, gpuBwdRRMapped, gpuBwdRRCopied;
	//
	STNBScnRenderJobLimits limits;
	NBMemory_setZeroSt(limits, STNBScnRenderJobLimits);
	limits.header.alignment = 16; //constant buffers must be multiple of 16
	limits.buffer.alignment = 128; //basedon Vulkan 'limits.nonCoherentAtomSize' Ryzen-5700G-iGPU value
	limits.dispatch.maxThreads = 65535; //defined in DX11 documentation
	//
	STNBScnRenderBuffRngs treeRngs, flatRngs;
	const UI32 treeBuffSz = NBScnRenderJobTree_getDispatchBufferRngs(src, &limits, 0, &treeRngs);
	const UI32 flatBuffSz = NBScnRenderJobFlat_getDispatchBufferRngs(src, &limits, 0, &flatRngs);
	BYTE* treeBuffDataTmp = (BYTE*)malloc(treeBuffSz);
	//copy tree data
	NBMemory_copy(&treeBuffDataTmp[treeRngs.nodes.offset], (void*)NBArray_dataPtr(&src->nodes, STNBScnTreeNode*), sizeof(STNBScnTreeNode) * src->nodes.use);
	NBMemory_copy(&treeBuffDataTmp[treeRngs.verts.v.offset], (void*)NBArray_dataPtr(&src->verts.v, STNBScnVertex*), sizeof(STNBScnVertex) * src->verts.v.use);
	NBMemory_copy(&treeBuffDataTmp[treeRngs.verts.v1.offset], (void*)NBArray_dataPtr(&src->verts.v1, STNBScnVertexTex*), sizeof(STNBScnVertexTex) * src->verts.v1.use);
	NBMemory_copy(&treeBuffDataTmp[treeRngs.verts.v2.offset], (void*)NBArray_dataPtr(&src->verts.v2, STNBScnVertexTex2*), sizeof(STNBScnVertexTex2) * src->verts.v2.use);
	NBMemory_copy(&treeBuffDataTmp[treeRngs.verts.v3.offset], (void*)NBArray_dataPtr(&src->verts.v3, STNBScnVertexTex3*), sizeof(STNBScnVertexTex3) * src->verts.v3.use);
	//
	NBScnRenderJobFlat_init(&cpuFwdRR);
	NBScnRenderJobFlat_init(&cpuBwdRR);
	NBScnRenderJobFlat_init(&cpuBwdRR2);	//gpu shader code running in cpu
	NBScnRenderJobFlat_init(&gpuBwdRRMapped);
	NBScnRenderJobFlat_init(&gpuBwdRRCopied);
	//
	NBScnRenderJobFlat_prepare(&cpuFwdRR, src, &limits, 0, NULL); //preallocate (cpu)
	NBScnRenderJobFlat_prepare(&cpuBwdRR, src, &limits, 0, NULL); //preallocate (cpu)
	NBScnRenderJobFlat_prepare(&cpuBwdRR2, src, &limits, 0, NULL); //preallocate (cpu)
	//NBScnRenderJobFlat_prepare(&gpuBwdRRMapped, src, &limits, 0, NULL); //do not preallocate mapped version (will point to gpu buffers)
	NBScnRenderJobFlat_prepare(&gpuBwdRRCopied, src, &limits, 0, NULL); //preallocate (gpu)
	//
	//Run in CPU
	{
		{
			NBTHREAD_CLOCK startTime, endTime;
			startTime = NBThread_clock();
			{
				NBScnRenderJobFlat_dispatchForwards(&cpuFwdRR, src, &limits);
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
				NBScnRenderJobFlat_dispatchBackwards(&cpuBwdRR, src, &limits, 0, src->nodes.use);
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
				STNBScnRenderDispatchHeader hdr;
				NBMemory_setZeroSt(hdr, STNBScnRenderDispatchHeader);
				hdr.iNodeOffset = 0;
				hdr.src = treeRngs;
				hdr.dst = flatRngs;
				research_scn_compute_convertTreeToPlainGpuAlgorithmToDst((const BYTE*)&hdr, (const BYTE*)treeBuffDataTmp, (BYTE*)cpuBwdRR2.buff.data, src->nodes.use);
			}
			endTime = NBThread_clock();
			cpuBwd2Time = endTime - startTime;
			//printf("RESULTS :: CPU :: backward execution:\n");
			//printf("---------------------->\n");
			//research_scn_compute_print_flat_job(&cpuBwdRR2, spacesPerLvl);
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
		ID3D11Buffer *treeBuff = NULL, *flatBuff = NULL, *fltBuff = NULL, *flatBuffMapped = NULL;
		ID3D11ShaderResourceView *treeBuffVw = NULL;
		ID3D11UnorderedAccessView *flatBuffVw = NULL;
		//allocate buffers
		{
			if (!App_d3d_compute_create_raw_buff(app->d3d.compute.dev.obj, treeBuffSz, (void*)treeBuffDataTmp, &treeBuff)) {
				printf("D3D, error, treeBuff allocation failed.\n");
			} else if (!App_d3d_compute_create_buff_shad_res_view(app->d3d.compute.dev.obj, treeBuff, &treeBuffVw)) {
				printf("D3D, error, treeBuffVw allocation failed.\n");
				//flatBuff
			} else if (!App_d3d_compute_create_raw_buff(app->d3d.compute.dev.obj, flatBuffSz, NULL, &flatBuff)) {
				printf("D3D, error, flatBuff allocation failed.\n");
			} else if (!App_d3d_compute_create_buff_unord_accs_view(app->d3d.compute.dev.obj, flatBuff, &flatBuffVw)) {
				printf("D3D, error, flatBuffVw allocation failed.\n");
			}
		}
		//
		if (treeBuffVw != NULL && flatBuffVw){
			NBTHREAD_CLOCK startTime, midTime, midTime2, endTime;
			midTime = midTime2 = endTime = startTime = NBThread_clock();
			App_d3d_compute_print_msgs_queue(app, "before-run");
			//Dispatch ComputeShader in groups
			{
				const UI32 headerPaddedSz	= NBScnRenderJobFlat_getDispatchHeaderPaddedSz(src, &limits);
				const UI32 ammDispatchCalls	= NBScnRenderJobFlat_getDispatcCallsNeeded(src, &limits);
				const UI32 headersBuffSz	= (headerPaddedSz * ammDispatchCalls);
				ID3D11Buffer* execBuff = NULL;
				STNBScnRenderDispatchHeader hdr;
				NBMemory_setZeroSt(hdr, STNBScnRenderDispatchHeader);
				hdr.iNodeOffset = 0;
				hdr.src = treeRngs;
				hdr.dst = flatRngs;
				//
				D3D11_BUFFER_DESC cbDesc;
				cbDesc.ByteWidth = headersBuffSz;
				cbDesc.Usage = D3D11_USAGE_DYNAMIC;
				cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbDesc.MiscFlags = 0;
				cbDesc.StructureByteStride = 0;
				// Fill in the subresource data.
				D3D11_SUBRESOURCE_DATA InitData;
				InitData.pSysMem = &hdr;
				InitData.SysMemPitch = 0;
				InitData.SysMemSlicePitch = 0;
				if ((hr = app->d3d.compute.dev.obj->CreateBuffer(&cbDesc, &InitData, &execBuff)) < 0) {
					printf("D3D, error, CreateBuffer failed.\n");
				} else {
					ID3D11ShaderResourceView* readVws[] = { treeBuffVw };
					ID3D11UnorderedAccessView* writeVws[] = { flatBuffVw };
					//set state
					{
						//shader
						app->d3d.compute.dev.ctx->CSSetShader(app->d3d.compute.shader.obj, nullptr, 0);
						App_d3d_compute_print_msgs_queue(app, "CSSetShader");
						//constants buffer
						app->d3d.compute.dev.ctx->CSSetConstantBuffers(0, 1, &execBuff);
						App_d3d_compute_print_msgs_queue(app, "CSSetConstantBuffers");
						//input buffers
						app->d3d.compute.dev.ctx->CSSetShaderResources(0, sizeof(readVws) / sizeof(readVws[0]), readVws);
						App_d3d_compute_print_msgs_queue(app, "CSSetShaderResources");
						//output buffer
						app->d3d.compute.dev.ctx->CSSetUnorderedAccessViews(0, sizeof(writeVws) / sizeof(writeVws[0]), writeVws, nullptr);
						App_d3d_compute_print_msgs_queue(app, "CSSetUnorderedAccessViews");
					}
					//Execute in max-groups
					{
						const unsigned int maxValPerDispatch = 65535;
						unsigned int execCount = 0;
						while (hdr.iNodeOffset < src->nodes.use) {
							execCount = src->nodes.use - hdr.iNodeOffset;
							if (execCount > maxValPerDispatch) {
								execCount = maxValPerDispatch;
							}
							//update index
							if(hdr.iNodeOffset > 0){
								D3D11_MAPPED_SUBRESOURCE MappedResource;
								app->d3d.compute.dev.ctx->Map(execBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
								{
									memcpy(MappedResource.pData, &hdr, sizeof(hdr));
								}
								app->d3d.compute.dev.ctx->Unmap(execBuff, 0);
							}
							//execute
							{
								app->d3d.compute.dev.ctx->Dispatch(execCount, 1, 1);
								App_d3d_compute_print_msgs_queue(app, "Dispatch");
							}
							//next
							hdr.iNodeOffset += execCount;
						}
						//flush messages
						App_d3d_compute_print_msgs_queue(app, "End-of-loop");
					}
					//unset state
					{
						ID3D11Buffer* buffNullptr = nullptr;
						//shader
						app->d3d.compute.dev.ctx->CSSetShader(nullptr, nullptr, 0);
						App_d3d_compute_print_msgs_queue(app, "CSSetShader(null)");
						//constants buffer
						app->d3d.compute.dev.ctx->CSSetConstantBuffers(0, 1, &buffNullptr);
						App_d3d_compute_print_msgs_queue(app, "CSSetConstantBuffers(null)");
						//input buffer
						memset(readVws, 0, sizeof(readVws));
						app->d3d.compute.dev.ctx->CSSetShaderResources(0, sizeof(readVws) / sizeof(readVws[0]), readVws);
						App_d3d_compute_print_msgs_queue(app, "CSSetShaderResources(null)");
						//output buffer
						memset(writeVws, 0, sizeof(writeVws));
						app->d3d.compute.dev.ctx->CSSetUnorderedAccessViews(0, sizeof(writeVws) / sizeof(writeVws[0]), writeVws, nullptr);
						App_d3d_compute_print_msgs_queue(app, "CSSetUnorderedAccessViews(null)");
					}
					//flush messages
					App_d3d_compute_print_msgs_queue(app, "End-of-execution");
				}
				if (execBuff != NULL) {
					execBuff->Release();
					execBuff = NULL;
				}
			}
			//copy results
			{
				midTime2 = endTime = midTime = NBThread_clock();
				gpuBwdTimeExec = midTime - startTime;
				//
				//printf("D3D, shader executed (%d units).\n", srcCount);
				//map buffers data
				void* flatBuffMappedPtr = NULL;
				if (!App_d3d_compute_map_buffer_data(app, flatBuff, &flatBuffMapped, &flatBuffMappedPtr, flatBuffSz)) {
					printf("D3D, error, could not copy result buffer: flatBuff(%d itms).\n", src->nodes.use);
				} else {
					STNBScnRenderBuffRngs rngs;
					const UI32 buffSz = NBScnRenderJobFlat_getDispatchBufferRngs(src, &limits, 0, &rngs);
					gpuBwdRRMapped.buff.data = (BYTE*)flatBuffMappedPtr;
					gpuBwdRRMapped.buff.sz = flatBuffSz;
					gpuBwdRRMapped.buff.use = flatBuffSz;
					NBScnRenderJobFlatMap_build(&gpuBwdRRMapped.map, gpuBwdRRMapped.buff.data, &rngs);
					//analyze results
					endTime = midTime2 = NBThread_clock();
					gpuBwdTimeMapping = midTime2 - midTime;
					//copy
					{
						NBMemory_copy(gpuBwdRRCopied.buff.data, flatBuffMappedPtr, flatBuffSz);
						endTime = NBThread_clock();
						gpuBwdTimeCpying = endTime - midTime2;
					}
					//compare
					{
						float matchRel = 0.0f;
						NBTHREAD_CLOCK startTime, endTime;
						endTime = startTime = NBThread_clock();
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
									printf("gpu_backwards_copied vs cpu_backwards: MATCH (%.4fms; %.2f%%).\n", (float)(endTime - startTime) / (float)(osFreq / 1000), 100.0f * matchRel);
								} else {
									printf("gpu_backwards_copied vs cpu_backwards: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
								}
							}
						} else {
							printf("gpu_backwards_maped vs cpu_backwards: DO NOT MATCH %.2f%% <---(!).\n", 100.0f * matchRel);
						}
					}
					//remove mapped data (memory is not owned by this object)
					{
						gpuBwdRRMapped.buff.data = NULL;
						gpuBwdRRMapped.buff.use = NULL;
						gpuBwdRRMapped.buff.sz = NULL;
						NBScnRenderJobFlatMap_reset(&gpuBwdRRMapped.map);
					}
				}
			}
		}
		//release
		if (flatBuffVw != NULL) { flatBuffVw->Release(); flatBuffVw = NULL; }
		if (treeBuffVw != NULL) { treeBuffVw->Release(); treeBuffVw = NULL; }
		if (flatBuffMapped != NULL) { app->d3d.compute.dev.ctx->Unmap(flatBuffMapped, 0); flatBuffMapped->Release(); flatBuffMapped = NULL; }
		if (flatBuff != NULL) { flatBuff->Release(); flatBuff = NULL; }
		if (treeBuff != NULL) { treeBuff->Release(); treeBuff = NULL; }
	}
	printf("Execution times (%d elems): cpu_fwd(%.4fms), cpu_bwd(%.4fms), gpu_bwd_on_cpu(%.4fms), gpu_bwd(%.4fms exc, +%.4fms map, +%.4fms cpy).\n"
		, src->nodes.use
		, (float)(cpuFwdTime) / (float)(osFreq / 1000)
		, (float)(cpuBwdTime) / (float)(osFreq / 1000)
		, (float)(cpuBwd2Time) / (float)(osFreq / 1000)
		, (float)(gpuBwdTimeExec) / (float)(osFreq / 1000)
		, (float)(gpuBwdTimeMapping) / (float)(osFreq / 1000)
		, (float)(gpuBwdTimeCpying) / (float)(osFreq / 1000)
	);
	//	printf("Info, OS-Window created.\n");
	NBScnRenderJobFlat_release(&cpuFwdRR);
	NBScnRenderJobFlat_release(&cpuBwdRR);
	NBScnRenderJobFlat_release(&gpuBwdRRMapped);
	NBScnRenderJobFlat_release(&gpuBwdRRCopied);
	if (treeBuffDataTmp != NULL) {
		free(treeBuffDataTmp);
		treeBuffDataTmp = NULL;
	}
}

bool App_d3d_compute_map_buffer_data(STApp* app, ID3D11Buffer* buff, ID3D11Buffer** dstBuffMapped, void** dstPtr, const int dstDataSz) {
	bool r = false;
	if (buff != NULL && dstBuffMapped != NULL && dstPtr != NULL && dstDataSz > 0) {
		ID3D11Buffer* buffTmp = NULL;
		HRESULT hr;
		D3D11_BUFFER_DESC desc = {};
		buff->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING; //A resource that supports data transfer (copy) from the GPU to the CPU
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		NBASSERT(desc.ByteWidth >= dstDataSz);
		//NBArray_dataPtr(&gpuBwdRR.nodes, STNBScnFlatNode)
		//(sizeof(STNBScnFlatNode) * gpuBwdRR.nodes.use)
		if ((hr = app->d3d.compute.dev.obj->CreateBuffer(&desc, nullptr, &buffTmp)) < 0) {
			printf("D3D, error, results-copy-buffer coudl not be created.\n");
		} else {
			app->d3d.compute.dev.ctx->CopyResource(buffTmp, buff);
			//printf("D3D, results copied to readable buffer.\n");
			D3D11_MAPPED_SUBRESOURCE mapRes;
			if ((hr = app->d3d.compute.dev.ctx->Map(buffTmp, 0, D3D11_MAP_READ, 0, &mapRes)) < 0) {
				printf("D3D, error, results buffer mapping failed.\n");
			} else {
				if (dstPtr != NULL) *dstPtr = mapRes.pData;
				*dstBuffMapped = buffTmp; buffTmp = NULL; //consume 
				r = true;
			}
		}
		//release (if not consumed)
		if (buffTmp != NULL) {
			buffTmp->Release();
			buffTmp = NULL;
		}
	}
	return r;
}

//app

void App_init(STApp* obj) {
	NBMemory_setZeroSt(*obj, STApp);
}

void App_release(STApp* obj) {
	//d3d
	{
		//compute
		{
			if (obj->d3d.compute.shader.obj != NULL) {
				obj->d3d.compute.shader.obj->Release();
				obj->d3d.compute.shader.obj = NULL;
			}
			if (obj->d3d.compute.dev.ctx != NULL) {
				obj->d3d.compute.dev.ctx->Release();
				obj->d3d.compute.dev.ctx = NULL;
			}
			if (obj->d3d.compute.dev.obj != NULL) {
				obj->d3d.compute.dev.obj->Release();
				obj->d3d.compute.dev.obj = NULL;
			}
			if (obj->d3d.compute.dev.infoQueue != NULL) {
				obj->d3d.compute.dev.infoQueue->Release();
				obj->d3d.compute.dev.infoQueue = NULL;
			}
		}
	}
	//win
	{
		if (obj->win.hWnd != NULL) {
			DestroyWindow(obj->win.hWnd);
			obj->win.hWnd = NULL;
		}
	}
}

//app:d3d

void App_d3d_compute_print_msgs_queue(STApp* app, const char* opHint) {
	if (app->d3d.compute.dev.infoQueue != NULL) {
		HRESULT hr;
		ID3D11InfoQueue* infoQueue = app->d3d.compute.dev.infoQueue;
		UINT64 message_count = infoQueue->GetNumStoredMessages();
		for (UINT64 i = 0; i < message_count; i++) {
			SIZE_T message_size = 0;
			infoQueue->GetMessage(i, nullptr, &message_size); //get the size of the message
			D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(message_size); //allocate enough space
			if (message != NULL) {
				if ((hr = infoQueue->GetMessage(i, message, &message_size)) < 0) {
					printf("Directx11: %s, error getting message.\n", opHint);
				} else {
					switch (message->Severity) {
					case D3D11_MESSAGE_SEVERITY_CORRUPTION:
						printf("D3D, corruption: %s, %.*s.\n", opHint, message->DescriptionByteLength, message->pDescription);
						break;
					case D3D11_MESSAGE_SEVERITY_ERROR:
						printf("D3D, error: %s, %.*s.\n", opHint, message->DescriptionByteLength, message->pDescription);
						break;
					case D3D11_MESSAGE_SEVERITY_WARNING:
						printf("D3D, warning: %s, %.*s.\n", opHint, message->DescriptionByteLength, message->pDescription);
						break;
					default:
						//D3D11_MESSAGE_SEVERITY_INFO
						//D3D11_MESSAGE_SEVERITY_MESSAGE
						break;
					}
				}
				free(message);
			}
		}
		infoQueue->ClearStoredMessages();
	}
}

//bool App_d3d_compute_isSupported(ID3D11Device* dev);

bool App_d3d_compute_create(STApp* app) {
	bool r = false;
	HRESULT hr;
	///DirectX10 cs_4_0 only supports one UAV buffer slot.
	const D3D_FEATURE_LEVEL lvl[] = { /*D3D_FEATURE_LEVEL_11_1,*/ D3D_FEATURE_LEVEL_11_0 };
	UINT createDeviceFlags = 0;
#	ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#	endif
	ID3D11Device* dev = nullptr;
	ID3D11DeviceContext* ctx = nullptr;
	D3D_FEATURE_LEVEL featLvl;
	if ((hr = 
		D3D11CreateDevice(
			nullptr
			, D3D_DRIVER_TYPE_HARDWARE
			, nullptr
			, createDeviceFlags
			, lvl
			, sizeof(lvl) / sizeof(lvl[0])
			, D3D11_SDK_VERSION
			, &dev
			, &featLvl
			, &ctx)
		) < 0) 
	{
		printf("D3D, error, D3D11CreateDevice failed with %08X.\n", hr);
	} else {
		printf("D3D, device created.\n");
		if (featLvl < D3D_FEATURE_LEVEL_11_0) {
			//Note: DirectX10 cs_4_0 only supports one UAV buffer slot.
			printf("D3D, error, DirectCompute is not supported by this pre-DX11-device.\n");
		} else {
			printf("D3D, DirectCompute is supported.\n");
				ID3D11ComputeShader* cs = NULL;
				if ((hr = dev->CreateComputeShader(NBScnRenderJob_cs_5_0, sizeof(NBScnRenderJob_cs_5_0) / sizeof(NBScnRenderJob_cs_5_0[0]), nullptr, &cs)) < 0) {
					printf("D3D, error, CreateComputeShader failed.\n");
				} else {
					ID3D11InfoQueue* infoQueue;
					printf("D3D, CreateComputeShader success.\n");
					if ((hr = dev->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue)) < 0){
						printf("D3D, error, QueryInterface(c) failed.\n");
					} else {
						printf("D3D, ID3D11InfoQueue gotten.\n");
						//Unfiltering info-queue
						infoQueue->PushEmptyStorageFilter();
						//consume
						if (app->d3d.compute.shader.obj != NULL) {
							app->d3d.compute.shader.obj->Release();
							app->d3d.compute.shader.obj = NULL;
						}
						if (app->d3d.compute.dev.infoQueue != NULL) {
							app->d3d.compute.dev.infoQueue->Release();
							app->d3d.compute.dev.infoQueue = NULL;
						}
						if (app->d3d.compute.dev.ctx != NULL) {
							app->d3d.compute.dev.ctx->Release();
							app->d3d.compute.dev.ctx = NULL;
						}
						if (app->d3d.compute.dev.obj != NULL) {
							app->d3d.compute.dev.obj->Release();
							app->d3d.compute.dev.obj = NULL;
						}
						app->d3d.compute.dev.obj = dev; dev = NULL;
						app->d3d.compute.dev.ctx = ctx; ctx = NULL;
						app->d3d.compute.dev.infoQueue = infoQueue; infoQueue = NULL;
						app->d3d.compute.shader.obj = cs; cs = NULL;
						r = true;
					}
					//release (if not consumed)
					if (infoQueue != NULL) {
						infoQueue->Release();
						infoQueue = NULL;
					}
					//release (if not consumed)
					if (cs != NULL) {
						cs->Release();
						cs = NULL;
					}
				}
		}
		//release(if not conusmed)
		if (ctx != NULL) {
			ctx->Release();
			ctx = NULL;
		}
		if (dev != NULL) {
			dev->Release();
			dev = NULL;
		}
	}
	return r;
}

/*bool App_d3d_compute_isSupported(ID3D11Device* dev) {
	bool r = false;
	const D3D_FEATURE_LEVEL feaLvl = dev->GetFeatureLevel();
	if (feaLvl >= D3D_FEATURE_LEVEL_11_0) {
		r = true;
	} else {
		//Device is < DX11
		//Note: DirectX10 cs_4_0 only supports one UAV buffer slot.
		D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts = { 0 };
		(void)dev->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
		if (hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x) {
			r = true;
		}
	}
	return r;
}*/

bool App_d3d_compute_create_raw_buff(ID3D11Device* pDevice, UINT uSize, void* pInitData, ID3D11Buffer** ppBufOut) {
	bool r = false;
	HRESULT hr;
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER | D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = uSize;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	//
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pInitData;
	if ((hr = pDevice->CreateBuffer(&desc, (pInitData ? &InitData : nullptr), ppBufOut)) < 0) {
		printf("D3D, error, CreateBuffer failed.\n");
	} else {
		r = true;
	}
	return r;
}

bool App_d3d_compute_create_buff_shad_res_view(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut) { //input buffer
	bool r = false;
	HRESULT hr;
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;
	//
	if (!(descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)) {
		printf("D3D, error, buffer does not support RAW views.\n");
	} else {
		// This is a Raw Buffer
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
		if ((hr = pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut)) < 0) {
			printf("D3D, error, CreateShaderResourceView failed.\n");
		} else {
			r = true;
		}
	}
	return r;
}

bool App_d3d_compute_create_buff_unord_accs_view(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut) { //output buffer
	bool r = false;
	HRESULT hr;
	//
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);
	//
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (!(descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)) {
		printf("D3D, error, buffer does not support RAW views.\n");
	} else {
		// This is a Raw Buffer
		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
		if ((hr = pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut)) < 0) {
			printf("D3D, error, CreateUnorderedAccessView failed.\n");
		} else {
			r = true;
		}
	}
	return r;
}

/*bool App_d3d_create(STApp* app, HWND hWnd) {
	bool r = false;
	HRESULT hr;
	LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
	if (d3d == NULL) {
		printf("D3D, error, Direct3DCreate9 failed.\n");
	} else {
		printf("D3D, instance created.\n");
		LPDIRECT3DDEVICE9 dev = NULL;
		D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information
		ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
		d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
		d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
		// create a device class using this information and information from the d3dpp stuct
		if ((hr = d3d->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&dev)
			) < 0)
		{
			printf("D3D, error, CreateDevice failed.\n");
		} else {
			printf("D3D, device created.\n");
			//
			D3DCAPS9 caps;
			if ((hr = dev->GetDeviceCaps(&caps)) < 0) {
				printf("D3D, error, GetDeviceCaps failed.\n");
			} else {
				printf("D3D, device capabilities read.\n");
				VOID* vboPtr = NULL;
				LPDIRECT3DVERTEXBUFFER9 vbo = NULL;
				CUSTOMVERTEX verts[] = {
					{320.0f, 50.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 0, 255),},
					{520.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),},
					{120.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(255, 0, 0),},
				};
				if ((hr = dev->CreateVertexBuffer(3 * sizeof(verts[0]),
					0,
					CUSTOMFVF,
					D3DPOOL_MANAGED,
					&vbo,
					NULL)) < 0)
				{
					printf("D3D, error, CreateVertexBuffer failed.\n");
				} else {
					printf("D3D, VertexBuffer created.\n");
					if ((hr = vbo->Lock(0, 0, (void**)&vboPtr, 0)) < 0) {
						printf("D3D, error, vbo->Lock failed.\n");
					} else {
						printf("D3D, VertexBuffer populated (while locked).\n");
						memcpy(vboPtr, verts, sizeof(verts));
						vbo->Unlock();
						//results
						app->scene.vbo = vbo; vbo = NULL; //consume
						//
						app->d3d.dev.obj = dev; dev = NULL; //consume
						app->d3d.dev.caps = caps; //consume
						//
						app->d3d.obj = d3d; d3d = NULL; //cosume
						//
						r = true;
					}
					//release (if not consumed)
					if (vbo != NULL) {
						vbo->Release();
						vbo = NULL;
					}
				}
			}
			//release (if not consumed)
			if (dev != NULL) {
				dev->Release();
				dev = NULL;
			}
		}
		//release (if not consumed)
		if (d3d != NULL) {
			d3d->Release();
			d3d = NULL;
		}
	}
	return r;
}*/

/*void App_d3d_renderFrame(STApp* app) {
	if (app->d3d.dev.obj != NULL) {
		// clear the window to a deep blue
		app->d3d.dev.obj->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
		app->d3d.dev.obj->BeginScene();    // begins the 3D scene
		{
			app->d3d.dev.obj->SetFVF(CUSTOMFVF);
			app->d3d.dev.obj->SetStreamSource(0, app->scene.vbo, 0, sizeof(CUSTOMVERTEX));
			app->d3d.dev.obj->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
		}
		app->d3d.dev.obj->EndScene();    // ends the 3D scene
		app->d3d.dev.obj->Present(NULL, NULL, NULL, NULL);    // displays the created frame
	}
}*/

//app::win

LRESULT CALLBACK App_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool App_win_create(STApp* obj, const char* title, const int width, const int height) {
	bool r = false;
	WNDCLASS wndclass = { 0 };
	DWORD    wStyle = 0;
	RECT     windowRect;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL) {
		printf("Error, GetModuleHandle failed.\n");
	} else {
		//
		wndclass.style = CS_OWNDC;
		wndclass.lpfnWndProc = (WNDPROC)App_WindowProc;
		wndclass.hInstance = hInstance;
		wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndclass.lpszClassName = "Direct3D";
		//
		if (!RegisterClass(&wndclass)) {
			printf("WIN, Error, registering OS-Window's class.\n");
		} else {
			HWND hWnd = NULL;
			wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;
			// Adjust the window rectangle so that the client area has
			// the correct number of pixels
			windowRect.left = 0;
			windowRect.top = 0;
			windowRect.right = width;
			windowRect.bottom = height;
			//
			if (!AdjustWindowRect(&windowRect, wStyle, FALSE)) {
				printf("WIN, Error, AdjustWindowRect failed.\n");
			} else {
				//
				hWnd = CreateWindow(
					"Direct3D",
					title,
					wStyle,
					0,
					0,
					windowRect.right - windowRect.left,
					windowRect.bottom - windowRect.top,
					NULL,
					NULL,
					hInstance,
					NULL);
				if (hWnd == NULL) {
					printf("WIN, Error, creating OS-Window.\n");
				} else {
					printf("WIN, OS-Window created.\n");
					// Set the ESContext* to the GWLP_USERDATA so that it is available to the 
					SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)obj);
					//Show
					ShowWindow(hWnd, TRUE);
					//win
					if (obj->win.hWnd != NULL) {
						DestroyWindow(obj->win.hWnd);
						obj->win.hWnd = NULL;
					}
					obj->win.width = width;
					obj->win.height = height;
					obj->win.hWnd = hWnd; hWnd = NULL; //consume
					//
					r = true;
					//release (if not consumed)
					if (hWnd != NULL) {
						DestroyWindow(hWnd);
						hWnd = NULL;
					}
				}
			}
		}
	}
	return r;
}

void App_win_loop(STApp* obj) {
	MSG msg = { 0 };
	const int targetFPS = 60;
	ULONGLONG curSecAccumMs = 0, curSecAccumMsgs = 0;
	ULONGLONG lastTime = GetTickCount64(), curTime, msPassed;
	while (1) {
		curTime = GetTickCount64(); //ms
		msPassed = curTime - lastTime;
		curSecAccumMs += msPassed;
		obj->win.refresh.msAccum += (int)msPassed;
		//
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			curSecAccumMsgs++;
			if (msg.message == WM_QUIT) {
				break;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		// Call update function if registered
		/*if (esContext->updateFunc != NULL){
			esContext->updateFunc(esContext, deltaTime);
		}*/
		if (obj->win.refresh.msAccum >= (1000 / targetFPS)) {
			obj->win.refresh.msAccum = 0;
			SendMessage(obj->win.hWnd, WM_PAINT, 0, 0);
		}
		if (curSecAccumMs >= 1000) {
			if (curSecAccumMsgs > 0 && obj->win.curSec.msgsCountPaint > 0) {
				printf("WIN, %d frames, %d messages processed.\n", (int)obj->win.curSec.msgsCountPaint, (int)curSecAccumMsgs);
			} else if (obj->win.curSec.msgsCountPaint > 0) {
				printf("WIN, %d frames processed.\n", (int)obj->win.curSec.msgsCountPaint);
			} else if (curSecAccumMsgs > 0) {
				printf("WIN, %d messages processed.\n", (int)curSecAccumMsgs);
			}
			obj->win.curSec.msgsCountPaint = 0;
			curSecAccumMsgs = 0;
			curSecAccumMs %= 1000;
		}
		//keep track of time
		lastTime = curTime;
	}
}

LRESULT CALLBACK App_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT  lRet = 1;
	switch (uMsg) {
		case WM_CREATE:
			break;
		case WM_PAINT:
		{
			STApp* app = (STApp*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (app) {
				app->win.curSec.msgsCountPaint++;
				//App_d3d_renderFrame(app);
				ValidateRect(app->win.hWnd, NULL); //flags the rect as painted (valid)
			}
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		break;
		case WM_CHAR:
		{
			/*POINT      point;
			STESContext* esCtxt = (STESContext*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			GetCursorPos(&point);
			if (esCtxt && esCtxt->keyFunc)
				esCtxt->keyFunc(esCtxt, (unsigned char)wParam,
					(int)point.x, (int)point.y);
			*/
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		break;
		default:
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
	}
	return lRet;
}


