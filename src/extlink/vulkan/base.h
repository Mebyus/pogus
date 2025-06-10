typedef s32 vk_Result;

typedef u32 vk_Bool32;
typedef u64 vk_DeviceAddress;
typedef u64 vk_DeviceSize;

//     VK_SUCCESS = 0,
//     VK_NOT_READY = 1,
//     VK_TIMEOUT = 2,
//     VK_EVENT_SET = 3,
//     VK_EVENT_RESET = 4,
//     VK_INCOMPLETE = 5,
//     VK_ERROR_OUT_OF_HOST_MEMORY = -1,
//     VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
//     VK_ERROR_INITIALIZATION_FAILED = -3,
//     VK_ERROR_DEVICE_LOST = -4,
//     VK_ERROR_MEMORY_MAP_FAILED = -5,
//     VK_ERROR_LAYER_NOT_PRESENT = -6,
//     VK_ERROR_EXTENSION_NOT_PRESENT = -7,
//     VK_ERROR_FEATURE_NOT_PRESENT = -8,

typedef enum {
    VK_STRUCTURE_TYPE_APPLICATION_INFO = 0,
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1,
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 2,
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3,
    VK_STRUCTURE_TYPE_SUBMIT_INFO = 4,
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5,

    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO = 8,
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9,

    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO = 12,

    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 37,
    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 38,
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO = 39,
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO = 40,
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO = 41,
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42,
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO = 43,
    VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER = 44,
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER = 45,
    VK_STRUCTURE_TYPE_MEMORY_BARRIER = 46,

    VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR = 1000005000,
} vk_StructureType;

/* Opaque handle types */
typedef struct vk_ObjectInstance*        vk_Instance;
typedef struct vk_ObjectPhysicalDevice*  vk_PhysicalDevice;
typedef struct vk_ObjectSurfaceKHR*      vk_SurfaceKHR;
typedef struct vk_ObjectDevice*          vk_Device;
typedef struct vk_ObjectCommandPool*     vk_CommandPool;
typedef struct vk_ObjectQueue*           vk_Queue;
typedef struct vk_ObjectSemaphore*       vk_Semaphore;
typedef struct vk_ObjectFence*           vk_Fence;
typedef struct vk_ObjectCommandBuffer*   vk_CommandBuffer;
typedef struct vk_ObjectBuffer*          vk_Buffer;
typedef struct vk_ObjectDeviceMemory*    vk_DeviceMemory;
typedef struct vk_ObjectFramebuffer*     vk_Framebuffer;  
typedef struct vk_ObjectRenderPass*      vk_RenderPass;
/* =================== */

typedef struct {
    vk_StructureType   type;
    const void*        next;
    const u8*          app_name;
    u32                app_version;
    const u8*          engine_name;
    u32                engine_version;
    u32                api_version;
} vk_ApplicationInfo;

typedef struct {
    vk_StructureType            type;
    const void*                 next;
    u32                         flags;
    const vk_ApplicationInfo*   app_info;
    u32                         layer_count;
    const u8* const*            layer_names;
    u32                         ext_count;
    const u8* const*            ext_names;
} vk_InstanceCreateInfo;

#define VK_MAX_EXTENSION_NAME_SIZE 256

typedef struct {
    u8  ext_name[VK_MAX_EXTENSION_NAME_SIZE];
    u32 spec_version;
} vk_ExtensionProperties;

// typedef void* (VKAPI_PTR *PFN_vkAllocationFunction)(
//     void*                                       pUserData,
//     uint                                      size,
//     uint                                      alignment,
//     VkSystemAllocationScope                     allocationScope);

// typedef void (VKAPI_PTR *PFN_vkVoidFunction)(void);
// typedef struct VkAllocationCallbacks {
//     void*                                   pUserData;
//     PFN_vkAllocationFunction                pfnAllocation;
//     PFN_vkReallocationFunction              pfnReallocation;
//     PFN_vkFreeFunction                      pfnFree;
//     PFN_vkInternalAllocationNotification    pfnInternalAllocation;
//     PFN_vkInternalFreeNotification          pfnInternalFree;
// } VkAllocationCallbacks;

