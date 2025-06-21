
static void
log_vulkan_error(Logger *lg, str s, vk_Result r) {
    log_error_field(lg, s, log_field_s64(ss("vk_result"), r));
}

#define VULKAN_MAX_PHYSICAL_DEVICES      2
#define VULKAN_MAX_DEVICE_EXTENSIONS     256
#define VULKAN_MAX_DEVICE_QUEUE_FAMILIES 16
#define VULKAN_MAX_SURFACE_FORMATS       16
#define VULKAN_MAX_SURFACE_PRESENT_MODES 16

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
vulkan_create_logical_device(EngineHarness* h) {
    f32 queue_priority = 1.0f;

    vk_DeviceQueueCreateInfo queue_create_infos[1] = {};

    queue_create_infos[0].type = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queue_family_index = h->vk.graphics_queue_family;
    queue_create_infos[0].queue_count = 1;
    queue_create_infos[0].queue_priorities = &queue_priority;

    // TODO: optional present queue?

    // Create logical device from physical device
    // Note: there are separate instance and device extensions!
    vk_DeviceCreateInfo device_create_info = {};
    device_create_info.type = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queue_create_infos = queue_create_infos;
    device_create_info.queue_create_info_count = 1;

    // Necessary for shader (for some reason)
    vk_PhysicalDeviceFeatures enabled_features = {};
    enabled_features.shader_clip_distance = 1;
    enabled_features.shader_cull_distance = 1;

    str swapchain_extension_name = ss("VK_KHR_swapchain");
    const u8* device_extensions[] = {swapchain_extension_name.ptr};
    device_create_info.enabled_extension_count = array_len(device_extensions);
    device_create_info.enabled_extension_names = device_extensions;
    device_create_info.enabled_features = &enabled_features;

    // if (ENABLE_DEBUGGING) {
    //     device_create_info.enabledLayerCount = 1;
    //     device_create_info.ppEnabledLayerNames = &DEBUG_LAYER;
    // }

    vk_Result r = vk_create_device(h->vk.physical_device, &device_create_info, nil, &h->vk.device);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan logical device"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // Get graphics and presentation queues (which may be the same)
    vk_get_device_queue(h->vk.device, h->vk.graphics_queue_family, 0, &h->vk.graphics_queue);

    vk_get_physical_device_memory_properties(h->vk.physical_device, &h->vk.memory_properties);
}

static void
vulkan_create_command_pool(EngineHarness* h) {
    vk_CommandPoolCreateInfo pool_create_info = {};
    pool_create_info.type = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queue_family_index = h->vk.graphics_queue_family;

    vk_Result r = vk_create_command_pool(h->vk.device, &pool_create_info, nil, &h->vk.command_pool);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan command pool"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
}

static void
vulkan_create_semaphores(EngineHarness* h) {
    vk_SemaphoreCreateInfo create_info = {};
    create_info.type = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vk_Result r = vk_create_semaphore(h->vk.device, &create_info, nil, &h->vk.image_available_semaphore);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan image semaphore"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    r = vk_create_semaphore(h->vk.device, &create_info, nil, &h->vk.rendering_finished_semaphore);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan render semaphore"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
}

typedef struct {
    // Valid if {code} equals 0. 
    u32 index;

    ErrorCode code;
} RetDeviceMemoryType;

// Find device memory that is supported by the requirements (typeBits) and meets the desired properties.
static RetDeviceMemoryType
vulkan_find_device_memory_type(vk_PhysicalDeviceMemoryProperties* p, u32 type_bits, u32 property_flags) {
    RetDeviceMemoryType ret = {};

    for (u32 i = 0; i < p->memory_type_count; i += 1) {
        if ((type_bits & 1) == 1) {
            if ((p->memory_types[i].property_flags & property_flags) == property_flags) {
                ret.index = i;
                return ret;
            }
        }
        type_bits >>= 1;
    }

    ret.code = 1;
    return ret;
}

typedef struct {
    vk_DeviceMemory memory;
    vk_Buffer       buffer;
} StagingBuffer;

