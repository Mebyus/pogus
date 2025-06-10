typedef enum {
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
    VK_MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
    VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
    VK_MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
    VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 0x00000040,
    VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 0x00000080,
    VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 0x00000100,
    VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} vk_MemoryPropertyFlagBits;

typedef enum {
    VK_MEMORY_MAP_PLACED_BIT_EXT = 0x00000001,
    VK_MEMORY_MAP_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} vk_MemoryMapFlagBits;

typedef struct {
    vk_StructureType type;
    const void*      next;
    vk_DeviceSize    allocation_size;
    u32              memory_type_index;
} vk_MemoryAllocateInfo;

typedef struct {
    vk_DeviceSize    size;
    vk_DeviceSize    alignment;
    u32              memory_type_bits;
} vk_MemoryRequirements;

typedef struct {
    u32  property_flags;
    u32  heap_index;
} vk_MemoryType;

typedef struct {
    vk_DeviceSize  size;
    u32            flags;
} vk_MemoryHeap;

#define VK_MAX_MEMORY_TYPES 32
#define VK_MAX_MEMORY_HEAPS 16

typedef struct {
    u32              memory_type_count;
    vk_MemoryType    memory_types[VK_MAX_MEMORY_TYPES];
    u32              memory_heap_count;
    vk_MemoryHeap    memory_heaps[VK_MAX_MEMORY_HEAPS];
} vk_PhysicalDeviceMemoryProperties;

typedef struct {
    vk_StructureType  type;
    const void*       next;
    u32               flags;
} vk_SemaphoreCreateInfo;

typedef struct {
    vk_DeviceSize   source_offset;
    vk_DeviceSize   destination_offset;
    vk_DeviceSize   size;
} vk_BufferCopy;

void // linkname
vkGetPhysicalDeviceMemoryProperties(
    vk_PhysicalDevice                    physical_device,
    vk_PhysicalDeviceMemoryProperties*   memory_properties
);

static void
vk_get_physical_device_memory_properties(
    vk_PhysicalDevice                    physical_device,
    vk_PhysicalDeviceMemoryProperties*   memory_properties
) {
    vkGetPhysicalDeviceMemoryProperties(physical_device, memory_properties);
}

vk_Result // linkname
vkCreateSemaphore(
    vk_Device                                    device,
    const vk_SemaphoreCreateInfo*                create_info,
    /* const VkAllocationCallbacks* */ void*     allocator,
    vk_Semaphore*                                semaphore
);

static vk_Result
vk_create_semaphore(
    vk_Device                                    device,
    const vk_SemaphoreCreateInfo*                create_info,
    /* const VkAllocationCallbacks* */ void*     allocator,
    vk_Semaphore*                                semaphore
) {
    return vkCreateSemaphore(device, create_info, allocator, semaphore);
}

void // linkname
vkGetBufferMemoryRequirements(
    vk_Device                                    device,
    vk_Buffer                                    buffer,
    vk_MemoryRequirements*                       memory_requirements
);

static void
vk_get_buffer_memory_requirements(
    vk_Device                                    device,
    vk_Buffer                                    buffer,
    vk_MemoryRequirements*                       memory_requirements
) {
    vkGetBufferMemoryRequirements(device, buffer, memory_requirements);
}

vk_Result // linkname
vkAllocateMemory(
    vk_Device                                   device,
    const vk_MemoryAllocateInfo*                allocate_info,
    /* const VkAllocationCallbacks* */ void*    allocator,
    vk_DeviceMemory*                            memory
);

static vk_Result
vk_allocate_memory(
    vk_Device                                   device,
    const vk_MemoryAllocateInfo*                allocate_info,
    /* const VkAllocationCallbacks* */ void*    allocator,
    vk_DeviceMemory*                            memory
) {
    return vkAllocateMemory(device, allocate_info, allocator, memory);
}

void // linkname
vkFreeMemory(
    vk_Device                                 device,
    vk_DeviceMemory                           memory,
    /* const VkAllocationCallbacks* */ void*  allocator
);

static void
vk_free_memory(
    vk_Device                                 device,
    vk_DeviceMemory                           memory,
    /* const VkAllocationCallbacks* */ void*  allocator
) {
    vkFreeMemory(device, memory, allocator);
}

vk_Result // linkname
vkMapMemory(
    vk_Device          device,
    vk_DeviceMemory    memory,
    vk_DeviceSize      offset,
    vk_DeviceSize      size,
    u32                flags,
    void**             data
);

static vk_Result
vk_map_memory(
    vk_Device          device,
    vk_DeviceMemory    memory,
    vk_DeviceSize      offset,
    vk_DeviceSize      size,
    u32                flags,
    void**             data   // out
) {
    return vkMapMemory(device, memory, offset, size, flags, data);
}

void // linkname
vkUnmapMemory(vk_Device device, vk_DeviceMemory memory);

static void
vk_unmap_memory(vk_Device device, vk_DeviceMemory memory) {
    vkUnmapMemory(device, memory);
}
