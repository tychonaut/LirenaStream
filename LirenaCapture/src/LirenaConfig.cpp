

#include "LirenaConfig.h"




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
static struct argp_option options[] = {
  //{"verbose",     'v', 0,         0,  
  //  "Produce verbose output" },
  {"localdisplay",  'l', 0,         0, 
    "For debugging :display video locally, (not just stream per UDP)" },
  {"output",        'o', "FILE",    0, 
    "For debugging: Dump encoded stream to disk" }, //OPTION_ARG_OPTIONAL
  {"useTCP",        't', 0,       0, 
    "Experimental (may not work!): Use TCP instead of UDP " }, //OPTION_ARG_OPTIONAL
  {"exposure",      'e', "time_ms", 0, 
    "Exposure time in milliseconds" },
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

    case 'l':
      myArgs->doLocalDisplay = true;
      break;
    case 'o':
      myArgs->outputFile = arg;
      printf("%s", "Warning: local dump to disk currently not supported.\n");
      break;
    case 't':
      myArgs->useTCP = true;
      break;

    case 'e':
      myArgs->ximeaparams.exposure_ms = atoi(arg);
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



LirenaConfig parseArguments(int argc, char **argv)
{
	LirenaConfig ret;
   
    // Default values.
    ret.IP = "192.168.0.169";
    ret.port = "5001";
    ret.doLocalDisplay = false;
    ret.outputFile = nullptr;
    ret.useTCP = false;
    ret.ximeaparams.exposure_ms = 30;
    
    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, &ret);

    printf ("IP = %s\n"
            "Port = %s\n"
            "Exposure = %i\n"
            "display locally = %s\n"
            "use TCP instead UDP = %s\n"
            "OUTPUT_FILE = %s\n",
            ret.IP, 
            ret.port,
            ret.ximeaparams.exposure_ms,
            ret.doLocalDisplay ? "yes" : "no",
            ret.useTCP ? "yes" : "no",
            ret.outputFile ? ret.outputFile : "-none-"
    );

	return ret;
}

