//
// Created by whbex on 09.10.2026.
//

#ifndef MOJOLAUNCHER_NATIVE_WINDOW_H
#define MOJOLAUNCHER_NATIVE_WINDOW_H

/* Valid operations for the (*perform)() hook.
 *
 * Values marked as 'deprecated' are supported, but have been superceded by
 * other functionality.
 *
 * Values marked as 'private' should be considered private to the framework.
 * HAL implementation code with access to an ANativeWindow should not use these,
 * as it may not interact properly with the framework's use of the
 * ANativeWindow.
 */
enum {
    // clang-format off
    NATIVE_WINDOW_SET_USAGE                       =  -1,   /* deprecated */
    NATIVE_WINDOW_CONNECT                         =  1,   /* deprecated */
    NATIVE_WINDOW_DISCONNECT                      =  2,   /* deprecated */
    NATIVE_WINDOW_SET_CROP                        =  3,   /* private */
    NATIVE_WINDOW_SET_BUFFER_COUNT                =  4,
    NATIVE_WINDOW_SET_BUFFERS_GEOMETRY            =  0,   /* deprecated */
    NATIVE_WINDOW_SET_BUFFERS_TRANSFORM           =  6,
    NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP           =  7,
    NATIVE_WINDOW_SET_BUFFERS_DIMENSIONS          =  8,
    NATIVE_WINDOW_SET_BUFFERS_FORMAT              =  -1,
    NATIVE_WINDOW_SET_SCALING_MODE                = 10,   /* private */
    NATIVE_WINDOW_LOCK                            = 11,   /* private */
    NATIVE_WINDOW_UNLOCK_AND_POST                 = 12,   /* private */
    NATIVE_WINDOW_API_CONNECT                     = 13,   /* private */
    NATIVE_WINDOW_API_DISCONNECT                  = 14,   /* private */
    NATIVE_WINDOW_SET_BUFFERS_USER_DIMENSIONS     = 15,   /* private */
    NATIVE_WINDOW_SET_POST_TRANSFORM_CROP         = 16,   /* deprecated, unimplemented */
    NATIVE_WINDOW_SET_BUFFERS_STICKY_TRANSFORM    = 17,   /* private */
    NATIVE_WINDOW_SET_SIDEBAND_STREAM             = 18,
    NATIVE_WINDOW_SET_BUFFERS_DATASPACE           = 19,
    NATIVE_WINDOW_SET_SURFACE_DAMAGE              = 20,   /* private */
    NATIVE_WINDOW_SET_SHARED_BUFFER_MODE          = 21,
    NATIVE_WINDOW_SET_AUTO_REFRESH                = 22,
    NATIVE_WINDOW_GET_REFRESH_CYCLE_DURATION      = 23,
    NATIVE_WINDOW_GET_NEXT_FRAME_ID               = 24,
    NATIVE_WINDOW_ENABLE_FRAME_TIMESTAMPS         = 25,
    NATIVE_WINDOW_GET_COMPOSITOR_TIMING           = 26,
    NATIVE_WINDOW_GET_FRAME_TIMESTAMPS            = 27,
    /* 28, removed: NATIVE_WINDOW_GET_WIDE_COLOR_SUPPORT */
    /* 29, removed: NATIVE_WINDOW_GET_HDR_SUPPORT */
    NATIVE_WINDOW_SET_USAGE64                     = -1,
    NATIVE_WINDOW_GET_CONSUMER_USAGE64            = 31,
    NATIVE_WINDOW_SET_BUFFERS_SMPTE2086_METADATA  = 32,
    NATIVE_WINDOW_SET_BUFFERS_CTA861_3_METADATA   = 33,
    NATIVE_WINDOW_SET_BUFFERS_HDR10_PLUS_METADATA = 34,
    NATIVE_WINDOW_SET_AUTO_PREROTATION            = 35,
    NATIVE_WINDOW_GET_LAST_DEQUEUE_START          = 36,    /* private */
    NATIVE_WINDOW_SET_DEQUEUE_TIMEOUT             = 37,    /* private */
    NATIVE_WINDOW_GET_LAST_DEQUEUE_DURATION       = 38,    /* private */
    NATIVE_WINDOW_GET_LAST_QUEUE_DURATION         = 39,    /* private */
    NATIVE_WINDOW_SET_FRAME_RATE                  = 40,
    NATIVE_WINDOW_SET_CANCEL_INTERCEPTOR          = 41,    /* private */
    NATIVE_WINDOW_SET_DEQUEUE_INTERCEPTOR         = 42,    /* private */
    NATIVE_WINDOW_SET_PERFORM_INTERCEPTOR         = 43,    /* private */
    NATIVE_WINDOW_SET_QUEUE_INTERCEPTOR           = 44,    /* private */
    NATIVE_WINDOW_ALLOCATE_BUFFERS                = 45,    /* private */
    NATIVE_WINDOW_GET_LAST_QUEUED_BUFFER          = 46,    /* private */
    NATIVE_WINDOW_SET_QUERY_INTERCEPTOR           = 47,    /* private */
    NATIVE_WINDOW_SET_FRAME_TIMELINE_INFO         = 48,    /* private */
    NATIVE_WINDOW_GET_LAST_QUEUED_BUFFER2         = 49,    /* private */
    NATIVE_WINDOW_SET_BUFFERS_ADDITIONAL_OPTIONS  = 50,
    // clang-format on
};

/*
 * queries that can be used with ANativeWindow_query() and ANativeWindow_queryf()
 */