typedef enum {
    VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
    VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
    VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM = 0x7FFFFFFF
} vk_PhysicalDeviceType;

typedef struct {
    u32              maxImageDimension1D;
    u32              maxImageDimension2D;
    u32              maxImageDimension3D;
    u32              maxImageDimensionCube;
    u32              maxImageArrayLayers;
    u32              maxTexelBufferElements;
    u32              maxUniformBufferRange;
    u32              maxStorageBufferRange;
    u32              maxPushConstantsSize;
    u32              maxMemoryAllocationCount;
    u32              maxSamplerAllocationCount;
    vk_DeviceSize          bufferImageGranularity;
    vk_DeviceSize          sparseAddressSpaceSize;
    u32              maxBoundDescriptorSets;
    u32              maxPerStageDescriptorSamplers;
    u32              maxPerStageDescriptorUniformBuffers;
    u32              maxPerStageDescriptorStorageBuffers;
    u32              maxPerStageDescriptorSampledImages;
    u32              maxPerStageDescriptorStorageImages;
    u32              maxPerStageDescriptorInputAttachments;
    u32              maxPerStageResources;
    u32              maxDescriptorSetSamplers;
    u32              maxDescriptorSetUniformBuffers;
    u32              maxDescriptorSetUniformBuffersDynamic;
    u32              maxDescriptorSetStorageBuffers;
    u32              maxDescriptorSetStorageBuffersDynamic;
    u32              maxDescriptorSetSampledImages;
    u32              maxDescriptorSetStorageImages;
    u32              maxDescriptorSetInputAttachments;
    u32              maxVertexInputAttributes;
    u32              maxVertexInputBindings;
    u32              maxVertexInputAttributeOffset;
    u32              maxVertexInputBindingStride;
    u32              maxVertexOutputComponents;
    u32              maxTessellationGenerationLevel;
    u32              maxTessellationPatchSize;
    u32              maxTessellationControlPerVertexInputComponents;
    u32              maxTessellationControlPerVertexOutputComponents;
    u32              maxTessellationControlPerPatchOutputComponents;
    u32              maxTessellationControlTotalOutputComponents;
    u32              maxTessellationEvaluationInputComponents;
    u32              maxTessellationEvaluationOutputComponents;
    u32              maxGeometryShaderInvocations;
    u32              maxGeometryInputComponents;
    u32              maxGeometryOutputComponents;
    u32              maxGeometryOutputVertices;
    u32              maxGeometryTotalOutputComponents;
    u32              maxFragmentInputComponents;
    u32              maxFragmentOutputAttachments;
    u32              maxFragmentDualSrcAttachments;
    u32              maxFragmentCombinedOutputResources;
    u32              maxComputeSharedMemorySize;
    u32              maxComputeWorkGroupCount[3];
    u32              maxComputeWorkGroupInvocations;
    u32              maxComputeWorkGroupSize[3];
    u32              subPixelPrecisionBits;
    u32              subTexelPrecisionBits;
    u32              mipmapPrecisionBits;
    u32              maxDrawIndexedIndexValue;
    u32              maxDrawIndirectCount;
    f32                 maxSamplerLodBias;
    f32                 maxSamplerAnisotropy;
    u32              maxViewports;
    u32              maxViewportDimensions[2];
    f32                 viewportBoundsRange[2];
    u32              viewportSubPixelBits;
    uint                minMemoryMapAlignment;
    vk_DeviceSize          minTexelBufferOffsetAlignment;
    vk_DeviceSize          minUniformBufferOffsetAlignment;
    vk_DeviceSize          minStorageBufferOffsetAlignment;
    s32               minTexelOffset;
    u32              maxTexelOffset;
    s32               minTexelGatherOffset;
    u32              maxTexelGatherOffset;
    f32                 minInterpolationOffset;
    f32                 maxInterpolationOffset;
    u32              subPixelInterpolationOffsetBits;
    u32              maxFramebufferWidth;
    u32              maxFramebufferHeight;
    u32              maxFramebufferLayers;
    u32    framebufferColorSampleCounts;
    u32    framebufferDepthSampleCounts;
    u32    framebufferStencilSampleCounts;
    u32    framebufferNoAttachmentsSampleCounts;
    u32              maxColorAttachments;
    u32    sampledImageColorSampleCounts;
    u32    sampledImageIntegerSampleCounts;
    u32    sampledImageDepthSampleCounts;
    u32    sampledImageStencilSampleCounts;
    u32    storageImageSampleCounts;
    u32              maxSampleMaskWords;
    vk_Bool32              timestampComputeAndGraphics;
    f32                 timestampPeriod;
    u32              maxClipDistances;
    u32              maxCullDistances;
    u32              maxCombinedClipAndCullDistances;
    u32              discreteQueuePriorities;
    f32                 pointSizeRange[2];
    f32                 lineWidthRange[2];
    f32                 pointSizeGranularity;
    f32                 lineWidthGranularity;
    vk_Bool32              strictLines;
    vk_Bool32              standardSampleLocations;
    vk_DeviceSize          optimalBufferCopyOffsetAlignment;
    vk_DeviceSize          optimalBufferCopyRowPitchAlignment;
    vk_DeviceSize          nonCoherentAtomSize;
} vk_PhysicalDeviceLimits;

