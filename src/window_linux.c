#include "link_xcb.h"
#include "extlink/vulkan/include.h"

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
engine_harness_create_window(EngineHarness* h) {
    init_log(&h->lg, ss("test.log"), LOG_LEVEL_DEBUG);

	h->connection = xcb_connect(nil, nil); // Callers need to use xcb_connection_has_error() to check for failure.
    if (h->connection == nil) {
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

	xcb_ScreenIterator iter = xcb_setup_roots_iterator(xcb_get_setup(h->connection));

    // TODO: check if there no screens at all
	h->screen = &iter.data[0]; // TODO: pick appropriate screen if there are many
		
    // Register event types.
    // XCB_CW_BACK_PIXEL = filling then window bg with a single colour
    // XCB_CW_EVENT_MASK is required.
    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // Listen for keyboard and mouse buttons
    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                       XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                       XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    // Values to be sent over XCB (bg colour, events)
    u32 values[] = {h->screen->black_pixel, event_values};
	
	h->window = xcb_generate_id(h->connection);
	xcb_VoidCookie cookie = xcb_create_window(h->connection,
				   0, h->window, h->screen->root,
				   0, 0, 640, 480,
				   0,
				   XCB_WINDOW_CLASS_INPUT_OUTPUT,
				   h->screen->root_visual,
				   event_mask, values);

	(void)(cookie);


    // Change the title
    xcb_change_property(
        h->connection,
        XCB_PROP_MODE_REPLACE,
        h->window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,  // data should be viewed 8 bits at a time
        cast(u32, h->title.len), h->title.ptr
    );

    // Tell the server to notify when the window manager
    // attempts to destroy the window.
    c_string wm_delete_window = ss("WM_DELETE_WINDOW");
    xcb_InternalAtomCookie wm_delete_cookie = xcb_intern_atom(
        h->connection, 0,
        cast(u16, wm_delete_window.len), wm_delete_window.ptr
    );
    
    c_string wm_protocols = ss("WM_PROTOCOLS");
    xcb_InternalAtomCookie wm_protocols_cookie = xcb_intern_atom(
        h->connection, 0,
        cast(u16, wm_protocols.len), wm_protocols.ptr
    );
    
    xcb_InternalAtomReply* wm_delete_reply = xcb_intern_atom_reply(
        h->connection,
        wm_delete_cookie, nil
    );
    
    xcb_InternalAtomReply* wm_protocols_reply = xcb_intern_atom_reply(
        h->connection,
        wm_protocols_cookie, nil
    );
    
    h->wm_delete_window = wm_delete_reply->atom;
    h->wm_protocols = wm_protocols_reply->atom;

    xcb_change_property(
        h->connection,
        XCB_PROP_MODE_REPLACE,
        h->window,
        wm_protocols_reply->atom,
        4, 32, 1, // TODO: document this magic numbers
        &wm_delete_reply->atom
    );

	
	xcb_map_window(h->connection, h->window);
	
	xcb_flush(h->connection);
}

static void
log_vulkan_error(Logger *lg, str s, vk_Result r) {
    log_error_field(lg, s, log_field_s64(ss("vk_result"), r));
}

#define VULKAN_MAX_PHYSICAL_DEVICES      2
#define VULKAN_MAX_DEVICE_EXTENSIONS     256
#define VULKAN_MAX_DEVICE_QUEUE_FAMILIES 16

static void
init_vulkan(EngineHarness* h) {
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

    u32 physical_device_count;
    r = vk_enumerate_physical_devices(h->vk.instance, &physical_device_count, nil);
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

    u32 device_extension_count;
    r = vk_enumerate_device_extension_properties(h->vk.physical_device, nil, &device_extension_count, nil);
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
        }

        // TODO: exit if we unable to find appropriate queue
    }

    vk_XcbSurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.type = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_create_info.connection = h->connection;
    surface_create_info.window = h->window;

    r = vk_create_xcb_surface_khr(h->vk.instance, &surface_create_info, nil, &h->vk.surface);
    if (r != 0) {
        log_vulkan_error(&h->lg, ss("create vulkan surface"), r);
        engine_harness_mark_exit(h, ENGINE_EXIT_ERROR_INIT);
        return;
    }

    log_debug(&h->lg, ss("vulkan init successfully finished"));
}

static void
init_engine_harness(EngineHarness* h) {
    engine_harness_create_window(h);
    if (h->exit) {
        return;
    }

    init_vulkan(h);
    if (h->exit) {
        return;
    }
}

static void
poll_system_events(EngineHarness* h) {
    // Poll for events until null is returned
    while (true) {
        xcb_GenericEvent* event = xcb_poll_for_event(h->connection);
        if (event == 0) {
            // No more events for now, stop polling
            return;
        }

        // Input events
        switch (event->response_type & ~0x80) { // TODO: what is this secret mask?
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
                // TODO: Key presses and releases
            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
                // TODO: Mouse button presses and releases
            } break;
            case XCB_MOTION_NOTIFY: {
                // TODO: mouse movement
            } break;
            case XCB_CONFIGURE_NOTIFY: {
                // TODO: Resizing
            } break;

            case XCB_CLIENT_MESSAGE: {
                xcb_ClientMessageEvent* message = cast(xcb_ClientMessageEvent*, event);
                
                // Window close
                if (message->data.data32[0] == h->wm_delete_window) {
                    engine_harness_mark_exit(h, ENGINE_EXIT_CLOSE);
                    return;
                }
            } break;
            default:
                // Something else
                break;
        }

        free(event);
    }
}
