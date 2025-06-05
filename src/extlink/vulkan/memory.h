typedef struct {
    vk_StructureType type;
    const void*      next;
    vk_DeviceSize    allocation_size;
    u32              memory_type_index;
} vk_MemoryAllocateInfo;
