#ifndef __LIRENA_KLV_APP_SRC_H__
#define __LIRENA_KLV_APP_SRC_H__


#include <gst/app/gstappsrc.h>



// ---------------------------------------------------------------------------
//{ KLV experiments
//endianess converting (for KLV metadata)
#include <stdint.h>   // For 'uint64_t'
#include <endian.h>   // htobe64  be64toh (host to big endian 64)

#define KLV_KEY_SIZE_BYTES     8
#define KLV_LENGTH_SIZE_BYTES  8

// don't know if low keys are reserved or forbidden or sth...
#define KLV_KEY_OFFSET 0
enum Lirena_KLV_keys
{
    KLV_KEY_invalid  = 0 + KLV_KEY_OFFSET,
    KLV_KEY_string   = 1 + KLV_KEY_OFFSET,
    KLV_KEY_uint64   = 2 + KLV_KEY_OFFSET,
    KLV_KEY_float64  = 3 + KLV_KEY_OFFSET,
    
    KLV_KEY_camera_ID = 4 + KLV_KEY_OFFSET,
    
    KLV_KEY_image_capture_time_stamp  = 5 + KLV_KEY_OFFSET,
    KLV_KEY_navigation_data_time_stamp  = 6 + KLV_KEY_OFFSET,

	// enumerating all camera's image frames that are captured and streamed
    KLV_KEY_image_captured_frame_number  = 7 + KLV_KEY_OFFSET,
    // enumarting each data packet received from navigation computer
    KLV_KEY_navigation_data_frame_number = 8 + KLV_KEY_OFFSET,
	// enumerating all images that have been taken by the camera
	// (but not neccessarily captured by the app; 
	//  If synchronized triggering of all cameras works reliably,
	//  this number provides unambiguous frame information for later stitching)
    KLV_KEY_image_camera_frame_number  = 9 + KLV_KEY_OFFSET,

    KLV_KEY_navigation_data_payload = 10 + KLV_KEY_OFFSET,
    KLV_KEY_image_statistics = 11 + KLV_KEY_OFFSET,
    
    KLV_KEY_num_keys = 12
};

//forward:
struct LirenaFrame;

GstFlowReturn lirena_KLV_appsrc_CollectAndPushFrameMetaData(
    GstElement * appsrc_klv,
    LirenaFrame* frame
);


// Converts relevant values to big endian and writes a KLV triple
// return pointer points at the byte right after the area 
//  the func has written to: 
//  valuePtr + KLV_KEY_SIZE_BYTES + KLV_LENGTH_SIZE_BYTES + len_host
guint8 * 
write_KLV_item(
    guint8 * klv_buffer,
    uint64_t key_host,
    uint64_t len_host, 
    guint8 const * value_ptr);

// ---------------------------------------------------------------------------
//} KLV experiments




#endif //__LIRENA_KLV_APP_SRC_H__
