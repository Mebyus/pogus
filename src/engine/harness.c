// Main application window was closed.
#define ENGINE_EXIT_CLOSE 0

// Error during engine init phase.
#define ENGINE_EXIT_ERROR_INIT 1

typedef struct {
    vk_PhysicalDeviceMemoryProperties memory_properties;

    vk_Instance       instance;
    vk_PhysicalDevice physical_device;
    vk_Device         device;
    vk_Queue          graphics_queue;
    vk_CommandPool    command_pool;
    vk_SurfaceKHR     surface;

    vk_Extent2D       swapchain_extent;
    vk_SwapchainKHR   swapchain;
    vk_SwapchainKHR   old_swapchain;
    // vk_Framebuffer    framebuffer;

    vk_Semaphore  image_available_semaphore;
    vk_Semaphore  rendering_finished_semaphore;

    vk_Buffer       vertex_buffer;
	vk_DeviceMemory vertex_buffer_memory;
	
    vk_Buffer       index_buffer;
	vk_DeviceMemory index_buffer_memory;

    vk_VertexInputBindingDescription    vertex_binding_description;
    vk_VertexInputAttributeDescription  vertex_attribute_descriptions[2];

    u32 graphics_queue_family;
} VulkanContext;

/*/doc

Holds together platorm-specific context for running the engine.
*/
typedef struct {
    LogSink log_sink;
    Logger  lg;

    VulkanContext vk;

    // Title of the application window.
    str title;

    xcb_Connection* connection;
    xcb_Screen* screen;
    xcb_Window window;

    // Window manager event codes.
    xcb_Atom wm_delete_window;
    xcb_Atom wm_protocols;
    //

    // Sets process exit code when main engine loop breaks.
    uint exit_code;

    // If true then main engine loop will break.
    bool exit;
} EngineHarness;

static void
engine_harness_mark_exit(EngineHarness* h, uint code) {
    h->exit_code = code;
    h->exit = true;
}

static void
engine_harness_create_log(EngineHarness* h) {
    init_log_sink(&h->log_sink, ss("pogus.log"));
    init_log(&h->lg, &h->log_sink, LOG_LEVEL_DEBUG);
}

/* Forward declarations of platform stuff */

static void
engine_harness_create_window(EngineHarness* h);

static void
init_renderer(EngineHarness* h);

/* ====================================== */

static void
init_engine_harness(EngineHarness* h) {
    engine_harness_create_log(h);

    engine_harness_create_window(h);
    if (h->exit) {
        return;
    }

    init_renderer(h);
    if (h->exit) {
        return;
    }
}