typedef struct {
    vk_Bool32    residencyStandard2DBlockShape;
    vk_Bool32    residencyStandard2DMultisampleBlockShape;
    vk_Bool32    residencyStandard3DBlockShape;
    vk_Bool32    residencyAlignedMipSize;
    vk_Bool32    residencyNonResidentStrict;
} vk_PhysicalDeviceSparseProperties;

#define VK_MAX_PHYSICAL_DEVICE_NAME_SIZE  256
#define VK_UUID_SIZE                      16

typedef struct {
    u32                                  api_version;
    u32                                  driver_version;
    u32                                  vendor_id;
    u32                                  device_id;
    vk_PhysicalDeviceType                device_type;
    u8                                   device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    u8                                   pipeline_cache_uuid[VK_UUID_SIZE];
    vk_PhysicalDeviceLimits              limits;
    vk_PhysicalDeviceSparseProperties    sparse_Properties;
} vk_PhysicalDeviceProperties;

static u32
vk_make_version(u32 major, u32 minor) {
    return (major << 22) | (minor << 12);
}

vk_Result
vkCreateInstance(
    const vk_InstanceCreateInfo*                 create_info,
    /* const vk_AllocationCallbacks* */ void*    allocator, 
    vk_Instance*                                 instance
);


static vk_Result
vk_create_instance(
    const vk_InstanceCreateInfo*                 create_info,
    /* const vk_AllocationCallbacks* */ void*    allocator, 
    vk_Instance*                                 instance
) {
    return vkCreateInstance(create_info, allocator, instance);
}

vk_Result
vkEnumeratePhysicalDevices(
    vk_Instance                                  instance,
    u32*                                   pPhysicalDeviceCount,
    vk_PhysicalDevice*                           pPhysicalDevices
);

static vk_Result
vk_enumerate_physical_devices(
    vk_Instance                                  instance,
    u32*                                   pPhysicalDeviceCount,
    vk_PhysicalDevice*                           pPhysicalDevices
) {
    return vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

void
// linkname
vkGetPhysicalDeviceProperties(
    vk_PhysicalDevice                            physicalDevice,
    vk_PhysicalDeviceProperties*                 pProperties
);

static void
vk_get_physical_device_properties(
    vk_PhysicalDevice                            physicalDevice,
    vk_PhysicalDeviceProperties*                 pProperties
) {
    vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}


#define VK_KHR_SURFACE_EXTENSION_NAME     "VK_KHR_surface"
