static void
engine_harness_create_window(EngineHarness* h) {
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
