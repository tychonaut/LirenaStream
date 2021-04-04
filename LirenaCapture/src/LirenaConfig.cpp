

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
  //{"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"localdisplay",  'l', 0,      0,  "display video locally, (not just stream per UDP)" },
  {"output",   'o', "FILE", 0, "Dump encoded stream to disk" }, //OPTION_ARG_OPTIONAL
  {"exposure", 'e', "time_ms", 0, "Exposure time in milliseconds" },
  { 0 }
};



// Parse a single argument/option.
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  // Get the input argument from argp_parse, which we
  //   know is a pointer to our arguments structure.
  struct LirenaCamStreamConfig *myArgs = 
    (struct LirenaCamStreamConfig *) state->input;

  switch (key)
    {
    case 'l':
      myArgs->doLocalDisplay = true;
      break;
    case 'o':
      myArgs->output_file = arg;
      printf("%s", "Warning: local dump to disk currently not supported.\n");
      break;
    case 'e':
      myArgs->exposure_ms = atoi(arg);
      break;

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



LirenaCamStreamConfig parseArguments(int argc, char **argv)
{
	LirenaCamStreamConfig ret;
   
    // Default values.
    ret.doLocalDisplay = false;
    ret.output_file = nullptr;
    ret.exposure_ms = 30;
    ret.IP = "192.168.0.169";
    ret.port = "5001";

    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, &ret);

    printf ("IP = %s\nPort = %s\nExposure = %i\n"
            "OUTPUT_FILE = %s\ndisplay locally = %s\n",
            ret.IP, 
            ret.port,
            ret.exposure_ms,
            ret.output_file ? ret.output_file : "-none-",
            ret.doLocalDisplay ? "yes" : "no");

	return ret;
}

