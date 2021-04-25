


#include "LirenaKLVappsrc.h"


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
