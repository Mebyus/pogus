
static void
log_vulkan_error(Logger *lg, str s, vk_Result r) {
    log_error_field(lg, s, log_field_s64(ss("vk_result"), r));
}

#define VULKAN_MAX_PHYSICAL_DEVICES      2
#define VULKAN_MAX_DEVICE_EXTENSIONS     256
#define VULKAN_MAX_DEVICE_QUEUE_FAMILIES 16

static void
vulkan_create_instance(EngineHarness *h) {
    c_string app_name = ss("Pogus Test Game");
    c_string engine_name = ss("Pogus Engine");

    vk_ApplicationInfo app_info = {};
    app_info.type = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.api_version = vk_make_version(1, 4);
    app_info.app_name = app_name.ptr;
    app_info.app_version = vk_make_version(1, 0);
    app_info.engine_name = engine_name.ptr;
    app_info.engine_version = vk_make_version(1, 0);

    vk_InstanceCreateInfo create_info = {};
    create_info.type = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.app_info = &app_info;

    c_string surface_extension_name = ss("VK_KHR_surface");
    c_string xcb_surface_extension_name = ss("VK_KHR_xcb_surface");
    const u8* extensions[] = {
        surface_extension_name.ptr,
        xcb_surface_extension_name.ptr,
    };

    create_info.ext_count = 2;
    create_info.ext_names = extensions;

    // TODO: supply custom allocator
    vk_Result r = vk_create_instance(&create_info, nil, &h->vk.instance);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan instance"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
}

static void
vulkan_select_physical_device(EngineHarness* h) {
    u32 physical_device_count;
    vk_Result r = vk_enumerate_physical_devices(h->vk.instance, &physical_device_count, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("enumerate vulkan physical devices"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_info_field(&h->lg, ss("enumerate vulkan physical devices"), log_field_u64(ss("count"), physical_device_count));

    // TODO: log warning if VULKAN_MAX_PHYSICAL_DEVICES is too small
    physical_device_count = min_u32(physical_device_count, VULKAN_MAX_PHYSICAL_DEVICES);
    vk_PhysicalDevice physical_devices[VULKAN_MAX_PHYSICAL_DEVICES];
    r = vk_enumerate_physical_devices(h->vk.instance, &physical_device_count, physical_devices);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("load vulkan physical devices"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    for (u32 i = 0; i < physical_device_count; i += 1) {
        vk_PhysicalDeviceProperties properties;
        vk_get_physical_device_properties(physical_devices[i], &properties);

        c_string name = unsafe_make_c_string_from_ptr(properties.device_name);

        log_debug_field(&h->lg, ss("inspect vulkan physical device"), log_field_str(ss("name"), name));
    }

    // TODO: algorithm for selecting appropriate device
    h->vk.physical_device = physical_devices[0];
}

static void
vulkan_check_swapchain_support(EngineHarness* h) {
    u32 device_extension_count;
    vk_Result r = vk_enumerate_device_extension_properties(h->vk.physical_device, nil, &device_extension_count, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("enumerate vulkan physical device extensions"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_info_field(&h->lg, ss("enumerate vulkan physical device extensions"), log_field_u64(ss("count"), device_extension_count));

    // TODO: log warning if VULKAN_MAX_DEVICE_EXTENSIONS is too small
    device_extension_count = min_u32(device_extension_count, VULKAN_MAX_DEVICE_EXTENSIONS);
    vk_ExtensionProperties extension_properties[VULKAN_MAX_DEVICE_EXTENSIONS];
    r = vk_enumerate_device_extension_properties(h->vk.physical_device, nil, &device_extension_count, extension_properties);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("load vulkan physical device extensions"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    bool found_swapchain_extension = false;
    str swapchain_extension_name = ss("VK_KHR_swapchain");
    for (u32 i = 0; i < device_extension_count; i += 1) {
        str name = unsafe_make_c_string_from_ptr(extension_properties[i].ext_name);
        if (str_equal(name, swapchain_extension_name)) {
            found_swapchain_extension = true;
        }
    }
    if (!found_swapchain_extension) {
        log_vulkan_error(&h->lg, ss("swapchain device extension not found"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
}

static void
vulkan_select_queue_families(EngineHarness* h) {
    u32 queue_family_count;
    vk_get_physical_device_queue_family_properties(h->vk.physical_device, &queue_family_count, nil);
    if (queue_family_count == 0) {
        log_error(&h->lg, ss("physical device has no queue families"));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_info_field(&h->lg, ss("enumerate vulkan physical device queue families"), log_field_u64(ss("count"), queue_family_count));

    // Find queue family with graphics support
    // Note: is a transfer queue necessary to copy vertices to the gpu or can a graphics queue handle that?
    // TODO: log warning if VULKAN_MAX_DEVICE_QUEUE_FAMILIES is too small
    queue_family_count = min_u32(queue_family_count, VULKAN_MAX_DEVICE_QUEUE_FAMILIES);
    vk_QueueFamilyProperties queue_families[VULKAN_MAX_DEVICE_QUEUE_FAMILIES];
    vk_get_physical_device_queue_family_properties(h->vk.physical_device, &queue_family_count, queue_families);
    for (u32 i = 0; i < queue_family_count; i += 1) {
        if (queue_families[i].queue_count == 0) {
            continue;
        }

        if ((queue_families[i].queue_flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            h->vk.graphics_queue_family = i;
            log_info_field(&h->lg, ss("found graphics queue family"), log_field_u64(ss("family"), i));
            return;
        }
    }
    
    log_error(&h->lg, ss("no graphics queue families found"));
    engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
}

static void
vulkan_create_surface(EngineHarness* h) {
    vk_XcbSurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.type = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_create_info.connection = h->connection;
    surface_create_info.window = h->window;

    vk_Result r = vk_create_xcb_surface_khr(h->vk.instance, &surface_create_info, nil, &h->vk.surface);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan surface"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
}

static void
init_renderer(EngineHarness* h) {
    vulkan_create_instance(h);
    if (h->exit) {
        return;
    }

    vulkan_select_physical_device(h);
    if (h->exit) {
        return;
    }

    vulkan_check_swapchain_support(h);
    if (h->exit) {
        return;
    }

    vulkan_select_queue_families(h);
    if (h->exit) {
        return;
    }

    vulkan_create_surface(h);
    if (h->exit) {
        return;
    }

    log_debug(&h->lg, ss("vulkan init successfully finished"));
}
