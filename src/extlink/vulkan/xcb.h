#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"

typedef struct {
    vk_StructureType type;
    const void*      next;
    u32              flags;
    xcb_Connection*  connection;
    xcb_Window       window;
} vk_XcbSurfaceCreateInfoKHR;

vk_Result
vkCreateXcbSurfaceKHR(
    vk_Instance                                  instance,
    const vk_XcbSurfaceCreateInfoKHR*            create_info,
    /* const vk_AllocationCallbacks* */ void*    allocator,
    vk_SurfaceKHR*                               surface
);

static vk_Result
vk_create_xcb_surface_khr(
    vk_Instance                                  instance,
    const vk_XcbSurfaceCreateInfoKHR*            create_info,
    /* const vk_AllocationCallbacks* */ void*    allocator,
    vk_SurfaceKHR*                               surface
) {
    return vkCreateXcbSurfaceKHR(instance, create_info, allocator, surface);
}
