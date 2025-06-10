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
);

VkResult // linkname
vkAllocateMemory(
    vk_Device                                   device,
    const vk_MemoryAllocateInfo*                allocate_info,
    /* const VkAllocationCallbacks* */ void*    allocator,
    vk_DeviceMemory*                            memory
);


static VkResult
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

