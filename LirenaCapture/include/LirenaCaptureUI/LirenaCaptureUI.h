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
//class LirenaCaptureDevice;
class LirenaStreamer;


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
        //*show, 
        *run;
};



// UI Base class, with "do nothing" dummy implementations,
// so that not all the time the "hasGUI" flag needs to be checked.
// Specializezes to GUI (Graphical UI) and NUI (Network UI (planned)),
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
        
        // factory function to create subclasses based on streamer's capture device's type.
        // Only Ximea and MagewellEco are currently supported
        static LirenaCaptureUI * createInstance(LirenaStreamer* streamerPtr);


    protected: //protected ctr, use factory function
    //public: //TODO remove public, just make it compile for now
        explicit LirenaCaptureUI(LirenaStreamer* streamerPtr);
        
    public:
        virtual ~LirenaCaptureUI();


        // Setup network sockets or widgets
        //TODO make pure abstract and subclass
        // replace lirenaCaptureDisplayController_setupWidgets
        virtual bool setupUI();//{return true;}

        //replace lirenaCaptureDisplayController_setupCallbacks
        virtual bool setupCallbacks();//{return true;}

        virtual bool enterMainLoop();//{return false;}

        virtual bool shutdownUI();//{return true;}



        //contains pointers to config and capture device, to all is there
        // for the UI
        LirenaStreamer* streamerPtr;

        // non-GUI loop for bus-listening,
        // in case local displaying is not requested;
        // probably not nececcary as long as no listening to 
        // any messages ;()
        GMainLoop *pureMainLoop = nullptr;



    //TODO outsource to GUI subclass
        //useful for all GUI instances
	    guintptr drawableWindow_handle = 0;
        //TODO rename to  outsource to subclass
	    LirenaXimeaCaptureWidgets widgets;

};


//KISS --> no intermediat GUI class
// class LirenaCaptureGUI : public LirenaCaptureUI
// {
//     public:
//         explicit LirenaCaptureGUI(LirenaStreamer* streamerPtr);
    
//         //useful for all GUI instances

// };


class LirenaCaptureXimeaGUI : public LirenaCaptureUI
{
    public:
        explicit LirenaCaptureXimeaGUI(LirenaStreamer* streamerPtr);
        virtual ~LirenaCaptureXimeaGUI();

        //TODO move from superclass here
	    //guintptr drawableWindow_handle = 0;
        //LirenaXimeaCaptureWidgets widgets;

        virtual bool setupUI();//{return true;}

        //replace lirenaCaptureDisplayController_setupCallbacks
        virtual bool setupCallbacks();//{return true;}

        virtual bool enterMainLoop();//{return false;}

        virtual bool shutdownUI();//{return true;}
};










//----------------------------------------------------------------------------
// old code below





// Forward types for pointer params -------------------------------------------
class LirenaCaptureApp;
class LirenaStreamer;

//TODO rename
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



gboolean close_cb(GtkWidget*, GdkEvent*,  
    //hack: need to be g_malloc'ed, then is g_free'd at and of callback.
	LirenaStreamer* streamerPtr);







//following only widget-to-cam-param stuff ------------------------------------
//gboolean update_show(GtkToggleButton *show,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_gain(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_exposure(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_cy(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_cx(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_y0(GtkAdjustment *adj,  LirenaXimeaStreamer_CameraParams* cam); //gpointer)
gboolean update_x0(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams* cam);



#endif //__LIRENA_CAPTURE_DISPLAY_H__
