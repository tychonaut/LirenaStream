

#include "LirenaConfig.h"

#include <cstring> //strcmp


const char *argp_program_version =
  "ximeaJetsonStream 0.2.0";
const char *argp_program_bug_address =
  "<mschlueter@geomar.de>";

//  Program documentation. 
static char doc[] =
  "Capture and stream video from Ximea camera on Jetson using GStreamer";

//  A description of the arguments we accept. 
static char args_doc[] = "IP Port";


//  The options we understand. 
static struct argp_option options[] = 
{
  {"captureDeviceType",  'd', "TYPE",    0, 
    "Ximea (camera, default), MagewellEco (capture card)" }, //OPTION_ARG_OPTIONAL

  {"localdisplay",  'l', 0,         0, 
    "For debugging :display video locally, (not just stream per UDP)" },
  {"localGUI",      'g', 0,         0, 
    "For debugging :show GUI to control params (not yet supported for each device)" },
  
  {"output",        'o', "FILE",    0, 
    "For debugging: Dump encoded stream to disk" }, //OPTION_ARG_OPTIONAL
  
  {"targetFPS",      'f', "fps", 0, 
    "target fps; may be ignored depending on capture device&mode; default:-1(auto)" },

  {"targetResolutionX",      'x', "resX", 0, 
    "target resolution; may be ignored depending on capture device&mode; default:-1(auto)" },
  {"targetResolutionX",      'y', "resY", 0, 
    "target resolution; may be ignored depending on capture device&mode; default:-1(auto)" },

  {"useTCP",        't', 0,       0, 
    "Experimental (may not work!): Use TCP instead of UDP " }, //OPTION_ARG_OPTIONAL

  
  {"exposure",      'e', "time_ms", 0, 
    "Exposure time in milliseconds" },

  {"useCudaDemosaic",      'c', 0, 0, 
    "Use cuda for demoaicing of Ximea cam (instead of gstreamer software impl.)" },

  { 0 }
};



// Parse a single argument/option.
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  // Get the input argument from argp_parse, which we
  //   know is a pointer to our arguments structure.
  struct LirenaConfig *myArgs = 
    (struct LirenaConfig *) state->input;

  switch (key)
    {
    // non-optional mandatory arguments (order dependent)
    case ARGP_KEY_ARG:
      if (state->arg_num >= 2)
      {
        // Too many arguments.
        argp_usage (state);
      }
      if(state->arg_num == 0)
      {
        myArgs->IP = arg;
      }
      else
      {
        myArgs->port = arg;
      }
      break;

    //  keys to parse: dlgofxytec

    case 'd':
      if( strcmp(arg, "Ximea") == 0 )
      {
        myArgs->captureDeviceType = LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera;
      }
      else if( strcmp(arg, "MagewellEco") == 0 )
      {
         myArgs->captureDeviceType = LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture;
      }
      else {
        myArgs->captureDeviceType = LIRENA_CAPTURE_DEVICE_TYPE_invalid;
        printf("ERROR: unknown capture device type: %s ", arg);
        argp_usage (state);
      }
      break;

    case 'l':
      myArgs->doLocalDisplay = true;
      break;
    case 'g':
      myArgs->haveLocalGUI = true;
      break;

    case 'o':
      myArgs->outputFile = arg;
      break;

    case 'f':
      myArgs->targetFPS = atoi(arg);
      break;
    case 'x':
      myArgs->targetResolutionX = atoi(arg);
      break;
    case 'y':
      myArgs->targetResolutionY = atoi(arg);
      break;

    case 't':
      myArgs->useTCP = true;
      break;

    case 'e':
      myArgs->ximeaparams.exposure_ms = atoi(arg);
      break;
    
    case 'c':
      myArgs->ximeaparams.useCudaDemosaic = true;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2)
      {
        // Not enough arguments.
        argp_usage (state);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };



//parseArguments
LirenaConfig::LirenaConfig(int argc, char **argv)
  : argc(argc),
    argv(argv)
{
	//LirenaConfig ret;
   
    //{ Default values:

    this->captureDeviceType = LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera;

    this->IP = "192.168.0.169";
    this->port = "5001";

    this->targetResolutionX = -1;
    this->targetResolutionY = -1;
    this->targetFPS = -1;

    this->useTCP = false;

    this->doLocalDisplay = false;
    this->haveLocalGUI = false;

    this->outputFile = nullptr;

    this->ximeaparams.exposure_ms = 30;
    this->ximeaparams.useCudaDemosaic = false;
    //}


    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, this);


    if(this->haveLocalGUI && 
      this->captureDeviceType != LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera)
    {
      printf("%s", "Error: GUI is currently only supported for Ximea device! Disabling GUI option...\n");
      this->haveLocalGUI = false;
    }

    // dlgofxytec
    printf ("IP = %s\n"
            "Port = %s\n"
            "Capture device type: %s\n"
            "Target FPS = %i\n"
            "Target Resolution = (%i) x (%i)\n"
            "display locally = %s\n"
            "local GUI = %s\n"
            "output file = %s\n"
            "use TCP instead UDP = %s\n"
            "Exposure = %i\n"
            "use Cuda demosaic (instead of GStreamer software impl.) = %s\n",

            this->IP, 
            this->port,

            this->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera ?
              "Ximea (camera)" 
              :
              this->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture ?
              "MagewellEco (capture card)" 
              : "unsupported device type (internal error!)"
              ,

            this->targetFPS,
            this->targetResolutionX, this->targetResolutionY,

            this->doLocalDisplay ? "yes" : "no",
            this->haveLocalGUI ? "yes" : "no",

            this->outputFile ? this->outputFile : "-none-",

            this->useTCP ? "yes" : "no",

            this->ximeaparams.exposure_ms,
            this->ximeaparams.useCudaDemosaic ? "yes" : "no"
    );
}

