#define VK_KHR_SWAPCHAIN_EXTENSION_NAME "VK_KHR_swapchain"

typedef enum {
    VK_QUEUE_GRAPHICS_BIT = 0x00000001,
    VK_QUEUE_COMPUTE_BIT = 0x00000002,
    VK_QUEUE_TRANSFER_BIT = 0x00000004,
    VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
    VK_QUEUE_PROTECTED_BIT = 0x00000010,
    VK_QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,
    VK_QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,
    VK_QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,
    VK_QUEUE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} vk_QueueFlagBits;

typedef struct {
    vk_StructureType type;
    const void*      next;
    u32              flags;
    u32              queue_family_index;
    u32              queue_count;
    const f32*       queue_priorities;
} vk_DeviceQueueCreateInfo;

typedef struct {
    vk_Bool32    robustBufferAccess;
    vk_Bool32    fullDrawIndexUint32;
    vk_Bool32    imageCubeArray;
    vk_Bool32    independentBlend;
    vk_Bool32    geometry_shader;
    vk_Bool32    tessellationShader;
    vk_Bool32    sampleRateShading;
    vk_Bool32    dualSrcBlend;
    vk_Bool32    logicOp;
    vk_Bool32    multiDrawIndirect;
    vk_Bool32    drawIndirectFirstInstance;
    vk_Bool32    depthClamp;
    vk_Bool32    depthBiasClamp;
    vk_Bool32    fillModeNonSolid;
    vk_Bool32    depthBounds;
    vk_Bool32    wideLines;
    vk_Bool32    largePoints;
    vk_Bool32    alphaToOne;
    vk_Bool32    multiViewport;
    vk_Bool32    samplerAnisotropy;
    vk_Bool32    textureCompressionETC2;
    vk_Bool32    textureCompressionASTC_LDR;
    vk_Bool32    textureCompressionBC;
    vk_Bool32    occlusionQueryPrecise;
    vk_Bool32    pipelineStatisticsQuery;
    vk_Bool32    vertexPipelineStoresAndAtomics;
    vk_Bool32    fragmentStoresAndAtomics;
    vk_Bool32    shaderTessellationAndGeometryPointSize;
    vk_Bool32    shaderImageGatherExtended;
    vk_Bool32    shaderStorageImageExtendedFormats;
    vk_Bool32    shaderStorageImageMultisample;
    vk_Bool32    shaderStorageImageReadWithoutFormat;
    vk_Bool32    shaderStorageImageWriteWithoutFormat;
    vk_Bool32    shaderUniformBufferArrayDynamicIndexing;
    vk_Bool32    shaderSampledImageArrayDynamicIndexing;
    vk_Bool32    shaderStorageBufferArrayDynamicIndexing;
    vk_Bool32    shaderStorageImageArrayDynamicIndexing;
    vk_Bool32    shader_clip_distance;
    vk_Bool32    shader_cull_distance;
    vk_Bool32    shader_f64;
    vk_Bool32    shader_s64;
    vk_Bool32    shader_s16;
    vk_Bool32    shaderResourceResidency;
    vk_Bool32    shaderResourceMinLod;
    vk_Bool32    sparseBinding;
    vk_Bool32    sparseResidencyBuffer;
    vk_Bool32    sparseResidencyImage2D;
    vk_Bool32    sparseResidencyImage3D;
    vk_Bool32    sparseResidency2Samples;
    vk_Bool32    sparseResidency4Samples;
    vk_Bool32    sparseResidency8Samples;
    vk_Bool32    sparseResidency16Samples;
    vk_Bool32    sparseResidencyAliased;
    vk_Bool32    variableMultisampleRate;
    vk_Bool32    inheritedQueries;
} vk_PhysicalDeviceFeatures;

typedef struct {
    vk_StructureType                 type;
    const void*                      next;
    u32                              flags;
    u32                              queue_create_info_count;
    const vk_DeviceQueueCreateInfo*  queue_create_infos;
    
    // enabledLayerCount is deprecated and should not be used
    u32 enabled_layer_count;
    // ppEnabledLayerNames is deprecated and should not be used
    const u8* const* enabled_layer_names;
    
    u32                               enabled_extension_count;
    const u8* const*                  enabled_extension_names;
    const vk_PhysicalDeviceFeatures*  enabled_features;
} vk_DeviceCreateInfo;

vk_Result // linkname
vkEnumerateDeviceExtensionProperties(
    vk_PhysicalDevice            physical_device,
    const u8*                    layer_name,
    u32*                         property_count,
    vk_ExtensionProperties*      properties
);

static vk_Result
vk_enumerate_device_extension_properties(
    vk_PhysicalDevice            physical_device,
    const u8*                    layer_name,
    u32*                         property_count,
    vk_ExtensionProperties*      properties
) {
    return vkEnumerateDeviceExtensionProperties(physical_device, layer_name, property_count, properties);
}

typedef struct {
    u32          queue_flags;
    u32          queue_count;
    u32          timestamp_valid_bits;
    vk_Extent3D  min_image_transfer_granularity;
} vk_QueueFamilyProperties;

void // linkname
vkGetPhysicalDeviceQueueFamilyProperties(
    vk_PhysicalDevice                      physical_device,
    u32*                                   queue_family_property_count,
    vk_QueueFamilyProperties*              queue_family_properties
);

static void
vk_get_physical_device_queue_family_properties(
    vk_PhysicalDevice                      physical_device,
    u32*                                   queue_family_property_count,
    vk_QueueFamilyProperties*              queue_family_properties
) {
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, queue_family_property_count, queue_family_properties);
}

vk_Result // linkname
vkCreateDevice(
    vk_PhysicalDevice                           physical_device,
    const vk_DeviceCreateInfo*                  create_info,
    /* const vk_AllocationCallbacks* */ void*   allocator,
    vk_Device*                                  device
);

static vk_Result
vk_create_device(
    vk_PhysicalDevice                           physical_device,
    const vk_DeviceCreateInfo*                  create_info,
    /* const vk_AllocationCallbacks* */ void*   allocator,
    vk_Device*                                  device
) {
    return vkCreateDevice(physical_device, create_info, allocator, device);
}

void // linkname
vkGetDeviceQueue(
    vk_Device                                    device,
    u32                                          queue_family_index,
    u32                                          queue_index,
    vk_Queue*                                    queue
);

static void
vk_get_device_queue(
    vk_Device                                    device,
    u32                                          queue_family_index,
    u32                                          queue_index,
    vk_Queue*                                    queue
) {
    vkGetDeviceQueue(device, queue_family_index, queue_index, queue);
}
