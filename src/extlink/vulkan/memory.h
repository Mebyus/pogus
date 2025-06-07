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
