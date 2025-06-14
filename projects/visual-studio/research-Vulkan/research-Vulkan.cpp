// research-Vulkan.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "nb/NBFrameworkPch.h"

#define _WIN32_WINNT 0x600
#include <stdio.h>
//
#include <windows.h>
#include <windowsx.h>
#include <assert.h>	//assert
//Vulkan (after windows.h)
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h> 
//
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBThread.h"
#include "nb/scene/NBScnRenderJob_vulkan1_0_430.h"
#include "nb/research/research-scn-compute.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData){
	printf("Vulkan Validation layer: %s\n", callbackData->pMessage);
	return VK_FALSE;
}

//----------------
//-- STNBVulkanBuff
//----------------

typedef struct STNBVulkanBuff_ {
	VkDevice		dev;
	VkBuffer		buff;
	VkDeviceMemory	mem;
	UI32			memFlags;		//
	void*			mapped;			//"Memory in Vulkan doesn't need to be unmapped before using it on GPU..."
	UI32			mappedDataSz;	//user data
	UI32			mappedSz;		//nonCoherentAtomSize-compliant size
	UI32			alignment;		//requirement for vkBindbufferMemory() and vkBindImageMemory()
	UI32			nonCoherentAtomSize;	//flush/invalidate alignment
} STNBVulkanBuff;

void NBVulkanBuff_init(STNBVulkanBuff* obj);
void NBVulkanBuff_release(STNBVulkanBuff* obj);

//----------------
//-- App
//----------------

typedef struct STVlknCompute_ {
	int dummy;
	//vulkan
	/*struct {
		VkInstance inst;
	} vulkan;*/
	//shader
	struct {
		VkShaderModule mdl;
	} shader;
} STVlknCompute;

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
	//vlkn
	struct {
		//inst
		struct {
			VkInstance obj;
			VkDebugUtilsMessengerEXT vkDbgMsgr;
		} inst;
		//dev
		struct {
			VkDevice obj;
			//hw
			struct {
				VkPhysicalDeviceProperties props;
				//mem
				struct {
					VkPhysicalDeviceMemoryProperties props;
				} mem;
			} hw; //Physiscal device
			VkQueue  queue;
		} dev;
		//queueFam
		struct {
			int graphAndCompIdx;
		} queueFam;
		STVlknCompute compute;
	} vlkn;
} STApp;

//app
void App_init(STApp* obj);
void App_release(STApp* obj);
//app::win
bool App_win_create(STApp* obj, const char* title, const int width, const int height);
void App_win_loop(STApp* obj);
//
bool App_vulkan_compute_create(STApp* app);

bool App_vulkan_compute_create_src_buffer(STApp* app, const unsigned int sz, void* data, STNBVulkanBuff* dst, const bool printBuffTypeSelected);
bool App_vulkan_compute_create_dst_buffer(STApp* app, const unsigned int sz, STNBVulkanBuff* dst, const bool printBuffTypeSelected);
bool App_vulkan_compute_create_uniform_buffer(STApp* app, const unsigned int sz, STNBVulkanBuff* dst, const bool printBuffTypeSelected);
void App_vulkan_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs);

