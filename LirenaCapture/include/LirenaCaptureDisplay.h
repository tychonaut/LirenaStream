#ifndef __LIRENA_CAPTURE_DISPLAY_H__
#define __LIRENA_CAPTURE_DISPLAY_H__


#include "LirenaConfig.h"

#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif


#include <sys/time.h>
#include <pthread.h>
#include <string.h>


#include <gst/app/gstappsrc.h>

#include <gst/video/videooverlay.h>

#include <m3api/xiApi.h> //Ximea API




struct LirenaCaptureWidgets 
{
	GdkScreen *screen;

    //TODO rename stuff

    // render window stuff
	GtkWidget *videoWindow;


    // control window stuff
    GtkWidget *controlWindow;

    GtkWidget 
        *boxmain, 
        *boxgpi, 
        *boxx, 
        *boxy, 

        *gpi1, 
        *gpi2, 
        *gpi3, 
        *gpi4, 

        *labelexp, 
        *exp, 
        *labelgain, 
        *gain,

        *labelx0, 
        *x0, 
        *labely0, 
        *y0, 
        *labelcx, 
        *cx, 
        *labelcy, 
        *cy, 

        *raw, 
        *show, 
        *run;
};





struct LirenaCaptureDisplay
{
	pthread_t videoThread;
	guintptr window_handle;

    //TODO rename member to "widgets"
	LirenaCaptureWidgets widgets;
};






// hack from Ximea as long as no good GstClock integrated...
inline unsigned long getcurus() 
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}




//TODO get rid of app state in this function -> diviede display and cam stuff!
struct  LirenaCaptureApp;

struct LirenaCamera;



//TODO get rid of app state in this function -> diviede display and cam stuff!
void* videoDisplay(void* appVoidPtr);

GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message,  LirenaCamera* cam); //gpointer)

gboolean time_handler(LirenaCaptureApp* app);

void video_widget_realize_cb(GtkWidget* widget,  LirenaCamera* cam); //gpointer)

gboolean start_cb(LirenaCaptureApp* appPtr);

// Messy hack to handle "shutdown app if CONTROL window is closed, 
// but to sth ales if RENDER window is closed".
// The original logic didn't even work in the first place...
struct close_cb_params
{
	bool doShutDownApp;
	//always non-null, but pointee kan be 0, implying thead is inactive
	pthread_t* videoThreadPtr;
};

gboolean close_cb(GtkWidget*, GdkEvent*,  
    //hack: need to be g_malloc'ed, then is g_free'd at and of callback.
	close_cb_params * params);


//TODO get rid of app state in this function -> diviede display and cam stuff!
gboolean update_run(GtkToggleButton *run,  LirenaCaptureApp* app);
gboolean update_show(GtkToggleButton *show,  LirenaCamera* cam); //gpointer)
gboolean update_raw(GtkToggleButton *raw, LirenaCaptureApp* appPtr);

gboolean update_gain(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_exposure(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_cy(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_cx(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_y0(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_x0(GtkAdjustment *adj, LirenaCamera* cam);



#endif //__LIRENA_CAPTURE_DISPLAY_H__