typedef	struct {
    StagingBuffer vertices;
    StagingBuffer indices;
} StagingBuffers;

static void
vulkan_create_vertex_buffer(EngineHarness* h) {
    f32 vertices[6][3] = {
        { -0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f, 0.0f },
        { -0.5f,  0.5f,  0.0f }, { 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f,  0.0f }, { 0.0f, 0.0f, 1.0f }
    };
    u32 vertices_size = cast(u32, sizeof(vertices));

    u32 indices[] = { 0, 1, 2 };
    u32 indices_size = cast(u32, sizeof(indices));

    void* data;

    // Allocate command buffer for copy operation
    vk_CommandBufferAllocateInfo command_buffer_info = {};
    command_buffer_info.type = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.command_pool = h->vk.command_pool;
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.command_buffer_count = 1;

    vk_CommandBuffer copy_command_buffer;
    vk_Result r = vk_allocate_command_buffers(h->vk.device, &command_buffer_info, &copy_command_buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("allocate vulkan command buffers"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // First copy vertices to host accessible vertex buffer memory
    vk_BufferCreateInfo vertex_buffer_info = {};
    vertex_buffer_info.type = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_info.size = vertices_size;
    vertex_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	StagingBuffers staging_buffers;
    r = vk_create_buffer(h->vk.device, &vertex_buffer_info, nil, &staging_buffers.vertices.buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan vertices buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_MemoryRequirements memory_requirements;
    vk_get_buffer_memory_requirements(h->vk.device, staging_buffers.vertices.buffer, &memory_requirements);
    
    vk_MemoryAllocateInfo mem_allocate_info = {};
    mem_allocate_info.type = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_allocate_info.allocation_size = memory_requirements.size;
		
    RetDeviceMemoryType ret_mem = vulkan_find_device_memory_type(&h->vk.memory_properties, memory_requirements.memory_type_bits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (ret_mem.code != 0) {
        log_error_field(&h->lg, ss("find suitable device memory type"), log_field_u64(ss("type"), memory_requirements.memory_type_bits));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    mem_allocate_info.memory_type_index = ret_mem.index;

    r =	vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &staging_buffers.vertices.memory);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("allocate memory for vertices buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    r = vk_map_memory(h->vk.device, staging_buffers.vertices.memory, 0, vertices_size, 0, &data);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("map vertices data from memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    unsafe_copy(data, cast(void*, vertices), vertices_size);
    vk_unmap_memory(h->vk.device, staging_buffers.vertices.memory);

    r = vk_bind_buffer_memory(h->vk.device, staging_buffers.vertices.buffer, staging_buffers.vertices.memory, 0);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("bind vertices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // Then allocate a gpu only buffer for vertices
    vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    r = vk_create_buffer(h->vk.device, &vertex_buffer_info, nil, &h->vk.vertex_buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create gpu-only vertices buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_get_buffer_memory_requirements(h->vk.device, h->vk.vertex_buffer, &memory_requirements);
		
    mem_allocate_info.allocation_size = memory_requirements.size;
    ret_mem = vulkan_find_device_memory_type(&h->vk.memory_properties, memory_requirements.memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (ret_mem.code != 0) {
        log_error_field(&h->lg, ss("find (2) suitable device memory type"), log_field_u64(ss("type"), memory_requirements.memory_type_bits));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    mem_allocate_info.memory_type_index = ret_mem.index;
	
    r = vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &h->vk.vertex_buffer_memory);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("allocate (2) vertices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    r = vk_bind_buffer_memory(h->vk.device, h->vk.vertex_buffer, h->vk.vertex_buffer_memory, 0);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("bind (2) vertices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // Next copy indices to host accessible index buffer memory
    vk_BufferCreateInfo index_buffer_info = {};
    index_buffer_info.type = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    index_buffer_info.size = indices_size;
    index_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    r = vk_create_buffer(h->vk.device, &index_buffer_info, nil, &staging_buffers.indices.buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create index buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
		
    vk_get_buffer_memory_requirements(h->vk.device, staging_buffers.indices.buffer, &memory_requirements);

    mem_allocate_info.allocation_size = memory_requirements.size;	
    ret_mem = vulkan_find_device_memory_type(&h->vk.memory_properties, memory_requirements.memory_type_bits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (ret_mem.code != 0) {
        log_error_field(&h->lg, ss("find (3) suitable device memory type"), log_field_u64(ss("type"), memory_requirements.memory_type_bits));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    mem_allocate_info.memory_type_index = ret_mem.index;
		
    r = vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &staging_buffers.indices.memory);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("allocate indices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    r = vk_map_memory(h->vk.device, staging_buffers.indices.memory, 0, indices_size, 0, &data);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("map indices data from memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    unsafe_copy(data, cast(void*, indices), indices_size);
    vk_unmap_memory(h->vk.device, staging_buffers.indices.memory);

    r = vk_bind_buffer_memory(h->vk.device, staging_buffers.indices.buffer, staging_buffers.indices.memory, 0);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("bind indices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // And allocate another gpu only buffer for indices
    index_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    r = vk_create_buffer(h->vk.device, &index_buffer_info, nil, &h->vk.index_buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create gpu-only index buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_get_buffer_memory_requirements(h->vk.device, h->vk.index_buffer, &memory_requirements);
		
    mem_allocate_info.allocation_size = memory_requirements.size;
	ret_mem = vulkan_find_device_memory_type(&h->vk.memory_properties, memory_requirements.memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (ret_mem.code != 0) {
        log_error_field(&h->lg, ss("find (4) suitable device memory type"), log_field_u64(ss("type"), memory_requirements.memory_type_bits));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    mem_allocate_info.memory_type_index = ret_mem.index;
		
    r = vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &h->vk.index_buffer_memory);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("allocate indices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    r = vk_bind_buffer_memory(h->vk.device, h->vk.index_buffer, h->vk.index_buffer_memory, 0);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("bind (2) indices buffer memory"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    // Now copy data from host visible buffer to gpu only buffer
    vk_CommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.type = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    r = vk_begin_command_buffer(copy_command_buffer, &buffer_begin_info);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("begin command buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_BufferCopy copy_region = {};
    copy_region.size = vertices_size;
    vk_command_copy_buffer(copy_command_buffer, staging_buffers.vertices.buffer, h->vk.vertex_buffer, 1, &copy_region);
		
    copy_region.size = indices_size;
    vk_command_copy_buffer(copy_command_buffer, staging_buffers.indices.buffer, h->vk.index_buffer, 1, &copy_region);

	r = vk_end_command_buffer(copy_command_buffer);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("end command buffer"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_SubmitInfo submit_info = {};
    submit_info.type = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.command_buffer_count = 1;
    submit_info.command_buffers = &copy_command_buffer;

    r = vk_queue_submit(h->vk.graphics_queue, 1, &submit_info, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("submit queue"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    vk_queue_wait_idle(h->vk.graphics_queue);

    vk_free_command_buffers(h->vk.device, h->vk.command_pool, 1, &copy_command_buffer);
    vk_destroy_buffer(h->vk.device, staging_buffers.vertices.buffer, nil);
    vk_free_memory(h->vk.device, staging_buffers.vertices.memory, nil);
    vk_destroy_buffer(h->vk.device, staging_buffers.indices.buffer, nil);
    vk_free_memory(h->vk.device, staging_buffers.indices.memory, nil);


    h->vk.vertex_binding_description.binding = 0;
    h->vk.vertex_binding_description.stride = sizeof(vertices[0]);
    h->vk.vertex_binding_description.input_rate = VK_VERTEX_INPUT_RATE_VERTEX;

    // vec2 position
    h->vk.vertex_attribute_descriptions[0].binding = 0;
    h->vk.vertex_attribute_descriptions[0].location = 0;
    h->vk.vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        
    // vec3 color
    h->vk.vertex_attribute_descriptions[1].binding = 0;
    h->vk.vertex_attribute_descriptions[1].location = 1;
    h->vk.vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    h->vk.vertex_attribute_descriptions[1].offset = sizeof(f32) * 3;
}

static vk_Extent2D
vulkan_select_swapchain_extent(vk_SurfaceCapabilitiesKHR *c) {
    return c->current_extent;
}

static vk_SurfaceFormatKHR
vulkan_select_surface_format(vk_SurfaceFormatKHR* formats, uint len) {
    must(len != 0);

    vk_SurfaceFormatKHR ret = {};

    if (len == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        ret.format = VK_FORMAT_R8G8B8A8_UNORM;
        ret.color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        return ret;
    }

    for (uint i = 0; i < len; i += 1) {
        vk_SurfaceFormatKHR f = formats[i];

        if (f.format == VK_FORMAT_R8G8B8A8_UNORM) {
            return f;
        }
    }

    return formats[0];
}

static vk_PresentModeKHR
vulkan_select_present_mode(vk_PresentModeKHR* modes, uint len) {
    for (uint i = 0; i < len; i += 1) {
        if (modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        }
    }

    // If mailbox is unavailable, fall back to FIFO (guaranteed to be available)
    return VK_PRESENT_MODE_FIFO_KHR;
}

static void
vulkan_create_swapchain(EngineHarness* h) {
    vk_SurfaceCapabilitiesKHR surface_capabilities;
	vk_Result r = vk_get_physical_device_surface_capabilities_khr(h->vk.physical_device, h->vk.surface, &surface_capabilities);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get surface capabilities"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_debug_field(&h->lg, ss("surface current extent"), log_field_u64(ss("width"), surface_capabilities.current_extent.width));
    log_debug_field(&h->lg, ss("surface current extent"), log_field_u64(ss("height"), surface_capabilities.current_extent.height));

    u32 image_count = surface_capabilities.min_image_count + 1;
    if (surface_capabilities.max_image_count != 0 && image_count > surface_capabilities.max_image_count) {
        image_count = surface_capabilities.max_image_count;
    }
    log_info_field(&h->lg, ss("determine number of images for swapchain"), log_field_u64(ss("count"), image_count));

    // Find supported surface formats
    u32 format_count;
    r = vk_get_physical_device_surface_formats_khr(h->vk.physical_device, h->vk.surface, &format_count, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get supported surface formats count"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    if (format_count == 0) {
        log_error(&h->lg, ss("no supported surface formats"));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_info_field(&h->lg, ss("get supported surface formats"), log_field_u64(ss("count"), format_count));

    vk_SurfaceFormatKHR surface_formats[VULKAN_MAX_SURFACE_FORMATS];
    format_count = min_u32(format_count, VULKAN_MAX_SURFACE_FORMATS);
    r = vk_get_physical_device_surface_formats_khr(h->vk.physical_device, h->vk.surface, &format_count, surface_formats);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get supported surface formats"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    for (uint i = 0; i < format_count; i += 1) {
        vk_SurfaceFormatKHR f = surface_formats[i];

        log_debug_field(&h->lg, ss("supported surface format"), log_field_u64(ss("index"), i));
        log_debug_field(&h->lg, ss("supported surface format"), log_field_u64(ss("format"), f.format));
        log_debug_field(&h->lg, ss("supported surface format"), log_field_u64(ss("color_space"), f.color_space));
    }

    u32 present_mode_count;
    r = vk_get_physical_device_surface_present_modes_khr(h->vk.physical_device, h->vk.surface, &present_mode_count, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get supported surface present modes count"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    if (present_mode_count == 0) {
        log_error(&h->lg, ss("no supported surface present modes"));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_info_field(&h->lg, ss("get supported surface present modes"), log_field_u64(ss("count"), present_mode_count));

    vk_PresentModeKHR present_modes[VULKAN_MAX_SURFACE_PRESENT_MODES];
    present_mode_count = min_u32(present_mode_count, VULKAN_MAX_SURFACE_PRESENT_MODES);
    r = vk_get_physical_device_surface_present_modes_khr(h->vk.physical_device, h->vk.surface, &present_mode_count, present_modes);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get supported surface present modes"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    for (uint i = 0; i < present_mode_count; i += 1) {
        log_debug_field(&h->lg, ss("supported surface present mode"), log_field_u64(ss("mode"), present_modes[i]));
    }

    h->vk.swapchain_extent = vulkan_select_swapchain_extent(&surface_capabilities);
    vk_SurfaceFormatKHR surface_format = vulkan_select_surface_format(surface_formats, format_count);
    vk_PresentModeKHR present_mode = vulkan_select_present_mode(present_modes, present_mode_count);

    // Determine transformation to use (preferring no transform)
    vk_SurfaceTransformFlagBitsKHR surface_transform;
    if ((surface_capabilities.supported_transforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0) {
        surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        surface_transform = surface_capabilities.current_transform;
    }

    h->vk.old_swapchain = h->vk.swapchain;

    vk_SwapchainCreateInfoKHR create_info = {};
    create_info.type = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = h->vk.surface;
    create_info.min_image_count = image_count;
    create_info.image_format = surface_format.format;
    create_info.image_color_space = surface_format.color_space;
    create_info.image_extent = h->vk.swapchain_extent;
    create_info.image_array_layers = 1;
    create_info.image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queue_family_index_count = 0;
    create_info.queue_family_indices = nil;
    create_info.pre_transform = surface_transform;
    create_info.composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.present_mode = present_mode;
    create_info.clipped = true;
    create_info.old_swapchain = h->vk.old_swapchain;

    r = vk_create_swapchain_khr(h->vk.device, &create_info, nil, &h->vk.swapchain);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create new swapchain"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    if (h->vk.old_swapchain != nil) {
        vk_destroy_swapchain_khr(h->vk.device, h->vk.old_swapchain, nil);
    }

    // Store the images used by the swap chain
    // Note: these are the images that swap chain image indices refer to
    // Note: actual number of images may differ from requested number, since it's a lower bound
    u32 swapchain_image_count = 0;
    r = vk_get_swapchain_images_khr(h->vk.device, h->vk.swapchain, &swapchain_image_count, nil);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("get swapchain image count"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    if (swapchain_image_count == 0) {
        log_vulkan_error(&h->lg, ss("no swapchain images"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }
    log_debug_field(&h->lg, ss("get swapchain image count"), log_field_u64(ss("count"), swapchain_image_count));
}

static vk_ShaderModule
vulkan_create_shader_module(EngineHarness* h, str path) {
    MemAllocator al = imake_mem_bump_allocator(&proc_mem_bump_allocator);
    MemBlob blob;
    ErrorCode c = os_load_file(al, path, &blob);
    if (c != 0) {
        log_error_field(&h->lg, ss("load shader code"), log_field_u64(ss("error"), c));
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return nil;
    }

    span_u8 data = mem_blob_get_data(blob);

    vk_ShaderModuleCreateInfo create_info = {};
    create_info.type = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.code_size = data.len;
    create_info.code = cast(u32*, data.ptr);

    vk_ShaderModule shader_module;
    vk_Result r = vk_create_shader_module(h->vk.device, &create_info, nil, &shader_module);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create shader module"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return nil;
    }

    log_info_field(&h->lg, ss("create shader module"), log_field_str(ss("path"), path));
    return shader_module;
}

static void
init_renderer(EngineHarness* h) {
    log_debug(&h->lg, ss("init vulkan renderer"));
    vulkan_create_instance(h);
    if (h->exit) {
        return;
    }

    log_debug(&h->lg, ss("select physical device"));
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

    vulkan_create_logical_device(h);
    if (h->exit) {
        return;
    }

    vulkan_create_semaphores(h);
    if (h->exit) {
        return;
    }

    vulkan_create_command_pool(h);
    if (h->exit) {
        return;
    }

    vulkan_create_vertex_buffer(h);
    if (h->exit) {
        return;
    }

    vulkan_create_swapchain(h);
    if (h->exit) {
        return;
    }

    log_debug(&h->lg, ss("vulkan init successfully finished"));
}