int main(){
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
	HRESULT hr;
	STApp app;
	App_init(&app);
	if (!App_win_create(&app, "NBFramework - OpenGL ES 2.0 research", 800, 600)) {
		printf("App, error, App_win_create failed.\n");
	} else if (!App_vulkan_compute_create(&app)) {
		printf("App, error, App_d3d_compute_create failed.\n");
	} else {
		const float maxDiffInSamples = 0.1f;
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
				App_vulkan_compute_run_in_samples(&app, &tree, maxDiffInSamples);
			}
			NBScnRenderJobTree_release(&tree);
		}
		//generated samples
		{
			const int amm[] = { 10 , 100, 1000, 10000, 100000, 100000, 1000000 };
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
					App_vulkan_compute_run_in_samples(&app, &tree, maxDiffInSamples);
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

//----------------
//-- STNBVulkanBuff
//----------------

void NBVulkanBuff_init(STNBVulkanBuff* obj) {
	NBMemory_setZeroSt(*obj, STNBVulkanBuff);
}

void NBVulkanBuff_release(STNBVulkanBuff* obj) {
	if (obj->dev != NULL) {
		if (obj->buff != NULL) {
			vkDestroyBuffer(obj->dev, obj->buff, nullptr);
			obj->buff = NULL;
		}
		if (obj->mem != NULL) {
			if (obj->mapped != NULL) {
				vkUnmapMemory(obj->dev, obj->mem);
				obj->mapped = NULL;
			}
			vkFreeMemory(obj->dev, obj->mem, NULL);
			obj->mem = NULL;
		}
		obj->dev = NULL;
	}
	obj->memFlags = 0;
}

//app

void App_init(STApp* obj) {
	NBMemory_setZeroSt(*obj, STApp);
}

void App_release(STApp* obj) {
	//vlkn
	{
		//compute
		{
			if (obj->vlkn.compute.shader.mdl != NULL) {
				vkDestroyShaderModule(obj->vlkn.dev.obj, obj->vlkn.compute.shader.mdl, NULL);
				obj->vlkn.compute.shader.mdl = NULL;
			}
		}
		//dev
		{
			if (obj->vlkn.dev.obj != NULL) {
				vkDestroyDevice(obj->vlkn.dev.obj, NULL);
				obj->vlkn.dev.obj = NULL;
			}
			if (obj->vlkn.dev.queue != NULL) {
				//
				obj->vlkn.dev.queue = NULL;
			}
		}
		//queueFam
		{
			obj->vlkn.queueFam.graphAndCompIdx = -1;
		}
		//inst (instance)
		{
			if (obj->vlkn.inst.vkDbgMsgr != NULL) {
				if (obj->vlkn.inst.obj != NULL) {
					PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(obj->vlkn.inst.obj, "vkDestroyDebugUtilsMessengerEXT");
					if (vkDestroyDebugUtilsMessengerEXT == NULL) {
						printf("Vulkan, error, vkGetInstanceProcAddr(vkDestroyDebugUtilsMessengerEXT) failed.\n");
					} else {
						vkDestroyDebugUtilsMessengerEXT(obj->vlkn.inst.obj, obj->vlkn.inst.vkDbgMsgr, NULL);
					}
				}
				obj->vlkn.inst.vkDbgMsgr = NULL;
			}
			if (obj->vlkn.inst.obj != NULL) {
				vkDestroyInstance(obj->vlkn.inst.obj, NULL);
				obj->vlkn.inst.obj = NULL;
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

//compute

bool App_vulkan_compute_create(STApp* app) {
	bool r = false;
	unsigned int vkPropsCount = 0;
	if (VK_SUCCESS != vkEnumerateInstanceLayerProperties(&vkPropsCount, NULL)) {
		printf("Vulkan, error, unexpected return by vkEnumerateInstanceLayerProperties.\n");
	} else if (vkPropsCount <= 0) {
		printf("Vulkan, error, zero vkEnumerateInstanceLayerProperties returned.\n");
	} else {
		VkLayerProperties* vkProps = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * vkPropsCount);
		if (VK_SUCCESS != vkEnumerateInstanceLayerProperties(&vkPropsCount, vkProps)) {
			printf("Vulkan, error, unexpected return by vkEnumerateInstanceLayerProperties.\n");
		} else if (vkPropsCount <= 0) {
			printf("Vulkan, error, zero vkEnumerateInstanceLayerProperties returned.\n");
		} else {
			bool isDbgUtilPresent = false;
			const char* vkLyrsEnbld[32] = { NULL }; int vkLyrsEnbldUse = 0;
			const char* vkExts[32] = { NULL }; int vkExtsUse = 0;
			{
				int i; for (i = 0; i < vkPropsCount; i++) {
					const VkLayerProperties* vProp = &vkProps[i];
					if (0 == strcmp(vProp->layerName, "VK_LAYER_KHRONOS_validation")) {
						vkLyrsEnbld[vkLyrsEnbldUse++] = vProp->layerName;
						vkExts[vkExtsUse++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME; // "VK_EXT_debug_utils"
						isDbgUtilPresent = true;
					}
					//printf("Vulkan, prop-#%d/%d: '%s' (v%u imp).\n", i + 1, vkPropsCount, vProp->layerName, vProp->implementationVersion);
				}
			}
			{
				int i;
				for (i = 0; i < vkLyrsEnbldUse; i++) {
					const char* lyr = vkLyrsEnbld[i];
					printf("Vulkan, enabled-lyr-#%d/%d: '%s'.\n", i + 1, vkLyrsEnbldUse, lyr);
				}
				//VK_KHR_SURFACE_EXTENSION_NAME, // "VK_KHR_surface"
				//VK_KHR_WIN32_SURFACE_EXTENSION_NAME, // "VK_KHR_win32_surface"
				for (i = 0; i < vkExtsUse; i++) {
					const char* lyr = vkExts[i];
					printf("Vulkan, enabled-extension-#%d/%d: '%s'.\n", i + 1, vkLyrsEnbldUse, lyr);
				}
			}
			//create application
			{
				VkInstance vkInst = NULL;
				VkDebugUtilsMessengerEXT vkDbgMsgr = NULL;
				VkApplicationInfo appInfo =
				{
					VK_STRUCTURE_TYPE_APPLICATION_INFO,
					NULL,
					"Vulkan research",
					1, // application Version
					"Vulkan research",
					1, // engine Version
					VK_API_VERSION_1_0
				};
				VkInstanceCreateInfo createInfo =
				{
					VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					NULL,
					0, // flags (this is the only time I'm commenting on this)
					&appInfo,
					vkLyrsEnbldUse, // layer count
					vkLyrsEnbld, // layers to enable
					vkExtsUse, // extension count
					vkExts // extension names
				};
				if (vkCreateInstance(&createInfo, NULL, &vkInst) != VK_SUCCESS){
					printf("Vulkan, error, vkCreateInstance failed with %d lyrs and %d extensions.\n", vkLyrsEnbldUse, vkExtsUse);
				} else {
					printf("Vulkan, vkCreateInstance success.\n");
					unsigned int vkHwDevsCount = 0;
					VkPhysicalDevice* vkHwDevs = NULL;
					if (isDbgUtilPresent) {
						VkDebugUtilsMessageSeverityFlagsEXT messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
						VkDebugUtilsMessageTypeFlagsEXT messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
						VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
						{
							VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
							NULL,
							0,
							messageSeverity,
							messageType,
							vulkan_debug_callback,
							NULL // user data
						};

						// Load the debug utils extension function
						PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkCreateDebugUtilsMessengerEXT");
						if (vkCreateDebugUtilsMessengerEXT == NULL) {
							printf("Vulkan, error, vkGetInstanceProcAddr(vkCreateDebugUtilsMessengerEXT) failed.\n");
						} else {
							if (vkCreateDebugUtilsMessengerEXT(vkInst, &debugCreateInfo, NULL, &vkDbgMsgr) != VK_SUCCESS) {
								printf("Vulkan, error, vkCreateDebugUtilsMessengerEXT failed.\n");
							} else {
								printf("Vulkan, vkCreateDebugUtilsMessengerEXT success.\n");
							}
						}
					}
					//
					if (VK_SUCCESS != vkEnumeratePhysicalDevices(vkInst, &vkHwDevsCount, NULL)) {
						printf("Vulkan, error, unexpected return by vkEnumerateInstanceLayerProperties.\n");
					} else if (vkHwDevsCount <= 0) {
						printf("Vulkan, error, zero vkEnumeratePhysicalDevices returned.\n");
					} else {
						vkHwDevs = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * vkHwDevsCount);
						if (vkHwDevs != NULL) {
							if (VK_SUCCESS != vkEnumeratePhysicalDevices(vkInst, &vkHwDevsCount, vkHwDevs)) {
								printf("Vulkan, error, unexpected return by vkEnumerateInstanceLayerProperties.\n");
							} else if (vkHwDevsCount <= 0) {
								printf("Vulkan, error, zero vkEnumeratePhysicalDevices returned.\n");
							} else {
								VkPhysicalDevice vkHwDev = NULL;
								VkPhysicalDeviceProperties vkHwDevProps;
								VkPhysicalDeviceMemoryProperties vkHwDevMemProps;
								printf("Vulkan, %d physical devices:\n", vkHwDevsCount);
								if (vkHwDevsCount == 0) {
									printf("        NONE.\n");
								}
								int i; for (i = 0; i < vkHwDevsCount; i++) {
									VkPhysicalDeviceProperties p;
									VkPhysicalDeviceMemoryProperties mp;
									vkGetPhysicalDeviceProperties(vkHwDevs[i], &p);
									vkGetPhysicalDeviceMemoryProperties(vkHwDevs[i], &mp);
									// If the device is a dedicated (discrete) GPU, prefer it
									printf("        device-#%d/%d, %s: '%s' (API %u, driver %d).\n", i + 1, vkHwDevsCount, p.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER ? "OTHER" : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "INTEGRATED_GPU" : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "DISCRETE_GPU" : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? "VIRTUAL_GPU" : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? "CPU" : "TYPE_UNEXPECTED", p.deviceName, p.apiVersion, p.driverVersion);
									if (mp.memoryTypeCount > 0) {
										int i; for (i = 0; i < mp.memoryTypeCount; i++) {
											const VkMemoryType* mt = &mp.memoryTypes[i];
											printf("             memory-type-#%d/%d: heap#%d %s%s%s%s%s%s%s%s%s.\n", i + 1, mp.memoryTypeCount, (mt->heapIndex + 1)
												, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ? " | device_local" : ""
												, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? " | host_visible" : ""
												, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? " | host_coherent" : ""	//vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to manage this memory.
												, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT ? " | host_cached" : ""		//memory is cached on the host for faster access
												, mt->propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ? " | lazily_alloc" : ""	//only accesible to device (gpu)
												//VK_VERSION_1_1
												, mt->propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT ? " | protected" : ""			//only device's protected queue operations to access the memory.
												//VK_AMD_device_coherent_memory
												, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ? " | device_coherent_amd" : ""	//device's access and visibility of this memory is automatic
												, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ? " | device_uncached_amd" : ""	//uncached device memory is always device coherent
												//VK_NV_external_memory_rdma
												, mt->propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ? " | rdma_capable_nv" : ""	//external devices can access this memory directly
											);
										}
									}
									if (mp.memoryHeapCount > 0) {
										int i; for (i = 0; i < mp.memoryHeapCount; i++) {
											const VkMemoryHeap* mh = &mp.memoryHeaps[i];
											if (mh->size >= (1024 * 1024 * 1024)) {
												printf("             memory-heap-#%d/%d: %.2fGBs flgs(%d) %s%s%s%s.\n", i + 1, mp.memoryTypeCount, (float)((double)mh->size / (1024.0 * 1024.0 * 1024.0)), mh->flags
													, mh->flags& VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? " | device_local" : ""
													//VK_VERSION_1_1
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ? " | multi_instance" : "" //in a logical device that groups multiple phisical devices, the allocation is replicated to all devices.
													//VK_QCOM_tile_memory_heap
													, mh->flags& VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM ? " | tile_memory_qcom" : ""
													//VK_KHR_device_group_creation
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR ? " | multi_instance_khr" : ""
												);
											} else if (mh->size >= (1024 * 1024)) {
												printf("             memory-heap-#%d/%d: %.2fMBs flgs(%d) %s%s%s%s.\n", i + 1, mp.memoryTypeCount, (float)((double)mh->size / (1024.0 * 1024.0)), mh->flags
													, mh->flags& VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? " | device_local" : ""
													//VK_VERSION_1_1
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ? " | multi_instance" : "" //in a logical device that groups multiple phisical devices, the allocation is replicated to all devices.
													//VK_QCOM_tile_memory_heap
													, mh->flags& VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM ? " | tile_memory_qcom" : ""
													//VK_KHR_device_group_creation
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR ? " | multi_instance_khr" : ""
												);
											} else if (mh->size >= 1024) {
												printf("             memory-heap-#%d/%d: %.2fKBs flgs(%d) %s%s%s%s.\n", i + 1, mp.memoryTypeCount, (float)((double)mh->size / 1024.0), mh->flags
													, mh->flags& VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? " | device_local" : ""
													//VK_VERSION_1_1
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ? " | multi_instance" : "" //in a logical device that groups multiple phisical devices, the allocation is replicated to all devices.
													//VK_QCOM_tile_memory_heap
													, mh->flags& VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM ? " | tile_memory_qcom" : ""
													//VK_KHR_device_group_creation
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR ? " | multi_instance_khr" : ""
												);
											} else {
												printf("             memory-heap-#%d/%d: %u bytes flgs(%d) %s%s%s%s.\n", i + 1, mp.memoryTypeCount, mh->size, mh->flags
													, mh->flags& VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? " | device_local" : ""
													//VK_VERSION_1_1
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ? " | multi_instance" : "" //in a logical device that groups multiple phisical devices, the allocation is replicated to all devices.
													//VK_QCOM_tile_memory_heap
													, mh->flags& VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM ? " | tile_memory_qcom" : ""
													//VK_KHR_device_group_creation
													, mh->flags& VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR ? " | multi_instance_khr" : ""
												);
											}
										}
									}
									switch (p.deviceType) {
										case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
											if (vkHwDev == NULL || vkHwDevProps.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
												//always prefer discrete GPU
												vkHwDev = vkHwDevs[i];
												vkHwDevProps = p;
												vkHwDevMemProps = mp;
											}
											break;
										case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
											if (vkHwDev == NULL || vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU || vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU || vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER) {
												vkHwDev = vkHwDevs[i];
												vkHwDevProps = p;
												vkHwDevMemProps = mp;
											}
											break;
										case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
											if (vkHwDev == NULL || vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU || vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER) {
												vkHwDev = vkHwDevs[i];
												vkHwDevProps = p;
												vkHwDevMemProps = mp;
											}
											break;
										case VK_PHYSICAL_DEVICE_TYPE_CPU:
											if (vkHwDev == NULL) {
												vkHwDev = vkHwDevs[i];
												vkHwDevProps = p;
												vkHwDevMemProps = mp;
											}
											break;
									}
								}
								//
								if (vkHwDev == NULL) {
									printf("Vulkan, error, no suitable device found.\n");
								} else {
									printf("Vulkan, prefered device, %s: '%s' (API %u, driver %d).\n", vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER ? "OTHER" : vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "INTEGRATED_GPU" : vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "DISCRETE_GPU" : vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? "VIRTUAL_GPU" : vkHwDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? "CPU" : "TYPE_UNEXPECTED", vkHwDevProps.deviceName, vkHwDevProps.apiVersion, vkHwDevProps.driverVersion);
									printf("        flush/invalidate alignment: %d bytes (limits.nonCoherentAtomSize).\n", (int)vkHwDevProps.limits.nonCoherentAtomSize);
									printf("            uniform-buff alignment: %d bytes (limits.minUniformBufferOffsetAlignment).\n", (int)vkHwDevProps.limits.minUniformBufferOffsetAlignment);
									printf("                max-dispatch sizes: (%d, %d, %d).\n", (int)vkHwDevProps.limits.maxComputeWorkGroupCount[0], (int)vkHwDevProps.limits.maxComputeWorkGroupCount[1], (int)vkHwDevProps.limits.maxComputeWorkGroupCount[2]);
									unsigned int vkQFamsCount = 0;
									VkQueueFamilyProperties* vkQFams = NULL;
									vkGetPhysicalDeviceQueueFamilyProperties(vkHwDev, &vkQFamsCount, NULL);
									if (vkQFamsCount <= 0) {
										printf("Vulkan, error, zero vkGetPhysicalDeviceQueueFamilyProperties returned.\n");
									} else {
										vkQFams = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * vkQFamsCount);
										if (vkQFams != NULL) {
											vkGetPhysicalDeviceQueueFamilyProperties(vkHwDev, &vkQFamsCount, vkQFams);
											if (vkQFamsCount <= 0) {
												printf("Vulkan, error, zero vkGetPhysicalDeviceQueueFamilyProperties returned.\n");
											} else {
												int vkGraphAndComputeQueueFamIdx = -1;
												int i; for (i = 0; i < vkHwDevsCount; i++) {
													VkQueueFamilyProperties p = vkQFams[i];
													if (p.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
														vkGraphAndComputeQueueFamIdx = i;
													}
													printf("Vulkan, queueFamily#%d/%d: count(%d) flags(%s%s%s%s%s%s%s%s).\n", i + 1, vkHwDevsCount, p.queueCount, p.queueFlags & VK_QUEUE_GRAPHICS_BIT ? " GRAPHICS" : "", p.queueFlags & VK_QUEUE_COMPUTE_BIT ? " COMPUTE" : "", p.queueFlags & VK_QUEUE_TRANSFER_BIT ? " TRANSFER" : "", p.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ? " SPARSE_BINDING" : "", p.queueFlags & VK_QUEUE_PROTECTED_BIT ? " PROTECTED" : "", p.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR ? " VIDEO_DECODE" : "", p.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR ? " VIDEO_ENCODE" : "", p.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV ? " OPTICAL_FLOW" : "");
												}
												//
												if (vkGraphAndComputeQueueFamIdx < 0) {
													printf("Vulkan, error, no suitable queueFamily found (graphis-and-compute support).\n");
												} else {
													VkDevice vkDev = NULL;
													float vkQueuePriots[] = { 1.0f };
													VkDeviceQueueCreateInfo vkQueueCreateInfo =
													{
														VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
														NULL,
														0,
														vkGraphAndComputeQueueFamIdx,
														sizeof(vkQueuePriots) / sizeof(vkQueuePriots[0]),
														vkQueuePriots
													};
													VkDeviceQueueCreateInfo vkQueueCreateInfos[] = { vkQueueCreateInfo };
													VkDeviceCreateInfo vkDevCreateInfo =
													{
														VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
														NULL,
														0,
														sizeof(vkQueueCreateInfos) / sizeof(vkQueueCreateInfos[0]),
														vkQueueCreateInfos,
														0, // enabledLayerCount deprecated
														NULL, // ppEnabledLayerNames deprecated
														0, //count(deviceExtensions)
														NULL, //deviceExtensions
														NULL // pEnabledFeatures
													};
													if (VK_SUCCESS != vkCreateDevice(vkHwDev, &vkDevCreateInfo, NULL, &vkDev)) {
														printf("Vulkan, error, vkCreateDevice failed.\n");
													} else {
														printf("Vulkan, device created.\n");
														VkQueue vkQueue = NULL;
														vkGetDeviceQueue(vkDev, vkGraphAndComputeQueueFamIdx, 0, &vkQueue);
														if (vkQueue == NULL) {
															printf("Vulkan, error, vkGetDeviceQueue failed.\n");
														} else {
															printf("Vulkan, device queue found.\n");
															//shader
															//OpenGL ES 3.0 => GLSL ES 3.0 => GLSL 3.3
															VkShaderModule sModule = NULL;
															VkShaderModuleCreateInfo sCreateInfo = {
																VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
																NULL,
																0,
																sizeof(NBScnRenderJob_vulkan1_0_430) / sizeof(NBScnRenderJob_vulkan1_0_430[0]), //codeSize
																(const uint32_t*)NBScnRenderJob_vulkan1_0_430 //pCode //TODO: match 'csSrc' length to multiple of 4-bytes
															};
															if (VK_SUCCESS != vkCreateShaderModule(vkDev, &sCreateInfo, NULL, &sModule)) {
																printf("Vulkan, error, vkCreateShaderModule failed.\n");
															} else {
																printf("Vulkan, shader module created.\n");
																//consume
																app->vlkn.inst.obj = vkInst; vkInst = NULL;
																app->vlkn.inst.vkDbgMsgr = vkDbgMsgr; vkDbgMsgr = NULL;
																//
																app->vlkn.queueFam.graphAndCompIdx = vkGraphAndComputeQueueFamIdx;
																app->vlkn.dev.obj = vkDev; vkDev = NULL;
																app->vlkn.dev.queue = vkQueue; vkQueue = NULL;
																app->vlkn.dev.hw.props = vkHwDevProps;
																app->vlkn.dev.hw.mem.props = vkHwDevMemProps;
																app->vlkn.compute.shader.mdl = sModule; sModule = NULL;
																r = true;
															}
															//release (if not consumed)
															if (sModule != NULL) {
																vkDestroyShaderModule(vkDev, sModule, NULL);
																sModule = NULL;
															}
														}
														//
														if (vkQueue != NULL) {
															//
															vkQueue = NULL;
														}
													}
													//release (if not consumed)
													if (vkDev != NULL) {
														vkDestroyDevice(vkDev, NULL);
														vkDev = NULL;
													}
												}
											}
										}
									}
								}
							}
						}
					}
					//release (if not consumed)
					if (vkHwDevs != NULL) {
						free(vkHwDevs);
						vkHwDevs = NULL;
					}
				}
				//release (if not consumed)
				if (vkDbgMsgr != NULL) {
					PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkDestroyDebugUtilsMessengerEXT");
					if (vkDestroyDebugUtilsMessengerEXT == NULL) {
						printf("Vulkan, error, vkGetInstanceProcAddr(vkDestroyDebugUtilsMessengerEXT) failed.\n");
					} else {
						vkDestroyDebugUtilsMessengerEXT(vkInst, vkDbgMsgr, NULL);
						vkDbgMsgr = NULL;
					}
				}
				//release (if not consumed)
				if (vkInst != NULL) {
					vkDestroyInstance(vkInst, NULL);
					vkInst = NULL;
				}
			}
		}
		//release (if not consumed)
		if (vkProps != NULL) {
			free(vkProps);
			vkProps = NULL;
		}
	}
	return r;
}

void App_vulkan_compute_run_in_samples(STApp* app, const STNBScnRenderJobTree* src, const float compareMaxDiffAbs) {
	HRESULT hr;
	const unsigned int spacesPerLvl = 4;
	NBTHREAD_CLOCK osFreq = NBThread_clocksPerSec();
	NBTHREAD_CLOCK cpuFwdTime = 0, cpuBwdTime = 0, cpuBwd2Time = 0, gpuBwdTimeExec = 0, gpuBwdTimeMapping = 0, gpuBwdTimeCpying = 0;
	STNBScnRenderJobPlain cpuFwdRR, cpuBwdRR, cpuBwdRR2, gpuBwdRRMapped, gpuBwdRRCopied;
	//
	NBScnRenderJobPlain_init(&cpuFwdRR);
	NBScnRenderJobPlain_init(&cpuBwdRR);
	NBScnRenderJobPlain_init(&cpuBwdRR2);	//gpu shader code running in cpu
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
		const unsigned long paramsPaddedSz = (sizeof(STParams) + app->vlkn.dev.hw.props.limits.minUniformBufferOffsetAlignment - 1) / app->vlkn.dev.hw.props.limits.minUniformBufferOffsetAlignment * app->vlkn.dev.hw.props.limits.minUniformBufferOffsetAlignment;
		const unsigned long maxValPerDispatch = app->vlkn.dev.hw.props.limits.maxComputeWorkGroupCount[0];
		const unsigned long ammDispatchCalls = (src->nodes.use + maxValPerDispatch - 1) / maxValPerDispatch;
		STNBVulkanBuff paramsBuff;
		STNBVulkanBuff treNodesBuff, treV0Buff, treV1Buff, treV2Buff, treV3Buff;
		STNBVulkanBuff fltNodesBuff, fltV0Buff, fltV1Buff, fltV2Buff, fltV3Buff;
		NBVulkanBuff_init(&treNodesBuff); NBVulkanBuff_init(&treV0Buff); NBVulkanBuff_init(&treV1Buff); NBVulkanBuff_init(&treV2Buff); NBVulkanBuff_init(&treV3Buff);
		NBVulkanBuff_init(&fltNodesBuff); NBVulkanBuff_init(&fltV0Buff); NBVulkanBuff_init(&fltV1Buff); NBVulkanBuff_init(&fltV2Buff); NBVulkanBuff_init(&fltV3Buff);
		//
		if (!App_vulkan_compute_create_uniform_buffer(app, paramsPaddedSz * ammDispatchCalls, &paramsBuff, true)) {
			printf("Vulkan, error, paramsBuff allocation failed.\n");
			//
			//
		} else if (!App_vulkan_compute_create_src_buffer(app, sizeof(STNBScnTreeNode) * src->nodes.use, (void*)NBArray_dataPtr(&src->nodes, STNBScnTreeNode*), &treNodesBuff, true)) {
			printf("Vulkan, error, treNodesBuff allocation failed.\n");
		} else if (!App_vulkan_compute_create_src_buffer(app, sizeof(STNBScnVertex) * src->verts.v.use, (void*)NBArray_dataPtr(&src->verts.v, STNBScnVertex*), &treV0Buff, false)) {
			printf("Vulkan, error, treV0Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_src_buffer(app, sizeof(STNBScnVertexTex) * src->verts.v1.use, (void*)NBArray_dataPtr(&src->verts.v1, STNBScnVertexTex*), &treV1Buff, false)) {
			printf("Vulkan, error, treV1Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_src_buffer(app, sizeof(STNBScnVertexTex2) * src->verts.v2.use, (void*)NBArray_dataPtr(&src->verts.v2, STNBScnVertexTex2*), &treV2Buff, false)) {
			printf("Vulkan, error, treV2Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_src_buffer(app, sizeof(STNBScnVertexTex3) * src->verts.v3.use, (void*)NBArray_dataPtr(&src->verts.v3, STNBScnVertexTex3*), &treV3Buff, false)) {
			printf("Vulkan, error, treV3Buff allocation failed.\n");
			//
			//
		} else if (!App_vulkan_compute_create_dst_buffer(app, sizeof(STNBScnFlatNode) * src->nodes.use, &fltNodesBuff, true)) {
			printf("Vulkan, error, fltNodesBuff allocation failed.\n");
		} else if (!App_vulkan_compute_create_dst_buffer(app, sizeof(STNBScnVertexF) * src->verts.v.use, &fltV0Buff, false)) {
			printf("Vulkan, error, fltV0Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_dst_buffer(app, sizeof(STNBScnVertexTexF) * src->verts.v1.use, &fltV1Buff, false)) {
			printf("Vulkan, error, fltV1Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_dst_buffer(app, sizeof(STNBScnVertexTex2F) * src->verts.v2.use, &fltV2Buff, false)) {
			printf("Vulkan, error, fltV2Buff allocation failed.\n");
		} else if (!App_vulkan_compute_create_dst_buffer(app, sizeof(STNBScnVertexTex3F) * src->verts.v3.use, &fltV3Buff, false)) {
			printf("Vulkan, error, fltV3Buff allocation failed.\n");
		} else {
			//printf("Vulkan, buffs allocated.\n");
			VkDescriptorSetLayoutBinding lays[] = {
				//uniform
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				//src
				, { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				//dst
				, { 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
				, { 10, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL } //binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers
			};
			VkDescriptorSetLayout layDesc = NULL;
			VkDescriptorSetLayoutCreateInfo layoutInfo = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
				, NULL
				, 0 //flags;
				, sizeof(lays) / sizeof(lays[0]) // bindingCount
				, lays //pBindings
			};
			if (VK_SUCCESS != vkCreateDescriptorSetLayout(app->vlkn.dev.obj, &layoutInfo, nullptr, &layDesc)) {
				printf("Vulkan, error, vkCreateDescriptorSetLayout failed.\n");
			} else {
				VkDescriptorPoolSize poolSzs[] = {
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ammDispatchCalls }  //for INLINE_UNIFORM this is the size in bytes
					, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 * ammDispatchCalls } //ammount of buffers
				};
				VkDescriptorPoolCreateInfo poolCreateInfo = {
					VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
					, NULL
					, 0 //flags
					, ammDispatchCalls //maxSets
					, sizeof(poolSzs) / sizeof(poolSzs[0]) //poolSizeCount
					, poolSzs //pPoolSizes;
				};
				VkDescriptorPool descPool = NULL;
				if (VK_SUCCESS != vkCreateDescriptorPool(app->vlkn.dev.obj, &poolCreateInfo, NULL, &descPool)) {
					printf("Vulkan, error, vkCreateDescriptorPool failed.\n");
				} else {
					VkDescriptorSetLayout* layDescs = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * ammDispatchCalls);
					VkDescriptorSetAllocateInfo descSetAllocInfo = {
						VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
						, NULL
						, descPool //descriptorPool;
						, ammDispatchCalls //descriptorSetCount;
						, layDescs //pSetLayouts
					};
					VkDescriptorSet* descSet = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet) * ammDispatchCalls);
					{
						int i; for (i = 0; i < ammDispatchCalls; i++) layDescs[i] = layDesc;
					}
					if (VK_SUCCESS != vkAllocateDescriptorSets(app->vlkn.dev.obj, &descSetAllocInfo, descSet)) {
						printf("Vulkan, error, vkAllocateDescriptorSets(%d) failed (%d threads per dispatch).\n", ammDispatchCalls, maxValPerDispatch);
					} else {
						//populate exec params
						{
							unsigned int execCount = 0, iDispacth = 0;
							STParams execPrms;
							NBMemory_setZeroSt(execPrms, STParams);
							BYTE* params = (BYTE*)malloc(paramsPaddedSz * ammDispatchCalls);
							VkDescriptorBufferInfo* buffsInfo = (VkDescriptorBufferInfo*)malloc(sizeof(VkDescriptorBufferInfo) * ammDispatchCalls * 11);
							VkWriteDescriptorSet* descsWrite = (VkWriteDescriptorSet*)malloc(sizeof(VkWriteDescriptorSet) * ammDispatchCalls * 2);
							//
							memset(params, 0, paramsPaddedSz * ammDispatchCalls);
							memset(buffsInfo, 0, sizeof(VkDescriptorBufferInfo) * ammDispatchCalls * 11);
							memset(descsWrite, 0, sizeof(VkWriteDescriptorSet) * ammDispatchCalls * 2);
							//
							while (execPrms.iNodeOffset < src->nodes.use) {
								execCount = src->nodes.use - execPrms.iNodeOffset;
								if (execCount > maxValPerDispatch) {
									execCount = (UI32)maxValPerDispatch;
								}
								//populate step param
								*((STParams*)&params[paramsPaddedSz * iDispacth]) = execPrms;
								//
								buffsInfo[(iDispacth * 11) + 0] = { paramsBuff.buff, paramsPaddedSz * iDispacth, paramsPaddedSz }; //buffer, offset, range
								//
								buffsInfo[(iDispacth * 11) + 1] = { treNodesBuff.buff, 0, treNodesBuff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 2] = { treV0Buff.buff, 0, treV0Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 3] = { treV1Buff.buff, 0, treV1Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 4] = { treV2Buff.buff, 0, treV2Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 5] = { treV3Buff.buff, 0, treV3Buff.mappedSz }; //buffer, offset, range
								//
								buffsInfo[(iDispacth * 11) + 6] = { fltNodesBuff.buff, 0, fltNodesBuff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 7] = { fltV0Buff.buff, 0, fltV0Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 8] = { fltV1Buff.buff, 0, fltV1Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 9] = { fltV2Buff.buff, 0, fltV2Buff.mappedSz }; //buffer, offset, range
								buffsInfo[(iDispacth * 11) + 10] = { fltV3Buff.buff, 0 , fltV3Buff.mappedSz }; //buffer, offset, range
								//
								// //uniform buffer
								descsWrite[(iDispacth * 2) + 0] = {
									VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
										, NULL
										, descSet[iDispacth] //dstSet
										, 0 //dstBinding
										, 0 //dstArrayElement
										, 1 //descriptorCount
										, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER //descriptorType
										, NULL //pImageInfo
										, &buffsInfo[(iDispacth * 11) + 0] //pBufferInfo
										, NULL // pTexelBufferView
								};
								//storage
								descsWrite[(iDispacth * 2) + 1] = {
									VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
									, NULL
									, descSet[iDispacth] //dstSet
									, 1 //dstBinding
									, 0 //dstArrayElement
									, 10 //descriptorCount
									, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER //descriptorType
									, NULL //pImageInfo
									, &buffsInfo[(iDispacth * 11) + 1] //pBufferInfo
									, NULL // pTexelBufferView
								};
								//next
								execPrms.iNodeOffset += execCount;
								iDispacth++;
							}
							//
							memcpy(paramsBuff.mapped, params, paramsPaddedSz * ammDispatchCalls);
							vkUpdateDescriptorSets(app->vlkn.dev.obj, ammDispatchCalls * 2, descsWrite, 0, NULL);
							//
							if (params != NULL) {
								free(params);
								params = NULL;
							}
							if (buffsInfo != NULL) {
								free(buffsInfo);
								buffsInfo = NULL;
							}
							if (descsWrite != NULL) {
								free(descsWrite);
								descsWrite = NULL;
							}
						}
						//pipeline
						{
							VkPipelineLayout pipeLay = NULL;
							VkPipelineLayoutCreateInfo pipeLayInfo = {
								VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
								, NULL
								, 0 //flags
								, 1 //setLayoutCount
								, &layDesc //pSetLayouts
								, 0 //pushConstantRangeCount;
								, NULL //pPushConstantRanges;
							};
							if (VK_SUCCESS != vkCreatePipelineLayout(app->vlkn.dev.obj, &pipeLayInfo, nullptr, &pipeLay)) {
								printf("Vulkan, error, vkCreatePipelineLayout failed.\n");
							} else {
								VkPipelineShaderStageCreateInfo stageCreateInfo = {
								VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
								, NULL
								, 0 //flags
								, VK_SHADER_STAGE_COMPUTE_BIT //stage
								, app->vlkn.compute.shader.mdl //module
								, "main"
								, NULL //pSpecializationInfo;
								};
								VkComputePipelineCreateInfo pipeCreateInfo = {
									VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
									, NULL
									, 0 //flags;
									, stageCreateInfo //stage;
									, pipeLay //layout;
									, NULL //basePipelineHandle
									, 0 //basePipelineIndex
								};
								VkPipeline pipeln = NULL;
								if (VK_SUCCESS != vkCreateComputePipelines(app->vlkn.dev.obj, NULL, 1, &pipeCreateInfo, nullptr, &pipeln)) {
									printf("Vulkan, error, vkCreateComputePipelines failed.\n");
								} else {
									VkCommandPoolCreateInfo cmdPoolCreateInfo = {
										VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
										, NULL
										, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT //flags
										, app->vlkn.queueFam.graphAndCompIdx //queueFamilyIndex
									};
									VkCommandPool cmdPool = NULL;
									if (VK_SUCCESS != vkCreateCommandPool(app->vlkn.dev.obj, &cmdPoolCreateInfo, NULL, &cmdPool)) {
										printf("Vulkan, error, vkCreateCommandPool failed.\n");
									} else {
										VkCommandBufferAllocateInfo cmdBuffAllocInfo = {
											VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
											, NULL
											, cmdPool //commandPool
											, VK_COMMAND_BUFFER_LEVEL_PRIMARY //VkCommandBufferLevel    level;
											, 1 //uint32_t                commandBufferCount;
										};
										VkCommandBuffer cmdBuff = NULL;
										if (VK_SUCCESS != vkAllocateCommandBuffers(app->vlkn.dev.obj, &cmdBuffAllocInfo, &cmdBuff)) {
											printf("Vulkan, error, vkAllocateCommandBuffers failed.\n");
										} else {
											unsigned int execCount = 0;
											STParams execPrms;
											NBMemory_setZeroSt(execPrms, STParams);
											//
											NBTHREAD_CLOCK startTime, midTime, midTime2, endTime;
											midTime = midTime2 = endTime = startTime = NBThread_clock();
											//
											VkCommandBufferBeginInfo cmdBuffBeginInfo = {
														VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
														, NULL
														, 0 //flags;
														, NULL //pInheritanceInfo;
											};
											if (VK_SUCCESS != vkBeginCommandBuffer(cmdBuff, &cmdBuffBeginInfo)) { //implicit reset of the command
												printf("Vulkan, error, vkBeginCommandBuffer failed.\n");
											} else {
												vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipeln);
												//
												while (execPrms.iNodeOffset < src->nodes.use) {
													execCount = src->nodes.use - execPrms.iNodeOffset;
													if (execCount > maxValPerDispatch) {
														execCount = maxValPerDispatch;
													}
													//execute
													vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipeLay, 0, 1, &descSet[execPrms.iNodeOffset / maxValPerDispatch], 0, 0);
													vkCmdDispatch(cmdBuff, execCount, 1, 1);
													//next
													execPrms.iNodeOffset += execCount;
												}
												//
												if (VK_SUCCESS != vkEndCommandBuffer(cmdBuff) != VK_SUCCESS) {
													printf("Vulkan, error, vkEndCommandBuffer failed.\n");
												} else {
													VkSubmitInfo submtInfo = {
														VK_STRUCTURE_TYPE_SUBMIT_INFO
														, NULL
														, 0 //waitSemaphoreCount;
														, NULL //pWaitSemaphores;
														, NULL //pWaitDstStageMask;
														, 1 //commandBufferCount;
														, &cmdBuff //pCommandBuffers;
														, 0 //signalSemaphoreCount;
														, NULL //pSignalSemaphores;
													};
													//
													if (VK_SUCCESS != vkQueueSubmit(app->vlkn.dev.queue, 1, &submtInfo, NULL)) {
														printf("Vulkan, error, vkQueueSubmit failed.\n");
													} else {
														printf("Vulkan, waiting for queue enter to iddle state.\n");
														vkQueueWaitIdle(app->vlkn.dev.queue);
													}
												}
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
												NBArray_initWithExternalBuffer(&gpuBwdRRMapped.nodes, sizeof(STNBScnFlatNode), NULL, (BYTE*)fltNodesBuff.mapped);
												gpuBwdRRMapped.nodes.use = src->nodes.use;
												NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v, sizeof(STNBScnVertexF), NULL, (BYTE*)fltV0Buff.mapped);
												gpuBwdRRMapped.verts.v.use = src->verts.v.use;
												NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v1, sizeof(STNBScnVertexTexF), NULL, (BYTE*)fltV1Buff.mapped);
												gpuBwdRRMapped.verts.v1.use = src->verts.v1.use;
												NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v2, sizeof(STNBScnVertexTex2F), NULL, (BYTE*)fltV2Buff.mapped);
												gpuBwdRRMapped.verts.v2.use = src->verts.v2.use;
												NBArray_initWithExternalBuffer(&gpuBwdRRMapped.verts.v3, sizeof(STNBScnVertexTex3F), NULL, (BYTE*)fltV3Buff.mapped);
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
										}
										if (cmdBuff != NULL) {
											vkFreeCommandBuffers(app->vlkn.dev.obj, cmdPool, 1, &cmdBuff);
											cmdBuff = NULL;
										}
									}
									if (cmdPool != NULL) {
										vkDestroyCommandPool(app->vlkn.dev.obj, cmdPool, NULL);
										cmdPool = NULL;
									}
								}
								if (pipeln != NULL) {
									vkDestroyPipeline(app->vlkn.dev.obj, pipeln, NULL);
									pipeln = NULL;
								}
							}
							if (pipeLay != NULL) {
								vkDestroyPipelineLayout(app->vlkn.dev.obj, pipeLay, NULL);
								pipeLay = NULL;
							}
						}
					}
					if (layDescs != NULL) {
						free(layDescs);
						layDescs = NULL;
					}
					if (descSet != NULL) {
						free(descSet);
						descSet = NULL;
					}
				}
				if (descPool != NULL) {
					vkDestroyDescriptorPool(app->vlkn.dev.obj, descPool, NULL);
					descPool = NULL;
				}
			}
			if (layDesc != NULL) {
				vkDestroyDescriptorSetLayout(app->vlkn.dev.obj, layDesc, NULL);
				layDesc = NULL;
			}
		}
		//release
		NBVulkanBuff_release(&paramsBuff);
		NBVulkanBuff_release(&treNodesBuff); NBVulkanBuff_release(&treV0Buff); NBVulkanBuff_release(&treV1Buff); NBVulkanBuff_release(&treV2Buff); NBVulkanBuff_release(&treV3Buff);
		NBVulkanBuff_release(&fltNodesBuff); NBVulkanBuff_release(&fltV0Buff); NBVulkanBuff_release(&fltV1Buff); NBVulkanBuff_release(&fltV2Buff); NBVulkanBuff_release(&fltV3Buff);
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
	NBScnRenderJobPlain_release(&cpuFwdRR);
	NBScnRenderJobPlain_release(&cpuBwdRR);
	NBScnRenderJobPlain_release(&gpuBwdRRMapped);
	NBScnRenderJobPlain_release(&gpuBwdRRCopied);
}

