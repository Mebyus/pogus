
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

    // std::cout << "acquired graphics and presentation queues" << std::endl;

    // vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
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

    for (u32 i = 0; i < p.memory_type_count; i += 1) {
        if ((type_bits & 1) == 1) {
            if ((p.memory_types[i].property_flags & property_flags) == property_flags) {
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
    		// Setup vertices
		std::vector<Vertex> vertices = {
			{ { -0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f,  0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
		uint32_t verticesSize = (uint32_t) (vertices.size() * sizeof(vertices[0]));

		// Setup indices
		std::vector<uint32_t> indices = { 0, 1, 2 };
		uint32_t indicesSize = (uint32_t) (indices.size() * sizeof(indices[0]));

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
    vertex_buffer_info.size = verticesSize;
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
        log_error_field(&h->lg, ss("find suitable device memory type"), log_field_u64(s("type"), memory_requirements.memory_type_bits));
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

		vkMapMemory(h->vk.device, staging_buffers.vertices.memory, 0, verticesSize, 0, &data);
		memcpy(data, vertices.data(), verticesSize);
		vkUnmapMemory(h->vk.device, staging_buffers.vertices.memory);
		vkBindBufferMemory(h->vk.device, staging_buffers.vertices.buffer, staging_buffers.vertices.memory, 0);

		// Then allocate a gpu only buffer for vertices
		vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vkCreateBuffer(h->vk.device, &vertex_buffer_info, nil, &vertexBuffer);
		vk_get_buffer_memory_requirements(h->vk.device, vertexBuffer, &memory_requirements);
		mem_allocate_info.allocationSize = memory_requirements.size;
		getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_allocate_info.memoryTypeIndex);
		vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &vertexBufferMemory);
		vkBindBufferMemory(h->vk.device, vertexBuffer, vertexBufferMemory, 0);

		// Next copy indices to host accessible index buffer memory
		vk_BufferCreateInfo index_buffer_info = {};
		index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		index_buffer_info.size = indicesSize;
		index_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		vkCreateBuffer(h->vk.device, &index_buffer_info, nil, &staging_buffers.indices.buffer);
		vk_get_buffer_memory_requirements(h->vk.device, staging_buffers.indices.buffer, &memory_requirements);
		mem_allocate_info.allocationSize = memory_requirements.size;
		getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_allocate_info.memoryTypeIndex);
		vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &staging_buffers.indices.memory);
		vkMapMemory(h->vk.device, staging_buffers.indices.memory, 0, indicesSize, 0, &data);
		memcpy(data, indices.data(), indicesSize);
		vkUnmapMemory(h->vk.device, staging_buffers.indices.memory);
		vkBindBufferMemory(h->vk.device, staging_buffers.indices.buffer, staging_buffers.indices.memory, 0);

		// And allocate another gpu only buffer for indices
		index_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vk_create_buffer(h->vk.device, &index_buffer_info, nil, &indexBuffer);
		vk_get_buffer_memory_requirements(h->vk.device, indexBuffer, &memory_requirements);
		mem_allocate_info.allocationSize = memory_requirements.size;
		getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_allocate_info.memoryTypeIndex);
		vk_allocate_memory(h->vk.device, &mem_allocate_info, nil, &indexBufferMemory);
		vkBindBufferMemory(h->vk.device, indexBuffer, indexBufferMemory, 0);

		// Now copy data from host visible buffer to gpu only buffer
		VkCommandBufferBeginInfo bufferBeginInfo = {};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(copy_command_buffer, &bufferBeginInfo);

		vk_BufferCopy copy_region = {};
		copy_region.size = verticesSize;
		vkCmdCopyBuffer(copy_command_buffer, staging_buffers.vertices.buffer, h->vk.vertex_buffer, 1, &copy_region);
		copy_region.size = indicesSize;
		vkCmdCopyBuffer(copy_command_buffer, staging_buffers.indices.buffer, h->vk.index_buffer, 1, &copy_region);

		vkEndCommandBuffer(copy_command_buffer);

		// Submit to queue
		vk_SubmitInfo submit_info = {};
		submit_info.type = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.command_buffer_count = 1;
		submit_info.command_buffers = &copy_command_buffer;

		vk_queue_submit(h->vk.graphics_queue, 1, &submit_info, nil);
		vkQueueWaitIdle(h->vk.graphics_queue);

		vkFreeCommandBuffers(h->vk.device, h->vk.command_pool, 1, &copy_command_buffer);

		vkDestroyBuffer(h->vk.device, staging_buffers.vertices.buffer, nil);
		vk_free_memory(h->vk.device, staging_buffers.vertices.memory, nil);
		vkDestroyBuffer(h->vk.device, staging_buffers.indices.buffer, nil);
		vk_free_memory(h->vk.device, staging_buffers.indices.memory, nil);

		std::cout << "set up vertex and index buffers" << std::endl;

		// Binding and attribute descriptions
		vertexBindingDescription.binding = 0;
		vertexBindingDescription.stride = sizeof(vertices[0]);
		vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// vec2 position
		vertexAttributeDescriptions.resize(2);
		vertexAttributeDescriptions[0].binding = 0;
		vertexAttributeDescriptions[0].location = 0;
		vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		
		// vec3 color
		vertexAttributeDescriptions[1].binding = 0;
		vertexAttributeDescriptions[1].location = 1;
		vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributeDescriptions[1].offset = sizeof(float) * 3;
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

    vulkan_create_logical_device(h);
    if (h->exit) {
        return;
    }

    vulkan_create_semaphores(h);
    if (h->exit) {
        return;
    }

    vulkan_create_vertex_buffer(h);
    if (h->exit) {
        return;
    }

    log_debug(&h->lg, ss("vulkan init successfully finished"));
}
