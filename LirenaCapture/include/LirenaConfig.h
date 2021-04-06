#ifndef __LIRENA_CONFIG_H__
#define __LIRENA_CONFIG_H__


// Parse CLI options:

#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


struct LirenaConfig
{
  // host and IP are mandatory params; host's IP has different
  // meaning in TCP than in UDP.
  char const *IP;
  char const *port;

  // FOR DEBUGGING ONLY! Slows down pipeline, 
  // SEVERELY messing up UDP streaming to the point
  // of gstreamer refusing playback of dumped file
  // and even worse, on lost video frames possibly
  // destroying the precious 1:1 association of
  // video frames <--> KLV buffers !!
  // So REALLY ONLY USE FOR DEBUGGING, NOT IN PRODUCTION!!
  // default: false
  bool doLocalDisplay;

  // FOR DEBUGGING ONLY! Same warning as with 
  // default: NULL
  char const *outputFile;
  
  // experimental, in case UDP makes too many problems with 
  // corrupt/lost frames, possibly prohibiting video replay
  // and especially destroying the implicit 1:1 association 
  // of video frames and KLV buffers (they are treated as 
  // independent entities by the current implementation
  // of mpegtsmux and tsdemux!)
  // default: false
  bool useTCP; 

  //int silent, verbose;
  


  
  //may be ignored in final implementation;
  // currently it is mostly a hacky way of 
  // roughly controlling capture framerate
  int exposure_ms;
};

//{ forwards:
LirenaConfig parseArguments(int argc, char **argv);
//}




#endif //__LIRENA_CONFIG_H__
