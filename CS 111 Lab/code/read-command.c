// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "alloc.h"
#include <string.h>

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
  //error (1, 0, "command reading not yet implemented");

   [Modified by SK] Read the whole file into a char*,  overflow safe*/
  command_stream_t cmdStm  = checked_malloc(sizeof(struct command_stream));
  
  size_t stmSz = 100;//initial size set as 100
  
  cmdStm->stream = checked_malloc(stmSz);
  char * ptr = cmdStm->stream;
  int cGet;
  do{
	cGet =get_next_byte(get_next_byte_argument);
	if(cGet == '#'){ //check comment
		skip_until(get_next_byte,get_next_byte_argument,'\n');
	}
	else{
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
  }while(cGet!=EOF);
  printf("%s",cmdStm->stream);
  return cmdStm;
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
