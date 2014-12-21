#include <stdio.h>
#include <signal.h>

#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>

#include <JavaScriptCore/JavaScript.h>

#include "Browser.h"

Browser* Browser::browserInstance = NULL;

static void destroyWindowCb(GtkWidget* widget, GtkWidget* window);
static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window);


gboolean key_press_event(GtkWidget*, GdkEventKey*);
gboolean key_release_event(GtkWidget*, GdkEventKey*);
gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
gboolean button_press_event(GtkWidget *widget, GdkEventButton *event);
gboolean button_release_event(GtkWidget *widget, GdkEventButton *event);

void _afterInit();
void reload_browser(int);
void toggle_fullscreen(int);
void maximize();
void unmaximize();

static WebKitWebView* web_view;
static GtkWidget *window;
const char* default_url = "http://pitwo/mediahub.html";

volatile JSContextRef notification_ctx;
volatile JSObjectRef notification_object;

Browser* Browser::getInstance() {
    if (!browserInstance) {
        browserInstance = new Browser;
    }

    return browserInstance;
}

/* ================================================================= */

/* Class initialize */
static void mediahub_init_cb(JSContextRef ctx,
                                 JSObjectRef object)
{
    /* Inits MediaHub */
    printf("Init MediaHub.");

    notification_ctx = ctx;
    notification_object = object;

    fflush(stdout);
}

/* MediaHub.sendEvent method callback implementation */
static JSValueRef mediahub_sendEvent_cb(JSContextRef context,
                       JSObjectRef function,
                       JSObjectRef thisObject,
                       size_t argumentCount,
                       const JSValueRef arguments[],
                       JSValueRef *exception)
{
    /* At least, one argument must be received */
    if (argumentCount == 1 && JSValueIsString(context, arguments[0])) {
        /* Converts JSValue to char */
        size_t len;
        char *cstr;
        JSStringRef jsstr = JSValueToStringCopy(context, arguments[0], NULL);
        len = JSStringGetMaximumUTF8CStringSize(jsstr);
        cstr = g_new(char, len);
        JSStringGetUTF8CString(jsstr, cstr, len);

        //printf("cstr = [%s]", cstr);

        (*Browser::getInstance()->receivedUIEvent)(cstr);

        g_free(cstr);
        JSStringRelease(jsstr);
    }

    return JSValueMakeUndefined(context);
}

