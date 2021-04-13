


#include "LirenaKLVappsrc.h"

#include "LirenaStreamer.h"


// Converts relevant values to big endian and writes a KLV triple
// return pointer points at the byte right after the area 
//  the func has written to: 
//  valuePtr + KLV_KEY_SIZE_BYTES + KLV_LENGTH_SIZE_BYTES + len_host
guint8 * 
write_KLV_item(
    guint8 * klv_buffer,
    uint64_t key_host,
    uint64_t len_host, 
    guint8 const * value_ptr)
{   
    uint64_t key_be = htobe64(key_host);
    memcpy(klv_buffer, &key_be, KLV_KEY_SIZE_BYTES);
    klv_buffer += KLV_KEY_SIZE_BYTES;
            
    uint64_t len_be = htobe64(len_host);
    memcpy(klv_buffer, &len_be, KLV_LENGTH_SIZE_BYTES);
    klv_buffer += KLV_LENGTH_SIZE_BYTES; 
            
   memcpy(klv_buffer, value_ptr, len_host);
   klv_buffer += len_host;
   
   return klv_buffer;
}



GstFlowReturn lirena_KLV_appsrc_CollectAndPushFrameMetaData(
    GstElement * appsrc_klv,
    LirenaFrame* frame
)
{ 
    unsigned long frame_capture_PTS =
            // Gstreamer convention: Presentation timetamp (PTS) =
        //   current abs time - abs time of stream start
        frame->metaData.timingData.current_abs_time -
            frame->metaData.timingData.lastStreamStart_abs_time;

    double avg_fps =  
        frame->metaData.timingData.current_captured_frame_count
        /
        (
            1e-9 * (double)(frame_capture_PTS)
        );


    //format for KLV, aligned to 16 bytes
    gchar statistics_string[LIRENA_STATUS_STRING_MAX_LENGTH];
    g_snprintf(
        statistics_string,
        LIRENA_STATUS_STRING_MAX_LENGTH,
        //0123456789ABCDEF
        "0123456789ABCDE\n"
        "Acquisition: [ \n"
        "frames captr'd:\n"
        "%015lu\n"
        "frames skipped:\n"
        "%015lu\n"
        "PTS  timestamp:\n"
        "%015lu\n"
        "abs. timestamp:\n"
        "%015lu\n"
        "avg. fps:      \n"
        "%015.2f\n"
        "]              \n",
        //"normal frame counter"
        frame->metaData.timingData.current_captured_frame_count,
        //"sensor frame counter" - "normal frame counter" = num lost frames
        frame->metaData.timingData.current_sensed_frame_count -
            frame->metaData.timingData.current_captured_frame_count,
        frame_capture_PTS,
        frame->metaData.timingData.current_abs_time,
        avg_fps
       );

    GstFlowReturn ret = GST_FLOW_OK;

    // hundred thousand bytes
    #define KLV_BUFFER_MAX_SIZE 100000
    guint8 klv_data[KLV_BUFFER_MAX_SIZE];
    memset(klv_data, 0, KLV_BUFFER_MAX_SIZE);

    guint8 *klv_data_end_ptr = &klv_data[0];

    // Write string  "0123456789ABCDEF"
    gchar const *test16byteString = "0123456789ABCDEF";
    klv_data_end_ptr = write_KLV_item(
        klv_data_end_ptr,                // guint8 * klv_buffer,
        KLV_KEY_string,                  // uint64_t key_host,
        strlen(test16byteString),        // uint64_t len_host,
        (guint8 const *)test16byteString // guint8 * value_ptr
    );

    // write statistics_string as sth human readable
    g_assert("stat. string is multiple of 16 bytes/chars" &&
             strlen(statistics_string) % 16 == 0);

    klv_data_end_ptr = write_KLV_item(
        klv_data_end_ptr,                 // guint8 * klv_buffer,
        KLV_KEY_string,                   // uint64_t key_host,
        strlen(statistics_string),        // uint64_t len_host,
        (guint8 const *)statistics_string // guint8 * value_ptr
    );

    //TODO write GST clock -derived timestamps also as
    // binary big endian 64bit unsiged int!

    // Write frame count KLV_KEY_image_frame_number
    uint64_t value_be = htobe64((uint64_t)
        frame->metaData.timingData.current_captured_frame_count);
    klv_data_end_ptr = write_KLV_item(
        klv_data_end_ptr,                    // guint8 * klv_buffer,
        KLV_KEY_image_captured_frame_number, // uint64_t key_host,
        sizeof(uint64_t),                    // uint64_t len_host,
        (guint8 const *)&value_be            // guint8 * value_ptr
    );

    // Write frame timestamp KLV_KEY_image_capture_time_stamp
    value_be = htobe64((uint64_t)frame_capture_PTS);
    klv_data_end_ptr = write_KLV_item(
        klv_data_end_ptr,                 // guint8 * klv_buffer,
        KLV_KEY_image_capture_time_stamp, // uint64_t key_host,
        sizeof(uint64_t),                 // uint64_t len_host,
        (guint8 const *)&value_be         // guint8 * value_ptr
    );

    // Write string "!frame meta end!"
    gchar const *frameMetaEndString = "!frame meta end!";
    klv_data_end_ptr = write_KLV_item(
        klv_data_end_ptr,                  // guint8 * klv_buffer,
        KLV_KEY_string,                    // uint64_t key_host,
        strlen(frameMetaEndString),        // uint64_t len_host,
        (guint8 const *)frameMetaEndString // guint8 * value_ptr
    );

    //TODO maybe write statistics_string

    guint klvBufferSize = klv_data_end_ptr - (&klv_data[0]);

    // create empty GstBuffer
    GstBuffer *klv_frame_GstBuffer = gst_buffer_new();

    // alloc memory for buffer
    GstMemory *gstMem = gst_allocator_alloc(NULL, klvBufferSize, NULL);

    // add the buffer
    gst_buffer_append_memory(klv_frame_GstBuffer, gstMem);

    // get WRITE access to the memory and fill with our KLV data
    GstMapInfo gstInfo;
    gst_buffer_map(klv_frame_GstBuffer, &gstInfo, GST_MAP_WRITE);

    //do the writing
    memcpy(gstInfo.data, klv_data, gstInfo.size);

    gst_buffer_unmap(klv_frame_GstBuffer, &gstInfo);

    // Timestamp stuff onto KLV buffer itself to get associated
    // with video buffer:
    //GST_BUFFER_TIMESTAMP(klv_frame_GstBuffer) = GST_CLOCK_TIME_NONE;
    //GST_BUFFER_PTS(klv_frame_GstBuffer) = frame_capture_PTS;

    GST_BUFFER_PTS(klv_frame_GstBuffer) = frame_capture_PTS;
    //GST_BUFFER_DURATION (klv_frame_GstBuffer) = myDuration;

    // This function takes ownership of the buffer. so no unref!
    ret = gst_app_src_push_buffer(
        GST_APP_SRC(appsrc_klv),
        klv_frame_GstBuffer);

    if (ret != GST_FLOW_OK)
    {
        g_assert(0 && "pushing KLV buffer to appsrc failed!");
    }

    return ret;
}