bool App_vulkan_compute_create_src_buffer(STApp* app, const unsigned int sz, void* data, STNBVulkanBuff* dst, const bool printBuffTypeSelected) {
	bool r = false;
	const unsigned int allocSz = (sz == 0 ? app->vlkn.dev.hw.props.limits.nonCoherentAtomSize : (sz + (app->vlkn.dev.hw.props.limits.nonCoherentAtomSize - 1)) / app->vlkn.dev.hw.props.limits.nonCoherentAtomSize * app->vlkn.dev.hw.props.limits.nonCoherentAtomSize);
	VkBuffer buff = NULL;
	VkBufferCreateInfo buffInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
		, NULL	//next
		, 0		//flags
		, allocSz
		, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT //usage
		, VK_SHARING_MODE_EXCLUSIVE //sharingMode; exclusive: not  shared between family-queues
		, 0 //queueFamilyIndexCount
		, NULL //pQueueFamilyIndices
	};
	if (VK_SUCCESS != vkCreateBuffer(app->vlkn.dev.obj, &buffInfo, NULL, &buff)) {
		printf("Vulkan, error, vkCreateBuffer failed for compute_src_buffer(%d bytes).\n", allocSz);
	} else {
		int iMemoryType = -1;
		VkMemoryRequirements memReqs;
		//memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		vkGetBufferMemoryRequirements(app->vlkn.dev.obj, buff, &memReqs);
		//printf("Vulkan, compute_src_buffer(%d bytes) requires: size(%u) offset-alignment(%d).\n", allocSz, memReqs.size
		//	, memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		//);
		{
			int i; for (i = 0; i < app->vlkn.dev.hw.mem.props.memoryTypeCount; i++) {
				const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[i];
				const VkMemoryType* mtCur = (iMemoryType == -1 ? NULL : &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType]);
				if (
					memReqs.memoryTypeBits & (1 << i) //filtered by 'vkGetBufferMemoryRequirements'
					&& (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //must be visible to host
					&& (
						mtCur == NULL //is first option found
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) //always prefer device_local memory (even if we have to sync it)
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == (mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //under the same device_local value, prefer the host_coherent one
						)
					) {
					iMemoryType = i;
				}
			}
		}
		if (iMemoryType < 0) {
			printf("Vulkan, error, could not find memory-type for src-buffer.\n");
		} else {
			const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType];
			if (printBuffTypeSelected) {
				printf("Src-buffer selected memory-type-#%d/%d: heap#%d %s%s%s%s%s%s%s%s%s.\n", iMemoryType + 1, app->vlkn.dev.hw.mem.props.memoryTypeCount, (mt->heapIndex + 1)
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ? " | device_local" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? " | host_visible" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? " | host_coherent" : ""	//vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to manage this memory.
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT ? " | host_cached" : ""		//memory is cached on the host for faster access
					, mt->propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ? " | lazily_alloc" : ""	//only accesible to device (gpu)
					//VK_VERSION_1_1
					, mt->propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT ? " | protected" : ""			//only device's protected queue operations to access the memory.
					//VK_AMD_device_coherent_memory
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ? " | device_coherent_amd" : ""	//device's access and visibility of this memory is automatic
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ? " | device_uncached_amd" : ""	//uncached device memory is always device coherent
					//VK_NV_external_memory_rdma
					, mt->propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ? " | rdma_capable_nv" : ""	//external devices can access this memory directly
				);
			}
			VkDeviceMemory mem = NULL;
			VkMemoryAllocateInfo allocInfo = {
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
				, NULL //next
				, allocSz //allocationSize
				, iMemoryType//memoryTypeIndex
			};
			if (VK_SUCCESS != vkAllocateMemory(app->vlkn.dev.obj, &allocInfo, nullptr, &mem)) {
				printf("Vulkan, error, src-buffer vkAllocateMemory(%d bytes) falied.\n", allocSz);
			} else if (VK_SUCCESS != vkBindBufferMemory(app->vlkn.dev.obj, buff, mem, 0)) {
				printf("Vulkan, error, src-buffer vkBindBufferMemory(%d bytes) falied.\n", allocSz);
			} else {
				void* mapped = NULL;
				if (VK_SUCCESS != vkMapMemory(app->vlkn.dev.obj, mem, 0, allocSz, 0, &mapped)) {
					printf("Vulkan, error, src-buffer vkMapMemory(%d bytes) falied.\n", allocSz);
				} else {
					memcpy(mapped, data, (size_t)sz);
					//
					dst->dev = app->vlkn.dev.obj;
					dst->buff = buff; buff = NULL; //consume
					dst->mem = mem; mem = NULL; //consume
					dst->memFlags = mt->propertyFlags;
					dst->mapped = mapped;  mapped = NULL; //consume
					dst->mappedDataSz = sz;		//populated data
					dst->mappedSz = allocSz;	//allocated space
					dst->alignment = memReqs.alignment;
					dst->nonCoherentAtomSize = app->vlkn.dev.hw.props.limits.nonCoherentAtomSize;
					r = true;
				}
				//unmap (if not consumed)
				if (mapped != NULL) {
					vkUnmapMemory(app->vlkn.dev.obj, mem);
					mapped = NULL;
				}
			}
			//release (if not consumed)
			if (mem != NULL) {
				vkFreeMemory(app->vlkn.dev.obj, mem, NULL);
				mem = NULL;
			}
		}
	}
	//release (if not consumed)
	if (buff != NULL) {
		vkDestroyBuffer(app->vlkn.dev.obj, buff, NULL);
		buff = NULL;
	}
	return r;
}

