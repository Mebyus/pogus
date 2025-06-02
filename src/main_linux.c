#include "types.c"

typedef u32 xcb_Window;
typedef u32 xcb_Colormap;
typedef u32 xcb_VisualID;
typedef u8 xcb_Keycode;

typedef enum {
    XCB_CW_BACK_PIXMAP = 1,
    XCB_CW_BACK_PIXEL = 2,
    XCB_CW_BORDER_PIXMAP = 4,
    XCB_CW_BORDER_PIXEL = 8,
    XCB_CW_BIT_GRAVITY = 16,
    XCB_CW_WIN_GRAVITY = 32,
    XCB_CW_BACKING_STORE = 64,
    XCB_CW_BACKING_PLANES = 128,
    XCB_CW_BACKING_PIXEL = 256,
    XCB_CW_OVERRIDE_REDIRECT = 512,
    XCB_CW_SAVE_UNDER = 1024,
    XCB_CW_EVENT_MASK = 2048,
    XCB_CW_DONT_PROPAGATE = 4096,
    XCB_CW_COLORMAP = 8192,
    XCB_CW_CURSOR = 16384
} xcb_CW;

typedef enum {
    XCB_EVENT_MASK_NO_EVENT = 0,
    XCB_EVENT_MASK_KEY_PRESS = 1,
    XCB_EVENT_MASK_KEY_RELEASE = 2,
    XCB_EVENT_MASK_BUTTON_PRESS = 4,
    XCB_EVENT_MASK_BUTTON_RELEASE = 8,
    XCB_EVENT_MASK_ENTER_WINDOW = 16,
    XCB_EVENT_MASK_LEAVE_WINDOW = 32,
    XCB_EVENT_MASK_POINTER_MOTION = 64,
    XCB_EVENT_MASK_POINTER_MOTION_HINT = 128,
    XCB_EVENT_MASK_BUTTON_1_MOTION = 256,
    XCB_EVENT_MASK_BUTTON_2_MOTION = 512,
    XCB_EVENT_MASK_BUTTON_3_MOTION = 1024,
    XCB_EVENT_MASK_BUTTON_4_MOTION = 2048,
    XCB_EVENT_MASK_BUTTON_5_MOTION = 4096,
    XCB_EVENT_MASK_BUTTON_MOTION = 8192,
    XCB_EVENT_MASK_KEYMAP_STATE = 16384,
    XCB_EVENT_MASK_EXPOSURE = 32768,
    XCB_EVENT_MASK_VISIBILITY_CHANGE = 65536,
    XCB_EVENT_MASK_STRUCTURE_NOTIFY = 131072,
    XCB_EVENT_MASK_RESIZE_REDIRECT = 262144,
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY = 524288,
    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT = 1048576,
    XCB_EVENT_MASK_FOCUS_CHANGE = 2097152,
    XCB_EVENT_MASK_PROPERTY_CHANGE = 4194304,
    XCB_EVENT_MASK_COLOR_MAP_CHANGE = 8388608,
    XCB_EVENT_MASK_OWNER_GRAB_BUTTON = 16777216
} xcb_EventMask;


typedef enum {
    XCB_WINDOW_CLASS_COPY_FROM_PARENT = 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT = 1,
    XCB_WINDOW_CLASS_INPUT_ONLY = 2
} xcb_WindowClass;

typedef struct xcb_Connection xcb_Connection;

typedef struct {
    u8 status;
    u8 pad0;
    u16 protocol_major_version;
    u16 protocol_minor_version;
    u16 length;
    u32 release_number;
    u32 resource_id_base;
    u32 resource_id_mask;
    u32 motion_buffer_size;
    u16 vendor_len;
    u16 maximum_request_length;
    u8 roots_len;
    u8 pixmap_formats_len;
    u8 image_byte_order;
    u8 bitmap_format_bit_order;
    u8 bitmap_format_scanline_unit;
    u8 bitmap_format_scanline_pad;
    xcb_Keycode min_keycode;
    xcb_Keycode max_keycode;
    u8 pad1[4];
} xcb_Setup;

typedef struct {
    u32 sequence;
} xcb_VoidCookie;

typedef struct {
    xcb_Window root;
    xcb_Colormap default_colormap;
    u32 white_pixel;
    u32 black_pixel;
    u32 current_input_masks;
    u16 width_in_pixels;
    u16 height_in_pixels;
    u16 width_in_millimeters;
    u16 height_in_millimeters;
    u16 min_installed_maps;
    u16 max_installed_maps;
    xcb_VisualID root_visual;
    u8 backing_stores;
    u8 save_unders;
    u8 root_depth;
    u8 allowed_depths_len;
} xcb_Screen;

typedef struct {
    xcb_Screen* data;
    s32 rem;
    s32 index;
} xcb_ScreenIterator;

xcb_Connection* xcb_connect(const char *display_name, s32 *screenp);

u32
xcb_generate_id(xcb_Connection* c);

const xcb_Setup*
xcb_get_setup(xcb_Connection* c);

s32
xcb_flush(xcb_Connection* c);

xcb_ScreenIterator
xcb_setup_roots_iterator(const xcb_Setup* setup);

xcb_VoidCookie
xcb_map_window(xcb_Connection* c, xcb_Window window);

xcb_VoidCookie
xcb_create_window(xcb_Connection *c,
                   u8 depth,
                   xcb_Window wid,
                   xcb_Window parent,
                   s16 x,
                   s16 y,
                   u16 width,
                   u16 height,
                   u16 border_width,
                   u16 window_class,
                   xcb_VisualID visual,
                   u32 value_mask,
                   const void *value_list);

u32
sleep(u32 seconds);

xcb_Connection* connection;
xcb_Screen* screen;

void
create_window(void) {
	u32 mask;
	u32 values[2];
	
	xcb_Window window;
	xcb_VoidCookie cookie;
	
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
	
	window = xcb_generate_id(connection);
	cookie = xcb_create_window(connection,
				   0, window, screen->root,
				   0, 0, 640, 480,
				   0,
				   XCB_WINDOW_CLASS_INPUT_OUTPUT,
				   screen->root_visual,
				   mask, values);

	(void)(cookie);
	
	xcb_map_window(connection, window);
}

int
main(void) {
	connection = xcb_connect(nil, nil); // Callers need to use xcb_connection_has_error() to check for failure.
	xcb_ScreenIterator iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
	screen = &iter.data[0];
	
	create_window();
	
	xcb_flush(connection);
	sleep(5);
}
