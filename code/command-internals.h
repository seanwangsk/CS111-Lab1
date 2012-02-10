// UCLA CS 111 Lab 1 command internals
#include <stdlib.h>
typedef struct command_stream *command_stream_t;
typedef struct command *command_t;
typedef struct command_unit *command_unit_t; 
typedef struct file_tracker *file_tracker_t;
typedef struct cmd_queue* cmd_queue_t;
typedef struct file* file_t;
typedef struct file_array* file_array_t;
typedef struct cmd_option* cmd_option_t;


struct file
{
	char* name;
	int op_type;//0 for read, 1 for write, 2 for unknown
	//SK: New here for analyzing
	int position;
	char* option; 
};

struct file_array{
	file_t array;
	int size;
};


struct cmd_option
{
	char* name;
	int reqarg;//0 as no, 1 as yes
	int op; //0 as read, 1 as write, 2 as not applicable(if take no arg)
	int known;
};

enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  int cmdId;
  file_array_t arg_files;
  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;
    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
    
};


struct command_stream
{
	char** tokens;
	unsigned int size; //total # of tokens
	size_t maxsize;
	unsigned int ptrIndex;
	int curLineNum;
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

file_t create_file(char* name);
cmd_option_t create_new_cmd_option(char* name);
command_t create_command(void);
command_stream_t create_command_stream(void);
file_array_t create_file_array(int maxsize);
void add_arg_file(command_t cmd, struct file file);