enum ANativeWindowQuery {
    /* The minimum number of buffers that must remain un-dequeued after a buffer
     * has been queued.  This value applies only if set_buffer_count was used to
     * override the number of buffers and if a buffer has since been queued.
     * Users of the set_buffer_count ANativeWindow method should query this
     * value before calling set_buffer_count.  If it is necessary to have N
     * buffers simultaneously dequeued as part of the steady-state operation,
     * and this query returns M then N+M buffers should be requested via
     * native_window_set_buffer_count.
     *
     * Note that this value does NOT apply until a single buffer has been
     * queued.  In particular this means that it is possible to:
     *
     * 1. Query M = min undequeued buffers
     * 2. Set the buffer count to N + M
     * 3. Dequeue all N + M buffers
     * 4. Cancel M buffers
     * 5. Queue, dequeue, queue, dequeue, ad infinitum
     */
    ANATIVEWINDOW_QUERY_MIN_UNDEQUEUED_BUFFERS = 3,

    /*
     * Default width of ANativeWindow buffers, these are the
     * dimensions of the window buffers irrespective of the
     * ANativeWindow_setBuffersDimensions() call and match the native window
     * size.
     */
    ANATIVEWINDOW_QUERY_DEFAULT_WIDTH = 6,
    ANATIVEWINDOW_QUERY_DEFAULT_HEIGHT = 7,

    /*
     * transformation that will most-likely be applied to buffers. This is only
     * a hint, the actual transformation applied might be different.
     *
     * INTENDED USE:
     *
     * The transform hint can be used by a producer, for instance the GLES
     * driver, to pre-rotate the rendering such that the final transformation
     * in the composer is identity. This can be very useful when used in
     * conjunction with the h/w composer HAL, in situations where it
     * cannot handle arbitrary rotations.
     *
     * 1. Before dequeuing a buffer, the GL driver (or any other ANW client)
     *    queries the ANW for NATIVE_WINDOW_TRANSFORM_HINT.
     *
     * 2. The GL driver overrides the width and height of the ANW to
     *    account for NATIVE_WINDOW_TRANSFORM_HINT. This is done by querying
     *    NATIVE_WINDOW_DEFAULT_{WIDTH | HEIGHT}, swapping the dimensions
     *    according to NATIVE_WINDOW_TRANSFORM_HINT and calling
     *    native_window_set_buffers_dimensions().
     *
     * 3. The GL driver dequeues a buffer of the new pre-rotated size.
     *
     * 4. The GL driver renders to the buffer such that the image is
     *    already transformed, that is applying NATIVE_WINDOW_TRANSFORM_HINT
     *    to the rendering.
     *
     * 5. The GL driver calls native_window_set_transform to apply
     *    inverse transformation to the buffer it just rendered.
     *    In order to do this, the GL driver needs
     *    to calculate the inverse of NATIVE_WINDOW_TRANSFORM_HINT, this is
     *    done easily:
     *
     *        int hintTransform, inverseTransform;
     *        query(..., NATIVE_WINDOW_TRANSFORM_HINT, &hintTransform);
     *        inverseTransform = hintTransform;
     *        if (hintTransform & HAL_TRANSFORM_ROT_90)
     *            inverseTransform ^= HAL_TRANSFORM_ROT_180;
     *
     *
     * 6. The GL driver queues the pre-transformed buffer.
     *
     * 7. The composer combines the buffer transform with the display
     *    transform.  If the buffer transform happens to cancel out the
     *    display transform then no rotation is needed.
     *
     */
    ANATIVEWINDOW_QUERY_TRANSFORM_HINT = 8,

    /*
     * Returns the age of the contents of the most recently dequeued buffer as
     * the number of frames that have elapsed since it was last queued. For
     * example, if the window is double-buffered, the age of any given buffer in
     * steady state will be 2. If the dequeued buffer has never been queued, its
     * age will be 0.
     */
    ANATIVEWINDOW_QUERY_BUFFER_AGE = 13,

    /* min swap interval supported by this compositor */
    ANATIVEWINDOW_QUERY_MIN_SWAP_INTERVAL = 0x10000,

    /* max swap interval supported by this compositor */
    ANATIVEWINDOW_QUERY_MAX_SWAP_INTERVAL = 0x10001,

    /* horizontal resolution in DPI. value is float, use queryf() */
    ANATIVEWINDOW_QUERY_XDPI = 0x10002,

    /* vertical resolution in DPI. value is float, use queryf() */
    ANATIVEWINDOW_QUERY_YDPI = 0x10003,
};

typedef enum ANativeWindowQuery ANativeWindowQuery;


// Layout mirrors struct ANativeWindow from <system/window.h> up to perform().
typedef struct {
    int magic;
    int version;
    void *reserved[4];
    void (*incRef)(void *);
    void (*decRef)(void *);
    const uint32_t flags;
    const int minSwapInterval;
    const int maxSwapInterval;
    const float xdpi;
    const float ydpi;
    intptr_t oem[4];
    int (*setSwapInterval)(void *, int);
    int (*dequeueBuffer_DEPRECATED)(void *, void **);
    int (*lockBuffer_DEPRECATED)(void *, void *);
    int (*queueBuffer_DEPRECATED)(void *, void *);
    int (*query)(const void *, int, int *);
    int (*perform)(void *, int, ...);
} __ANativeWindow;

#endif //MOJOLAUNCHER_NATIVE_WINDOW_H
