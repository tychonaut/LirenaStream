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





// hack from Ximea as long as no good GstClock integrated...
inline unsigned long getcurus() 
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}



struct CamControlWindow 
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



void* videoDisplay(void*);

GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message, gpointer);

gboolean time_handler(CamControlWindow *ctrl);

void video_widget_realize_cb(GtkWidget* widget, gpointer);

gboolean start_cb(CamControlWindow* ctrl);
gboolean close_cb(GtkWidget*, GdkEvent*, gpointer quit);


gboolean update_run(GtkToggleButton *run, CamControlWindow *ctrl);
gboolean update_show(GtkToggleButton *show, gpointer);
gboolean update_raw(GtkToggleButton *raw, CamControlWindow *ctrl);

gboolean update_gain(GtkAdjustment *adj, gpointer);
gboolean update_exposure(GtkAdjustment *adj, gpointer);
gboolean update_cy(GtkAdjustment *adj, gpointer);
gboolean update_cx(GtkAdjustment *adj, gpointer);
gboolean update_y0(GtkAdjustment *adj, gpointer);
gboolean update_x0(GtkAdjustment *adj, gpointer);



#endif //__STREAM_VIEWER_GUI_H__