bool App_vulkan_compute_create_dst_buffer(STApp* app, const unsigned int sz, STNBVulkanBuff* dst, const bool printBuffTypeSelected) {
	bool r = false;
	const unsigned int allocSz = (sz == 0 ? app->vlkn.dev.hw.props.limits.nonCoherentAtomSize : (sz + (app->vlkn.dev.hw.props.limits.nonCoherentAtomSize - 1)) / app->vlkn.dev.hw.props.limits.nonCoherentAtomSize * app->vlkn.dev.hw.props.limits.nonCoherentAtomSize);
	VkBuffer buff = NULL;
	VkBufferCreateInfo buffInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
		, NULL	//next
		, 0		//flags
		, allocSz
		, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT //usage
		, VK_SHARING_MODE_EXCLUSIVE //sharingMode; exclusive: not  shared between family-queues
		, 0 //queueFamilyIndexCount
		, NULL //pQueueFamilyIndices
	};
	if (VK_SUCCESS != vkCreateBuffer(app->vlkn.dev.obj, &buffInfo, NULL, &buff)) {
		printf("Vulkan, error, vkCreateBuffer failed for compute_dst_buffer(%d bytes).\n", allocSz);
	} else {
		int iMemoryType = -1;
		VkMemoryRequirements memReqs;
		//memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		vkGetBufferMemoryRequirements(app->vlkn.dev.obj, buff, &memReqs);
		//printf("Vulkan, compute_dst_buffer(%d bytes) requires: size(%u) offset-alignment(%d).\n", allocSz, memReqs.size
		//	, memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		//);
		{
			int i; for (i = 0; i < app->vlkn.dev.hw.mem.props.memoryTypeCount; i++) {
				const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[i];
				const VkMemoryType* mtCur = (iMemoryType == -1 ? NULL : &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType]);
				if (
					memReqs.memoryTypeBits & (1 << i) //filtered by 'vkGetBufferMemoryRequirements'
					&& (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //must be visible to host
					&& (
						mtCur == NULL //is first option found
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) //always prefer device_local memory (even if we have to sync it)
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == (mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //under the same device_local value, prefer the host_coherent one
						)
					) {
					iMemoryType = i;
				}
			}
		}
		if (iMemoryType < 0) {
			printf("Vulkan, error, could not find memory-type for dst-buffer.\n");
		} else {
			const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType];
			if (printBuffTypeSelected) {
				printf("Dst-buffer selected memory-type-#%d/%d: heap#%d %s%s%s%s%s%s%s%s%s.\n", iMemoryType + 1, app->vlkn.dev.hw.mem.props.memoryTypeCount, (mt->heapIndex + 1)
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ? " | device_local" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? " | host_visible" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? " | host_coherent" : ""	//vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to manage this memory.
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT ? " | host_cached" : ""		//memory is cached on the host for faster access
					, mt->propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ? " | lazily_alloc" : ""	//only accesible to device (gpu)
					//VK_VERSION_1_1
					, mt->propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT ? " | protected" : ""			//only device's protected queue operations to access the memory.
					//VK_AMD_device_coherent_memory
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ? " | device_coherent_amd" : ""	//device's access and visibility of this memory is automatic
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ? " | device_uncached_amd" : ""	//uncached device memory is always device coherent
					//VK_NV_external_memory_rdma
					, mt->propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ? " | rdma_capable_nv" : ""	//external devices can access this memory directly
				);
			}
			VkDeviceMemory mem = NULL;
			VkMemoryAllocateInfo allocInfo = {
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
				, NULL //next
				, allocSz //allocationSize
				, iMemoryType//memoryTypeIndex
			};
			if (VK_SUCCESS != vkAllocateMemory(app->vlkn.dev.obj, &allocInfo, nullptr, &mem)) {
				printf("Vulkan, error, dst-buffer vkAllocateMemory(%d bytes) falied.\n", allocSz);
			} else if (VK_SUCCESS != vkBindBufferMemory(app->vlkn.dev.obj, buff, mem, 0)) {
				printf("Vulkan, error, dst-buffer vkBindBufferMemory(%d bytes) falied.\n", allocSz);
			} else {
				void* mapped = NULL;
				if (VK_SUCCESS != vkMapMemory(app->vlkn.dev.obj, mem, 0, allocSz, 0, &mapped)) {
					printf("Vulkan, error, dst-buffer vkMapMemory(%d bytes) falied.\n", allocSz);
				} else {
					//	memcpy(mapped, data, (size_t)sz);
						//
					dst->dev = app->vlkn.dev.obj;
					dst->buff = buff; buff = NULL; //consume
					dst->mem = mem; mem = NULL; //consume
					dst->memFlags = mt->propertyFlags;
					dst->mapped = mapped;  mapped = NULL; //consume
					dst->mappedDataSz = sz;		//populated data
					dst->mappedSz = allocSz;	//allocated space
					dst->alignment = memReqs.alignment;
					dst->nonCoherentAtomSize = app->vlkn.dev.hw.props.limits.nonCoherentAtomSize;
					r = true;
				}
				//unmap (if not consumed)
				if (mapped != NULL) {
					vkUnmapMemory(app->vlkn.dev.obj, mem);
					mapped = NULL;
				}
			}
			//release (if not consumed)
			if (mem != NULL) {
				vkFreeMemory(app->vlkn.dev.obj, mem, NULL);
				mem = NULL;
			}
		}
	}
	//release (if not consumed)
	if (buff != NULL) {
		vkDestroyBuffer(app->vlkn.dev.obj, buff, NULL);
		buff = NULL;
	}
	return r;
}

