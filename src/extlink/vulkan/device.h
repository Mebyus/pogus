#define VK_KHR_SWAPCHAIN_EXTENSION_NAME "VK_KHR_swapchain"

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


