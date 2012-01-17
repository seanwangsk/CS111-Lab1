// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "alloc.h"
#include <string.h>

enum state_type{
	NORMAL,
	IN_COMMENT,
	IN_SPECIAL,
	IN_SUSPEND,
}

struct parse_status{
	enum state_type state;
	char suspend;
	command_t cmd_buffer;
}


//question about \"
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{

  command_stream_t cmdStm  = checked_malloc(sizeof(struct command_stream));
  
  size_t stmSz = 100;//initial size set as 100

  struct parse_state status;
  status.state = NORMAL;

  int curLineNum = 1;

  char* buffer = checked_malloc(stmSz);
  char* ptr = buffer;

  char cGet;

  do{
	cGet =get_next_byte(get_next_byte_argument);
	if(status.state == IN_COMMENT){
		if(cGet == '\n'){
			status.state = NORMAL;
		}
	}
	else{ //not in comment status
		case '#': //step into comments
			current end
			status.state = IN_COMMENT;
			break;
		case '\n':
			if before is not special mark,current end
			else
			just ignore
			curLineNum++;
			break;
		case ';'
		case '':
			current end
			put into buffer
			status into SPECIAL
			break;
		case normalword
			string_add(buffer,ptr,cGet);
		DEFAULT:
			error(1,0,"parse error @ \d",curLineNum);
	}
  }while(cGet!=EOF);
  printf("%s",cmdStm->stream);
  return cmdStm;
}

void string_add(char* string, char* ptr, char cGet){
	*ptr = cGet;
		ptr++;
		//check if need resize the buffer
		unsigned int offset = ptr-cmdStm->stream;
		if(offset >= stmSz){
			char* reStream = checked_grow_alloc(cmdStm->stream,&stmSz);
			cmdStm->stream = reStream;
			//printf("s is %d, offset is %d\n",s,offset);
			ptr = cmdStm->stream + offset;
		}
}

void
skip_until (int (*get_next_byte) (void *),
           void *get_next_byte_argument,
	   char keyword)
{
	char cGet;
	do{
		cGet =get_next_byte(get_next_byte_argument);
	}while(cGet != keyword || cGet !=EOF)
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
