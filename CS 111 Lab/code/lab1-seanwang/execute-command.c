// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <error.h>
#include <unistd.h>

int
command_status (command_t c)
{
  return c->status;
}

int
exec_cmd (command_t c){
	if(c->type == AND_COMMAND){
		return exec_cmd(c->u.command[0]) && exec_cmd(c->u.command[1]);		
	}
	else if(c->type == OR_COMMAND){
		return exec_cmd(c->u.command[0]) || exec_cmd(c->u.command[1]);
	}
	else if(c->type == SEQUENCE_COMMAND){
		exec_cmd(c->u.command[0]);
		return exec_cmd(c->u.command[1]);
	}
	else if(c->type == PIPE_COMMAND){
		//need to do something here
	}
	else if(c->type == SUBSHELL_COMMAND){
		return exec_cmd(c->u.subshell_command);
	}
	else if(c->type == SIMPLE_COMMAND){
		
	}
	else{
		error(1,0,"cannot recognize command type as %d\n",c->type);
	}
	return 1;
}

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	if(time_travel == 0){ //normal mode
			
	}

}


