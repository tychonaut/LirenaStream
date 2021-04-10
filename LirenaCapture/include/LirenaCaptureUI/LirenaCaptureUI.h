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

// to show a GStreamer rendering on a self-created 
// X/GDK/GTK window
#include <gst/video/videooverlay.h>

//Ximea API
#include <m3api/xiApi.h>


//fordwards:
class LirenaCaptureDevice;



// hack from Ximea as long as no good GstClock integrated...
inline unsigned long getcurus() 
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}


struct LirenaCaptureWidgets
{
    // Screen to grab window from, 
    // so GStreamer knows where to render
    // its xvimagesink stuff into
	GdkScreen *screen;
    // render window stuff
	GtkWidget *videoWindow;
    // control window stuff
    GtkWidget *controlWindow;
};



struct LirenaXimeaCaptureWidgets :
    public LirenaCaptureWidgets
{
    //TODO rename stuff
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



// UI Base class
// Specializezes to GUI and NUI (network UI),
// which in turn could specialize to 
// LirenaXimeaNUI and LirenaXimeaGUI, 
// LirenaMagewellEcoNUI and LirenaMagewellEcoGUI
//TODO make this abstract base class
class LirenaCaptureUI
{
    public:
        //call first and globally way before UI instance creation!
        // This may be required because of low level stuff
        // like X threads shouldn't be messed with by delaying their init too much
        // returns if successfully initialized; returns false after first call,
        // to hint towards inner logical errors
        static bool init(LirenaConfig*  configPtr);
        
        // factory function to create subclasses based on device's type.
        // only Ximea and MagewellEco are currently supported
        static LirenaCaptureUI* createInstance(LirenaCaptureDevice* device);


    protected: //protected ctr, use factory function
    public: //TODO remove public, just make it compile for now
        explicit LirenaCaptureUI(LirenaConfig const * config);
        
    public:
        virtual ~LirenaCaptureUI();


        // Setup network sockets or widgets
        //TODO make pure abstract and subclass
        // replace lirenaCaptureDisplayController_setupWidgets
        virtual bool setupUI();// = 0;

        //replace lirenaCaptureDisplayController_setupCallbacks
        virtual bool setupCallbacks();// = 0;

        //n.b. all the rest of the logic is in 
        // LirenaStreamer and
        // LirenaCaptureDevice (and its specializations)

        LirenaConfig const * configPtr;

        // yes, const is at the right place: the pointee can be altered,
        // the pointer itself not; i.e., 
        // the used device does not change for the life time of the UI
        LirenaCaptureDevice * const devicePtr;


       //TODO outsource to GUI subclass
        //useful for all GUI instances
	    guintptr drawableWindow_handle;

        //TODO rename to LirenaXimeaCaptureWidgets, outsource to subclass
	    LirenaXimeaCaptureWidgets widgets;

};


// class LirenaCaptureASSGUI : public LirenaCaptureUI
// {

// }




// Forward types for pointer params -------------------------------------------
struct LirenaCaptureApp;
struct LirenaXimeaStreamer_CameraParams;

// function interface ----------------------------------------------------------

//{ init GUI stuff
bool lirenaCaptureDisplayController_setupWidgets(
    LirenaCaptureUI *dispCtrl);
bool lirenaCaptureDisplayController_setupCallbacks(
    LirenaCaptureApp * appPtr);
//}




gboolean lirenaCaptureDisplayController_initCam_startCaptureThread(
    GtkToggleButton *run,  LirenaCaptureApp* app);

gboolean lirenaCaptureDisplayController_setupCamParams(
    GtkToggleButton *raw, LirenaCaptureApp* appPtr);




//-----------------------------------------------------------------------------
// Everything below seems irrelevant for non-GUI mode


//{ stuff to bind GStreamer renderings to an X/GDK/GTK window
//  via videooverlay
GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message, 
    LirenaCaptureUI* displayCtrl);  //LirenaXimeaStreamer_CameraParams* cam); //gpointer)

void video_widget_realize_cb(GtkWidget *widget, 
	LirenaCaptureUI* displayCtrl);
//}


// button sensitivity callback: irrelevant for non-GUI mode
gboolean lirenaCaptureDisplayController_initCamButtonSensitivity(
    LirenaCaptureApp* app);

// some kind of hack to force ANOTHER callback to be called early...
// ... don't understand the details (MS)
// is irrelevant for non-GUI mode
gboolean lirenaCaptureDisplayController_startHack_cb(
    LirenaCaptureApp* appPtr);



// Messy hack to handle "shutdown app if CONTROL window is closed, 
// but to sth ales if RENDER window is closed".
// The original logic didn't even work in the first place...
struct close_cb_params
{
	bool doShutDownApp;
	//always non-null, but pointee kan be 0, implying thead is inactive
	pthread_t* captureThreadPtr;
    LirenaXimeaStreamer_CameraParams * camPtr;
};

gboolean close_cb(GtkWidget*, GdkEvent*,  
    //hack: need to be g_malloc'ed, then is g_free'd at and of callback.
	close_cb_params * params);







//following only widget-to-cam-param stuff ------------------------------------
gboolean update_show(GtkToggleButton *show,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_gain(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_exposure(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_cy(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_cx(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_y0(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_x0(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams* cam);



#endif //__LIRENA_CAPTURE_DISPLAY_H__
