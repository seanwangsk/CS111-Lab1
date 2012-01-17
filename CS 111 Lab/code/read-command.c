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
	WAIT_FOR_AND;
	WAIT_FOR_OR;
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
		case ';':
			current end
		case '&':
			if(status.state = WAIT_FOR_AND){ //&&
				status.state = NORMAL;
				*(ptr-1)='\0'; //elimate the former &
				if(status.cmd_buffer!=NULL){
					status.cmd_buffer.command[1] = curCmd
				}
			        put into buffer
			        status into SPECIAL
			}
			else{
				string_add(buffer,ptr,cGet);
			}
			break;
		DEFAULT:
			if(isNormalChar(cGet)){
				string_add(buffer,ptr,cGet);
			}
			else{
				error(1,0,"parse error @ \d",curLineNum);
			}
	}
  }while(cGet!=EOF);
  printf("%s",cmdStm->stream);
  return cmdStm;
}

command_t init_simple_command(command_t cmd){
	cmd = checked_alloc(sizeof(struct command));
  	cmd.type = SIMPLE_COMMAND;
	cmd.word =  
  	char*  = checked_malloc(stmSz);
}


void :
current end

int isNormalChar(char c){
	        char* dict = "!%+,-./:@^_ \t";
	        int isNormal = isalnum(c);
		printf("isNormal result is %d\n",isNormal);
		int i;
		for(i=0;i<strlen(dict);i++){
	                isNormal = isNormal | (c==dict[i]);
			printf("with %c result is %d\n",dict[i],isNormal);
	        }
	        return isNormal;
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