/* Class method declarations */
static const JSStaticFunction mediahub_staticfuncs[] =
{
    { "sendEvent", mediahub_sendEvent_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static const JSClassDefinition mediahub_def =
{
    0,                     // version
    kJSClassAttributeNone, // attributes
    "MediaHub",            // className
    NULL,                  // parentClass
    NULL,                  // staticValues
    mediahub_staticfuncs,  // staticFunctions
    mediahub_init_cb,      // initialize
    NULL,                  // finalize
    NULL,                  // hasProperty
    NULL,                  // getProperty
    NULL,                  // setProperty
    NULL,                  // deleteProperty
    NULL,                  // getPropertyNames
    NULL,                  // callAsFunction
    NULL,                  // callAsConstructor
    NULL,                  // hasInstance
    NULL                   // convertToType
};

/* Callback - JavaScript window object has been cleared */
static void window_object_cleared_cb(WebKitWebView  *web_view,
                                     WebKitWebFrame *frame,
                                     gpointer        context,
                                     gpointer        window_object,
                                     gpointer        user_data)

{
    /* Add classes to JavaScriptCore */
    JSClassRef classDef = JSClassCreate(&mediahub_def);
    JSObjectRef classObj = JSObjectMake((OpaqueJSContext *) context, classDef, context);
    JSObjectRef globalObj = JSContextGetGlobalObject((OpaqueJSContext *) context);
    JSStringRef str = JSStringCreateWithUTF8CString("MediaHub");
    JSObjectSetProperty((OpaqueJSContext *) context, globalObj, str, classObj, kJSPropertyAttributeNone, NULL);
}
/* ================================================================= */



int Browser::initBrowser(int argc, char** argv) {
  gtk_init(&argc, &argv);

  mouseContinuousMove = false;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

  /* Connect the window object cleared event with callback */
  g_signal_connect (G_OBJECT (web_view), "window-object-cleared", G_CALLBACK(window_object_cleared_cb), web_view);

  //signal(SIGHUP, reload_browser);
  //signal(SIGUSR1, toggle_fullscreen);

  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

  if(argc > 1) {
    webkit_web_view_load_uri(web_view, argv[1]);
  }
  else {
    webkit_web_view_load_uri(web_view, default_url);
  }

  g_signal_connect(window, "key_press_event", G_CALLBACK(key_press_event), NULL);
  g_signal_connect(window, "key_release_event", G_CALLBACK(key_release_event), NULL);

  // Set up callbacks so that if either the main window or the browser instance is
  // closed, the program will exit
  g_signal_connect(window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
  g_signal_connect(web_view, "close-web-view", G_CALLBACK(closeWebViewCb), window);

  g_signal_connect(web_view, "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
  g_signal_connect(web_view, "button_press_event", G_CALLBACK(button_press_event), NULL);
  g_signal_connect(web_view, "button_release_event", G_CALLBACK(button_release_event), NULL);

  gtk_widget_set_events ((GtkWidget*) web_view, (gint) (GDK_EXPOSURE_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_POINTER_MOTION_MASK
             | GDK_POINTER_MOTION_HINT_MASK));

  // Make sure that when the browser area becomes visible, it will get mouse
  // and keyboard events
  gtk_widget_grab_focus(GTK_WIDGET(web_view));

  maximize();

  // Make sure the main window and all its contents are visible
  gtk_widget_show_all(window);

  _afterInit();

  // Run the main GTK+ event loop
  gtk_main();

  return 0;
}

void _afterInit() {
    int x, y;
    GdkModifierType state;

    gdk_window_get_pointer(window->window, &x, &y, &state);

    MouseCoordinates mouseCoordinates;
    mouseCoordinates.x = x;
    mouseCoordinates.y = y;

    (*Browser::getInstance()->afterInit)(mouseCoordinates);
}

void Browser::setMouseContinuousMove(bool mouseContinuousMove) {
    this->mouseContinuousMove = mouseContinuousMove;
}

bool Browser::isMouseContinuousMove() {
    return this->mouseContinuousMove;
}

void Browser::setPreventDefaultEvents(bool preventDefaultEvents) {
    this->preventDefaultEvents = preventDefaultEvents;
}

gboolean Browser::isPreventDefaultEvents() {
    return this->preventDefaultEvents;
}

gboolean key_press_event(GtkWidget* window, GdkEventKey* key) {
  if (!Browser::getInstance()->isPreventDefaultEvents()) {
      if(key->type == GDK_KEY_PRESS && key->keyval == GDK_F5) {
        reload_browser(0);
      }

      else if(key->type == GDK_KEY_PRESS && key->keyval == GDK_F11) {
        toggle_fullscreen(0);
      }
  }

  //printf("key_press_event (%d , %d)\n", key->type, key->keyval);

  (*Browser::getInstance()->keyPress)(key->keyval);

  return Browser::getInstance()->isPreventDefaultEvents();
}

gboolean key_release_event(GtkWidget* window, GdkEventKey* key) {

  //printf("key_release_event (%d , %d)\n", key->type, key->keyval);

  (*Browser::getInstance()->keyRelease)(key->keyval);

  return Browser::getInstance()->isPreventDefaultEvents();
}

void reload_browser(int signum) {
  webkit_web_view_reload_bypass_cache(web_view);
}

void toggle_fullscreen(int signum) {
  if(gtk_window_get_decorated(GTK_WINDOW(window))) {
    maximize();
  }
  else {
    unmaximize();
  }
}

void maximize() {
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
}

void unmaximize() {
  gtk_window_unmaximize(GTK_WINDOW(window));
  gtk_window_unfullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
  gtk_window_resize(GTK_WINDOW(window), 1280, 768);
}

gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event) {
    int x, y;
    GdkModifierType state;
    bool jump = false;

    if (event->is_hint)
        gdk_window_get_pointer(event->window, &x, &y, &state);
    else {
        x = event->x;
        y = event->y;
        state = (GdkModifierType) event->state;
    }

    if (Browser::getInstance()->isMouseContinuousMove()) {
        GdkDisplay *display = gdk_display_get_default ();
        GdkScreen *screen = gdk_display_get_default_screen (display);
        gint height = gdk_screen_get_height(screen);
        gint width = gdk_screen_get_width(screen);

        if (x == 0 || x >= (width - 2)) {
            gdk_display_warp_pointer (display, screen, width/2, y);
            jump = true;
        }

        if (y == 0 || y >= (height - 2)) {
            gdk_display_warp_pointer (display, screen, x, height/2);
            jump = true;
        }
    }

    MouseCoordinates mouseCoordinates;
    mouseCoordinates.x = x;
    mouseCoordinates.y = y;
    mouseCoordinates.jump = jump;

    (*Browser::getInstance()->mouseMove)(mouseCoordinates);

    return Browser::getInstance()->isPreventDefaultEvents();
}

gboolean button_press_event(GtkWidget *widget, GdkEventButton *event) {

    //printf("button_press_event (%d , %d) %d\n", (int) event->x, (int) event->y, event->button);

    (*Browser::getInstance()->mouseButtonPress)(event->button);

    return Browser::getInstance()->isPreventDefaultEvents();
}

gboolean button_release_event(GtkWidget *widget, GdkEventButton *event) {

    //printf("button_release_event (%d , %d) %d\n", (int) event->x, (int) event->y, event->button);

    (*Browser::getInstance()->mouseButtonRelease)(event->button);

    return Browser::getInstance()->isPreventDefaultEvents();
}

static void destroyWindowCb(GtkWidget* widget, GtkWidget* window)
{
    (*Browser::getInstance()->closeWindow)();
    gtk_main_quit();
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window) {
    gtk_widget_destroy(window);
    return TRUE;
}
