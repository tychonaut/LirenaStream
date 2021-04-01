#ifndef __STREAM_VIEWER_GUI_H__
#define __STREAM_VIEWER_GUI_H__


#include "LirenaOptions.h"

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




struct LirenaCamStreamControlWindow 
{
    GtkWidget *window, 

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


struct LirenaCamStreamLocalDisplay
{
	LirenaCamStreamControlWindow controlWindow;
	
	pthread_t videoThread;
	guintptr window_handle;

};






// hack from Ximea as long as no good GstClock integrated...
inline unsigned long getcurus() 
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}




//TODO get rid of app state in this function -> diviede display and cam stuff!
struct  LirenaCamStreamApp;

struct LirenaCamera;



//TODO get rid of app state in this function -> diviede display and cam stuff!
void* videoDisplay(void* appVoidPtr);

GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message,  LirenaCamera* cam); //gpointer)

gboolean time_handler(LirenaCamStreamApp* app);

void video_widget_realize_cb(GtkWidget* widget,  LirenaCamera* cam); //gpointer)

gboolean start_cb(LirenaCamStreamApp* appPtr);//(LirenaCamStreamControlWindow* ctrl);

gboolean close_cb(GtkWidget*, GdkEvent*, gpointer quit);

//TODO get rid of app state in this function -> diviede display and cam stuff!
gboolean update_run(GtkToggleButton *run,  LirenaCamStreamApp* app);
gboolean update_show(GtkToggleButton *show,  LirenaCamera* cam); //gpointer)
gboolean update_raw(GtkToggleButton *raw, LirenaCamStreamApp* appPtr);

gboolean update_gain(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_exposure(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_cy(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_cx(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_y0(GtkAdjustment *adj,  LirenaCamera* cam); //gpointer)
gboolean update_x0(GtkAdjustment *adj, LirenaCamera* cam);



#endif //__STREAM_VIEWER_GUI_H__