// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/wait.h>

#include "command.h"
//#define DEBUG

static char const *program_name;
static char const *script_name;

file_tracker_t *trackers;
int tracker_index = 0;
size_t tracker_size = 0;
int N = 128;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = 1; break;
      case 't': time_travel = 1; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;
/*
  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();
  */  
    //enter parallel limitation or not  
    if (optind != argc - 1 && time_travel == 1)
    {
        N = atoi(argv[optind]);
        optind++;
        if(N<=0)
        {
            usage();
        }
    }
    
    // There must be exactly one file argument.
    if(optind != argc -1)
        usage();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;
  
  trackers = NULL;
  tracker_index = 0;
  tracker_size = 0;

  open_db();
  	while ((command = read_command_stream (command_stream)))
    {
    	if (print_tree)
		{
		  printf ("# %d\n", command_number++);
		  int i;
		  for(i=0;i<command->arg_files->size;i++){
			  struct file f = command->arg_files->array[i];
			  printf("f name is %s, op_type is %d, position is %d, option is %s\n", f.name,f.op_type,f.position,f.option);
		  }
		  print_command (command);
		}
      	else
		{
#ifdef  DEBUG
        	printf ("# %d\n", command_number++);
#endif
	  		last_command = command;
	  		execute_command (command, time_travel);
		}	
    }
  if(time_travel){
	execute_command_list();
  }

  close_db();
  return print_tree || !last_command ? 0 : command_status (last_command);
}