bool App_vulkan_compute_create_uniform_buffer(STApp* app, const unsigned int sz, STNBVulkanBuff* dst, const bool printBuffTypeSelected) {
	bool r = false;
	const unsigned int allocSz = (sz == 0 ? app->vlkn.dev.hw.props.limits.nonCoherentAtomSize : (sz + (app->vlkn.dev.hw.props.limits.nonCoherentAtomSize - 1)) / app->vlkn.dev.hw.props.limits.nonCoherentAtomSize * app->vlkn.dev.hw.props.limits.nonCoherentAtomSize);
	VkBuffer buff = NULL;
	VkBufferCreateInfo buffInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
		, NULL	//next
		, 0		//flags
		, allocSz
		, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT //usage
		, VK_SHARING_MODE_EXCLUSIVE //sharingMode; exclusive: not  shared between family-queues
		, 0 //queueFamilyIndexCount
		, NULL //pQueueFamilyIndices
	};
	if (VK_SUCCESS != vkCreateBuffer(app->vlkn.dev.obj, &buffInfo, NULL, &buff)) {
		printf("Vulkan, error, vkCreateBuffer failed for compute_uniform_buffer(%d bytes).\n", allocSz);
	} else {
		int iMemoryType = -1;
		VkMemoryRequirements memReqs;
		//memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		vkGetBufferMemoryRequirements(app->vlkn.dev.obj, buff, &memReqs);
		//printf("Vulkan, compute_uniform_buffer(%d bytes) requires: size(%u) offset-alignment(%d).\n", allocSz, memReqs.size
		//	, memReqs.alignment //requirement for vkBindbufferMemory() and vkBindImageMemory()
		//);
		{
			int i; for (i = 0; i < app->vlkn.dev.hw.mem.props.memoryTypeCount; i++) {
				const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[i];
				const VkMemoryType* mtCur = (iMemoryType == -1 ? NULL : &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType]);
				if (
					memReqs.memoryTypeBits & (1 << i) //filtered by 'vkGetBufferMemoryRequirements'
					&& (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //must be visible to host
					&& (
						mtCur == NULL //is first option found
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) //always prefer device_local memory (even if we have to sync it)
						|| ((mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == (mtCur->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && (mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && !(mtCur->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //under the same device_local value, prefer the host_coherent one
						)
					) {
					iMemoryType = i;
				}
			}
		}
		if (iMemoryType < 0) {
			printf("Vulkan, error, could not find memory-type for uniform-buffer.\n");
		} else {
			const VkMemoryType* mt = &app->vlkn.dev.hw.mem.props.memoryTypes[iMemoryType];
			if (printBuffTypeSelected) {
				printf("Uniform-buffer selected memory-type-#%d/%d: heap#%d %s%s%s%s%s%s%s%s%s.\n", iMemoryType + 1, app->vlkn.dev.hw.mem.props.memoryTypeCount, (mt->heapIndex + 1)
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ? " | device_local" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? " | host_visible" : ""
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? " | host_coherent" : ""	//vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to manage this memory.
					, mt->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT ? " | host_cached" : ""		//memory is cached on the host for faster access
					, mt->propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ? " | lazily_alloc" : ""	//only accesible to device (gpu)
					//VK_VERSION_1_1
					, mt->propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT ? " | protected" : ""			//only device's protected queue operations to access the memory.
					//VK_AMD_device_coherent_memory
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ? " | device_coherent_amd" : ""	//device's access and visibility of this memory is automatic
					, mt->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ? " | device_uncached_amd" : ""	//uncached device memory is always device coherent
					//VK_NV_external_memory_rdma
					, mt->propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ? " | rdma_capable_nv" : ""	//external devices can access this memory directly
				);
			}
			VkDeviceMemory mem = NULL;
			VkMemoryAllocateInfo allocInfo = {
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
				, NULL //next
				, allocSz //allocationSize
				, iMemoryType//memoryTypeIndex
			};
			if (VK_SUCCESS != vkAllocateMemory(app->vlkn.dev.obj, &allocInfo, nullptr, &mem)) {
				printf("Vulkan, error, uniform-buffer vkAllocateMemory(%d bytes) falied.\n", allocSz);
			} else if (VK_SUCCESS != vkBindBufferMemory(app->vlkn.dev.obj, buff, mem, 0)) {
				printf("Vulkan, error, uniform-buffer vkBindBufferMemory(%d bytes) falied.\n", allocSz);
			} else {
				void* mapped = NULL;
				if (VK_SUCCESS != vkMapMemory(app->vlkn.dev.obj, mem, 0, allocSz, 0, &mapped)) {
					printf("Vulkan, error, uniform-buffer vkMapMemory(%d bytes) falied.\n", allocSz);
				} else {
					//memcpy(mapped, data, (size_t)sz);
					//
					dst->dev = app->vlkn.dev.obj;
					dst->buff = buff; buff = NULL; //consume
					dst->mem = mem; mem = NULL; //consume
					dst->memFlags = mt->propertyFlags;
					dst->mapped = mapped;  mapped = NULL; //consume
					dst->mappedDataSz = sz;		//populated data
					dst->mappedSz = allocSz;	//allocated space
					dst->alignment = memReqs.alignment;
					dst->nonCoherentAtomSize = app->vlkn.dev.hw.props.limits.nonCoherentAtomSize;
					r = true;
				}
				//unmap (if not consumed)
				if (mapped != NULL) {
					vkUnmapMemory(app->vlkn.dev.obj, mem);
					mapped = NULL;
				}
			}
			//release (if not consumed)
			if (mem != NULL) {
				vkFreeMemory(app->vlkn.dev.obj, mem, NULL);
				mem = NULL;
			}
		}
	}
	//release (if not consumed)
	if (buff != NULL) {
		vkDestroyBuffer(app->vlkn.dev.obj, buff, NULL);
		buff = NULL;
	}
	return r;
}

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

