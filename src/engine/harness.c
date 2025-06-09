// Main application window was closed.
#define ENGINE_EXIT_CLOSE 0

// Error during engine init phase.
#define ENGINE_EXIT_ERROR_INIT 1

typedef struct {
    vk_Instance       instance;
    vk_PhysicalDevice physical_device;
    vk_SurfaceKHR     surface;

    u32 graphics_queue_family;
} VulkanContext;

/*/doc

Holds together platorm-specific context for running the engine.
*/
typedef struct {
    Logger lg;

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
    init_log(&h->lg, ss("pogus.log"), LOG_LEVEL_DEBUG);
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
