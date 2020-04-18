#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>


xcb_connection_t    *conn;
xcb_screen_t	    *screen;


void clip_window(xcb_window_t wid)
{
    uint32_t mask;
    xcb_gcontext_t  gc;

    mask = XCB_GC_CLIP_ORIGIN_X | XCB_GC_CLIP_ORIGIN_Y;// | XCB_GC_CLIP_MASK

    const uint32_t values[] = {
	1,
	1,
    };

    gc = xcb_generate_id(conn);
    xcb_create_gc(
	conn,
	gc,
	wid,
	mask,
	values
    );
}


void set_window_type(xcb_window_t wid) {
    xcb_intern_atom_cookie_t cookie1, cookie2;
    xcb_intern_atom_reply_t *reply1, *reply2;

    cookie1 = xcb_intern_atom(conn, 0, 19, "_NET_WM_WINDOW_TYPE");
    reply1 = xcb_intern_atom_reply(conn, cookie1, NULL);
    cookie2 = xcb_intern_atom(conn, 0, 32, "_NET_WM_WINDOW_TYPE_NOTIFICATION");
    reply2 = xcb_intern_atom_reply(conn, cookie2, NULL);

    xcb_change_property(
	conn,
	XCB_PROP_MODE_REPLACE,
	wid,
	reply1->atom,
	XCB_ATOM_ATOM,
	32,
	1,
	&reply2->atom
    );
    free(reply1);
    free(reply2);
}


xcb_window_t setup_window()
{
    xcb_window_t wid = xcb_generate_id(conn);

    uint32_t mask_val = 0;
    mask_val |= XCB_EVENT_MASK_ENTER_WINDOW;

    xcb_create_window(
	conn,
	XCB_COPY_FROM_PARENT,
	wid,
	screen->root,
	0,
	0,
	1, // screen->width_in_pixels
	screen->height_in_pixels,
	0,
	XCB_WINDOW_CLASS_INPUT_ONLY,
	XCB_COPY_FROM_PARENT,
	XCB_CW_EVENT_MASK,
	&mask_val
    );

    set_window_type(wid);
    clip_window(wid);

    uint32_t above = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, &above);

    xcb_change_property(
	conn,
	XCB_PROP_MODE_REPLACE,
	wid,
	XCB_ATOM_WM_NAME,
	XCB_ATOM_STRING,
	8,
	4,
	"xinf"
    );

    xcb_map_window(conn, wid);
    xcb_flush(conn);
    return wid;
}


void event_loop()
{
    xcb_generic_event_t *event;

    while((event = xcb_wait_for_event(conn))) {
	switch (event->response_type) {

	    case XCB_ENTER_NOTIFY:
	    printf("# enter window\n");
	    break;

	    default:
	    printf("# %d\n", event->response_type);
	    break;
	}

    free(event);
    }
}


int main()
{
    xcb_window_t wid;

    conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn))
	exit(1);
    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    wid = setup_window();
    event_loop();

    xcb_unmap_window(conn, wid);
    xcb_disconnect(conn);

    return 0;
}
