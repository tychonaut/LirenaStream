// Parse CLI options:
// use global variable "globalAppArgs"

#ifndef __LIRENA_OPTIONS_H__
#define __LIRENA_OPTIONS_H__


#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


struct LirenaCamStreamConfig
{
  bool doLocalDisplay;
  //int silent, verbose;
  char const *output_file;

  char const *IP;
  char const *port;
  
  int exposure_ms;
};

//{ forwards:
LirenaCamStreamConfig parseArguments(int argc, char **argv);
//}




#endif //__LIRENA_OPTIONS_H__