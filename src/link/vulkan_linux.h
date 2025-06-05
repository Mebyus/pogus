typedef s32 vk_Result;
typedef u32 vk_InstanceCreateFlags;

typedef void* vk_Instance;
typedef void* vk_SurfaceKHR;

typedef enum {
    VK_STRUCTURE_TYPE_APPLICATION_INFO = 0,
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1,
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 2,
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3,

    VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR = 1000005000,
} vk_StructureType;

typedef struct {
    vk_StructureType    sType;
    const void*        pNext;
    const u8*          pApplicationName;
    u32           applicationVersion;
    const u8*        pEngineName;
    u32           engineVersion;
    u32           apiVersion;
} vk_ApplicationInfo;

typedef struct {
    vk_StructureType            sType;
    const void*                 pNext;
    vk_InstanceCreateFlags      flags;
    const vk_ApplicationInfo*   pApplicationInfo;
    u32                    enabledLayerCount;
    const u8* const*            ppEnabledLayerNames;
    u32                    enabledExtensionCount;
    const u8* const*            ppEnabledExtensionNames;
} vk_InstanceCreateInfo;

// typedef void* (VKAPI_PTR *PFN_vkAllocationFunction)(
//     void*                                       pUserData,
//     size_t                                      size,
//     size_t                                      alignment,
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

static u32
vk_make_version(u32 major, u32 minor) {
    return (major << 22) | (minor << 12);
}

vk_Result
vkCreateInstance(
    const vk_InstanceCreateInfo*                 pCreateInfo,
    /* const vk_AllocationCallbacks* */ void*    pAllocator, 
    vk_Instance*                                 pInstance
);


static vk_Result
vk_create_instance(
    const vk_InstanceCreateInfo*                 pCreateInfo,
    /* const vk_AllocationCallbacks* */ void*    pAllocator, 
    vk_Instance*                                 pInstance
) {
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

#define VK_KHR_SURFACE_EXTENSION_NAME     "VK_KHR_surface"

/* xcb integration */

#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"

typedef struct {
    vk_StructureType              sType;
    const void*                   pNext;
    u32                           flags;
    xcb_Connection*             connection;
    xcb_Window                  window;
} vk_XcbSurfaceCreateInfoKHR;

vk_Result
vkCreateXcbSurfaceKHR(
    vk_Instance                                  instance,
    const vk_XcbSurfaceCreateInfoKHR*            pCreateInfo,
    /* const vk_AllocationCallbacks* */ void*    pAllocator,
    vk_SurfaceKHR*                               pSurface
);

static vk_Result
vk_create_xcb_surface_khr(
    vk_Instance                                  instance,
    const vk_XcbSurfaceCreateInfoKHR*            pCreateInfo,
    /* const vk_AllocationCallbacks* */ void*    pAllocator,
    vk_SurfaceKHR*                               pSurface
) {
    return vkCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
