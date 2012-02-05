// UCLA CS 111 Lab 1 command interface
#include <stdlib.h>
typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

typedef struct command_unit *command_unit_t; 
typedef struct file_tracker *file_tracker_t;
typedef struct cmd_queue* cmd_queue_t;
typedef struct file* file_t;
typedef struct file_array* file_array_t;

struct file
{
	char* name;
	int op_type;//0 for read, 1 for write
};

struct file_array{
	file_t array;
	int size;
};

struct command_unit
{
    command_t cmd;
    int block;                  //indicate how many files are still blocking
    file_array_t dependFiles;
};


struct file_tracker
{
    char* fileName;
    int writing;               //indicate how many command are reading or writing this file
    int reading;   
    struct cmd_queue *q_head;
};

struct cmd_queue
{
    command_unit_t cmd_unit;
    pid_t pid;
    int cmdNum;
    int type; //0-read 1-write
    struct cmd_queue *next;
};

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);

//for parallel excution




