#define XCB_KEY_PRESS        2
#define XCB_KEY_RELEASE      3
#define XCB_BUTTON_PRESS     4
#define XCB_BUTTON_RELEASE   5
#define XCB_MOTION_NOTIFY    6
#define XCB_CONFIGURE_NOTIFY 22
#define XCB_CLIENT_MESSAGE   33

typedef u8  xcb_Keycode;
typedef u32 xcb_Window;
typedef u32 xcb_Atom;
typedef u32 xcb_Colormap;
typedef u32 xcb_VisualID;

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

typedef enum {
    XCB_PROP_MODE_REPLACE = 0,
    XCB_PROP_MODE_PREPEND = 1,
    XCB_PROP_MODE_APPEND = 2
} xcb_PropMode;


typedef enum xcb_atom_enum_t {
    XCB_ATOM_NONE = 0,
    XCB_ATOM_ANY = 0,
    XCB_ATOM_PRIMARY = 1,
    XCB_ATOM_SECONDARY = 2,
    XCB_ATOM_ARC = 3,
    XCB_ATOM_ATOM = 4,
    XCB_ATOM_BITMAP = 5,
    XCB_ATOM_CARDINAL = 6,
    XCB_ATOM_COLORMAP = 7,
    XCB_ATOM_CURSOR = 8,
    XCB_ATOM_CUT_BUFFER0 = 9,
    XCB_ATOM_CUT_BUFFER1 = 10,
    XCB_ATOM_CUT_BUFFER2 = 11,
    XCB_ATOM_CUT_BUFFER3 = 12,
    XCB_ATOM_CUT_BUFFER4 = 13,
    XCB_ATOM_CUT_BUFFER5 = 14,
    XCB_ATOM_CUT_BUFFER6 = 15,
    XCB_ATOM_CUT_BUFFER7 = 16,
    XCB_ATOM_DRAWABLE = 17,
    XCB_ATOM_FONT = 18,
    XCB_ATOM_INTEGER = 19,
    XCB_ATOM_PIXMAP = 20,
    XCB_ATOM_POINT = 21,
    XCB_ATOM_RECTANGLE = 22,
    XCB_ATOM_RESOURCE_MANAGER = 23,
    XCB_ATOM_RGB_COLOR_MAP = 24,
    XCB_ATOM_RGB_BEST_MAP = 25,
    XCB_ATOM_RGB_BLUE_MAP = 26,
    XCB_ATOM_RGB_DEFAULT_MAP = 27,
    XCB_ATOM_RGB_GRAY_MAP = 28,
    XCB_ATOM_RGB_GREEN_MAP = 29,
    XCB_ATOM_RGB_RED_MAP = 30,
    XCB_ATOM_STRING = 31,
    XCB_ATOM_VISUALID = 32,
    XCB_ATOM_WINDOW = 33,
    XCB_ATOM_WM_COMMAND = 34,
    XCB_ATOM_WM_HINTS = 35,
    XCB_ATOM_WM_CLIENT_MACHINE = 36,
    XCB_ATOM_WM_ICON_NAME = 37,
    XCB_ATOM_WM_ICON_SIZE = 38,
    XCB_ATOM_WM_NAME = 39,
    XCB_ATOM_WM_NORMAL_HINTS = 40,
    XCB_ATOM_WM_SIZE_HINTS = 41,
    XCB_ATOM_WM_ZOOM_HINTS = 42,
    XCB_ATOM_MIN_SPACE = 43,
    XCB_ATOM_NORM_SPACE = 44,
    XCB_ATOM_MAX_SPACE = 45,
    XCB_ATOM_END_SPACE = 46,
    XCB_ATOM_SUPERSCRIPT_X = 47,
    XCB_ATOM_SUPERSCRIPT_Y = 48,
    XCB_ATOM_SUBSCRIPT_X = 49,
    XCB_ATOM_SUBSCRIPT_Y = 50,
    XCB_ATOM_UNDERLINE_POSITION = 51,
    XCB_ATOM_UNDERLINE_THICKNESS = 52,
    XCB_ATOM_STRIKEOUT_ASCENT = 53,
    XCB_ATOM_STRIKEOUT_DESCENT = 54,
    XCB_ATOM_ITALIC_ANGLE = 55,
    XCB_ATOM_X_HEIGHT = 56,
    XCB_ATOM_QUAD_WIDTH = 57,
    XCB_ATOM_WEIGHT = 58,
    XCB_ATOM_POINT_SIZE = 59,
    XCB_ATOM_RESOLUTION = 60,
    XCB_ATOM_COPYRIGHT = 61,
    XCB_ATOM_NOTICE = 62,
    XCB_ATOM_FONT_NAME = 63,
    XCB_ATOM_FAMILY_NAME = 64,
    XCB_ATOM_FULL_NAME = 65,
    XCB_ATOM_CAP_HEIGHT = 66,
    XCB_ATOM_WM_CLASS = 67,
    XCB_ATOM_WM_TRANSIENT_FOR = 68
} xcb_AtomEnum;

typedef struct xcb_Connection xcb_Connection;

typedef struct {
    u32 sequence;
} xcb_InternalAtomCookie;

typedef struct {
    u8  response_type;
    u8  error_code;
    u16 sequence;
    u32 resource_id;
    u16 minor_code;
    u8  major_code;
    u8  pad0;
    u32 pad[5];
    u32 full_sequence;
} xcb_GenericError;

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

typedef struct {
    u8  response_type;
    u8  pad0;
    u16 sequence;
    u32 pad[7];
    u32 full_sequence;
} xcb_GenericEvent;

typedef union {
    u8  data8[20];
    u16 data16[10];
    u32 data32[5];
} xcb_ClientMessageData;

typedef struct {
    u8  response_type;
    u8  format;
    u16 sequence;
    
    xcb_Window window;
    xcb_Atom type;
    xcb_ClientMessageData data;
} xcb_ClientMessageEvent;

typedef struct {
    u8  response_type;
    u8  pad0;
    u16 sequence;
    u32 length;

    xcb_Atom atom;
} xcb_InternalAtomReply;

xcb_Connection*
xcb_connect(const char *display_name, s32 *screenp);

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

xcb_GenericEvent*
xcb_poll_for_event(xcb_Connection *c);

xcb_InternalAtomReply*
xcb_intern_atom_reply(xcb_Connection* c,
                       xcb_InternalAtomCookie cookie,
                       xcb_GenericError** e);

xcb_VoidCookie
xcb_change_property(xcb_Connection* c,
                     u8 mode,
                     xcb_Window window,
                     xcb_Atom property,
                     xcb_Atom type,
                     u8 format,
                     u32 data_len,
                     const void* data);

xcb_InternalAtomCookie
xcb_intern_atom(xcb_Connection* c,
                 u8 only_if_exists,
                 u16 name_len,
                 const u8* name);
