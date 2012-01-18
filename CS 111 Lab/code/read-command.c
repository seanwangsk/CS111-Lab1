// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "alloc.h"
#include <string.h>
#include <ctype.h>
#define initsize 100//some problem

enum state_type{
	NORMAL,
	IN_COMMENT,
	WAIT_FOR_AND,
	WAIT_FOR_OR
};

int isNormalChar(char);
void init_buffer(char**, char**);
void init_command_stream(command_stream_t*);
void change_last_token(command_stream_t, char*);
void add_token(command_stream_t, char*);

//convert the input file char stream into token stream, ignore the comments
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{

  command_stream_t cmdStm;
  init_command_stream(&cmdStm);   

  enum state_type state = NORMAL;

  int curLineNum = 1;

  char* buffer;
  char* ptr;
  size_t bufferSize = initsize*sizeof(char);
  init_buffer(&buffer,&ptr);

  int kkk = 0;

  char cGet;
    do{
	cGet =get_next_byte(get_next_byte_argument);
	if(state == IN_COMMENT){
		if(cGet == '\n'){
			state = NORMAL;
			add_token(cmdStm,"\n");
			curLineNum++;
		}
	}
	else{ //not in comment status
	switch(cGet)
		{
			case '#': //step add_token(cmdStm,buffer);
				add_token(cmdStm,buffer);
				init_buffer(&buffer,&ptr);
				state = IN_COMMENT;
				break;
			case '&':
				if(state == WAIT_FOR_AND){ //&&
					change_last_token(cmdStm,"&&");
					state = NORMAL;
				}
				else{
					add_token(cmdStm,buffer);
					init_buffer(&buffer,&ptr);
					add_token(cmdStm,"&");
					state = WAIT_FOR_AND;
				}
				break;
			case '|':
				if(state == WAIT_FOR_OR){ //&&
					change_last_token(cmdStm,"||");
					state = NORMAL;
				}
				else{
					add_token(cmdStm,buffer);
					init_buffer(&buffer,&ptr);
					add_token(cmdStm,"|");
					state = WAIT_FOR_OR;
				}
				break;
			case ' ':	//blank char
			case '\t':
				add_token(cmdStm,buffer);
				init_buffer(&buffer,&ptr);
				state = NORMAL;
				break;
			case '\n':
				curLineNum++;
			case '>': //special char
			case '<':
			case '(':
			case ')':
			case ';':
				add_token(cmdStm,buffer);
				init_buffer(&buffer,&ptr);
				char* tmp = checked_malloc(2*sizeof(char));
				tmp[0] = cGet;
				tmp[1] = '\0';
				add_token(cmdStm,tmp); //also add themselves
				state = NORMAL;
				break;
			case EOF:
				break;
			default: //normal word
				if(isNormalChar(cGet)||cGet=='\n'||cGet=='&'
						||cGet==';'){
					*ptr = cGet;
					ptr++;
					//check if need resize the buffer
					unsigned int offset = ptr-buffer;
					if(offset >= bufferSize/sizeof(char)){
						char* reStream = checked_grow_alloc(buffer,&bufferSize);
						buffer = reStream;
						ptr = buffer + offset;
					}
					state = NORMAL;
				}
				else{
					error(1,0,"parse error @ %d\n",curLineNum);
				}
		}
	}
  }while(cGet!=EOF);
  cmdStm->ptr = cmdStm->tokens; //init the ptr

  int i;
  for(i=0;i<cmdStm->size;i++){
  	printf("%s ",cmdStm->tokens[i]);//not work
	cmdStm->ptr++;
  }
  return cmdStm;
}

int 
isNormalChar(char c){
	        char* dict = "!%+,-./:@^_ \t";
	        int isNormal = isalnum(c);
		unsigned int i;
		for(i=0;i<strlen(dict);i++){
	                isNormal = isNormal | (c==dict[i]);
	        }
	        return isNormal;
}

void 
init_buffer(char** buffer, char**ptr){
	*buffer = checked_malloc(initsize*sizeof(char));
	*ptr = *buffer;
}

void
init_command_stream(command_stream_t* s){
	(*s) = checked_malloc(sizeof(struct command_stream));
	(*s)->tokens = checked_malloc(initsize*sizeof(char*));
	(*s)->size = 0;
	(*s)->ptr = (*s)->tokens;
	(*s)->maxsize = initsize*sizeof(char*);
}

void 
change_last_token(command_stream_t s, char* token){
	*(s->ptr-1)=token;
}

void 
add_token(command_stream_t s, char* token){
	if(strlen(token)>0){
		*(s->ptr) = token;
		s->ptr++;
		unsigned int offset = s->ptr-s->tokens;
		if(offset >= (s->maxsize)/sizeof(char*)){
			char** reStream = checked_grow_alloc(s->tokens,&(s->maxsize));
			s->tokens = reStream;
			s->ptr = s->tokens + offset;
		}
		s->size++;
	}
}



command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
<<<<<<< HEAD
    int *lineCount = 0;
    int tokenIndex = 0;
    char **tokenPtr = s->tokens;
    
    command_t buffer = checked_malloc(sizeof(struct command));
    
    while (tokenIndex < s->size)
    {
        printf("test\n");
            
        int charIndex = 0;
        while (charIndex < strlen(tokenPtr[tokenIndex]))
        {
            printf("%c\n",tokenPtr[tokenIndex][charIndex]);
            
            if (tokenPtr[tokenIndex][charIndex] == '\n') //calculate the line number
                *lineCount++;
            
            else if (tokenPtr[tokenIndex][charIndex] == '&') //&& AND Commands
          //          if (peek(tokenPtr[tokenIndex][charIndex])== '&')
                    
                            
                    
            
            
            
            charIndex++;
        }
        
        tokenIndex++;
    }

    
    
  error (1, 0, "command reading not yet implemented");
=======
>>>>>>> 6d3114550ad85acd95f321e88ee588b0a64023cf
  return 0;
}

/*
command_t 
parseCommand()
{
    
}
*/

char 
peekChar (char * ptr)
{
    char *tmp = ptr;
    tmp++;
    return *tmp;
}
 

int 
expectChar (char * ptr, char exp)
{
    ptr++;
    if (*ptr == exp)
        return 1;
    else
        return 0;
}

/*
int
acceptChar (char * ptr)
{
    if(( *ptr >= 65 && *ptr <= 90) | (*ptr >= 97 && *ptr <= 122) | (*ptr >= 48 && *ptr <= 58) | *ptr == '!' | *ptr == '%' | *ptr == '+' | *ptr == ',' | *ptr == '-' | *ptr == '.' | *ptr == '/' | *ptr == ':')
        return 1;
    else
        return 0;
}
 */
//command_t read_sim_command ()
//command_t read_subshell (command_t buffer, int * lineCount, command_stream_t s, )
//command_t read_link_command (command_t buffer, int * lineCount, command_stream_t s, )

