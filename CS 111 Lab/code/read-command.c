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

#define Debug 0 //for Debug printf

enum token_state{
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

//command_t parseSubshell(command_stream_t);
//command_t parse_Command(command_stream_t, int);


//convert the input file char stream into token stream, ignore the comments
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{

  command_stream_t cmdStm;
  init_command_stream(&cmdStm);   

  enum token_state state = NORMAL;

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
				if(isNormalChar(cGet)||cGet==';'){
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
  cmdStm->curLineNum = 1;
  int i;
#ifdef Debug
  for(i=0;i<cmdStm->size;i++){
  	printf("%s ",cmdStm->tokens[i]);//not work
  }
#endif
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



enum command_state{
	SIMPLE_INIT,
	SIMPLE_NO,
	SIMPLE_INPUT,
	SIMPLE_INPUT_FINISH,
	SIMPLE_OUTPUT,
	SIMPLE_OUTPUT_FINISH,
    SUBSHELL_FINISH,
};

int 
isEqual(char* a, char* b)
{
	int result = strcmp(a,b)?0:1;
	return result;
}

int 
isNormalToken(char* token)
{
	return (!isCombineToken(token) 
		&& !isEqual(token,"<") 
		&& !isEqual(token,">") 
		&& !isEqual(token,"\n") 
		&& !isEqual(token,";") 
		&& !isEqual(token,"(") 
		&& !isEqual(token,")"));
}

int 
isCombineToken(char* token)
{
	return (isEqual(token,"|") ||
		isEqual(token,"||") || 
		isEqual(token,"&&") || 
		isEqual(token,";"));
}
command_t push_command_buffer(command_t, char*);

command_t parse_Command(command_stream_t, int);

command_t 
parse_subshell(command_stream_t s){
#ifdef Debug
    printf("get into subshell\n");
#endif
    command_t subCmd = checked_malloc(sizeof(struct command));
    subCmd->type = SUBSHELL_COMMAND;
    subCmd->u.subshell_command = parse_Command(s, 1);
#ifdef Debug
    printf("get out of subshell\n");
#endif
    return subCmd;
}

command_t
parse_Command(command_stream_t s, int isSub)
{
#ifdef Debug     
    printf("get into parse_command and isSubshell = %d\n",isSub);
#endif
    command_t curCmd = checked_malloc(sizeof(struct command));
    curCmd->type = SIMPLE_COMMAND;
    command_t cmdBuffer = NULL;
    
    
    unsigned int curCmdWordIndex;
    size_t curCmdWordMax;
    
    
    enum command_state state = SIMPLE_INIT;   
    int i;
    
    int curOffset = (s->ptr - s->tokens);
    
    for(i=curOffset; i < s->size;i++){
        s->ptr++;
        
        char* token = s->tokens[i];
        
#ifdef Debug     
        printf("#%d token is %s\n",s->curLineNum,token);
        printf("i index = %d\n",i);
        printf("curOffset = %d\n",curOffset);
#endif
        
        if(isEqual(token,"\n")){
            s->curLineNum++;
        }
        switch(state){
            case SIMPLE_INIT:
#ifdef Debug   
                printf("simple_init\n");
#endif
                if(isEqual(token,"(")){//XIA: for subshell
                    curCmd = parse_subshell(s);
#ifdef Debug  
                    printf("Offset Before reset: %d\ni before reset: %d\n",curOffset,i);
#endif
                    
                    curOffset = (s->ptr - s->tokens);/////////////////////
                    i = curOffset - 1;/////////////////////////XIA: to jump directly to the last token enter

#ifdef Debug  
                    printf("Offset Reset to %d\ni  reset to: %d\n",curOffset,i);
#endif
                    if(i == s->size) // if ")" is the last token of the script  then return curCmd directly;
                    {
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
                        }
                        return curCmd;
                    }
                    state = SUBSHELL_FINISH;
                }
                else if(isEqual(token, "\n")){
                    state = SIMPLE_INIT;	
                }
                else if(isNormalToken(token)){
                    curCmd->u.word = checked_malloc(initsize*sizeof(char*));
                    curCmdWordMax = initsize * sizeof(char*);
                    curCmd->u.word[0] = token;
                    curCmdWordIndex = 1; 
                    state = SIMPLE_NO;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                } 
                break;
            case SIMPLE_NO:
#ifdef Debug
                printf("SIMPLE_NO\n");
                printf("==%s==\n",token);
#endif
                //XIA: for subshell
                if((isEqual(token,")"))&&(isSub != 0)){
                    
                    if(cmdBuffer != NULL){
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
#ifdef Debug
                    printf("returning from subshell from SIMPLE_NO\n");
#endif
                    
                    return curCmd; 
                }
                else if(isEqual(token,"\n")){
                    //XIA: for subshell
                    if(isSub != 0){
                        //ignore \n
                    }
                    else
                    {
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
#ifdef Debug 
                            printf("the second is %d\n",curCmd->u.command[1]->type);
#endif
                        }
                        state = SIMPLE_INIT;
                        return curCmd;
                    }
                }
                else if(isEqual(token,"<")){
                    state = SIMPLE_INPUT;
                }
                else if(isEqual(token, ">")){
                    state = SIMPLE_OUTPUT;
                }
                else if(isNormalToken(token)){
#ifdef Debug                
                    printf("normal word in SIMPLE_NO\n");
#endif
                    state = SIMPLE_NO;
                    if(curCmdWordIndex >= curCmdWordMax){
                        curCmd->u.word = checked_grow_alloc(curCmd->u.word,&curCmdWordMax);
                    }
                    curCmd->u.word[curCmdWordIndex] = token;
                    curCmdWordIndex++;
                }
                else if(isCombineToken(token)){
#ifdef Debug
                    printf("combining in SIMPLE_NO\n");
#endif
                    if(cmdBuffer != NULL){
                        //printf("not null\n");
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
                    //printf("push\n");
                    //printf("current command %s\n",curCmd->u.word[0]);
                    cmdBuffer = push_command_buffer(curCmd, token);
                    //printf("after\n");
                    curCmd = checked_malloc(sizeof(struct command));
                    curCmd->type = SIMPLE_COMMAND;
                    //printf("cmd Buffer %s\n",cmdBuffer->u.command[0]->u.word[0]);
                    state = SIMPLE_INIT;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
            case SIMPLE_INPUT:
#ifdef Debug            
                printf("input\n");
#endif
                if(isNormalToken(token)){
                    curCmd->input = token;
                    state = SIMPLE_INPUT_FINISH;	
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
            case SIMPLE_OUTPUT:
#ifdef Debug            
                printf("output\n");
#endif
                if(isNormalToken(token)){
                    curCmd->output = token;
                    state = SIMPLE_OUTPUT_FINISH;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
            case SIMPLE_INPUT_FINISH:
#ifdef Debug
                printf("input finish\n");
#endif
                if(isEqual(token,">")){
                    state = SIMPLE_OUTPUT;
                }
                else if(isEqual(token,"\n")){
                    //XIA: for subshell
                    if(isSub == 0 )
                    {
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
#ifdef Debug
                            printf("the second is %d\n",curCmd->u.command[1]->type);
#endif
                        }
                        return curCmd;
                        state = SIMPLE_INIT;
                    }
                    else
                    {
                        
                    }
                }
                else if(isCombineToken(token)){
                    if(cmdBuffer != NULL){
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
                    cmdBuffer = push_command_buffer(curCmd, token);			
                    curCmd = checked_malloc(sizeof(struct command));
                    curCmd->type = SIMPLE_COMMAND;
                    state = SIMPLE_INIT;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
            case SIMPLE_OUTPUT_FINISH:
#ifdef Debug
                printf("output finish\n");
#endif
                //XIA: for subshell
                
                if(isEqual(token,"\n")){
                    if(isSub == 0){
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
                            //printf("now  %d\n",curCmd->u.command[1]->type);
                        }
                        state = SIMPLE_INIT;
                        return curCmd;
                    }
                    else
                    {
                    
                    
                    }
                }

                else if(isCombineToken(token)){
                    
                    
                    
                    if(cmdBuffer != NULL){
                        //printf("not null2\n");
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
                    cmdBuffer = push_command_buffer(curCmd, token);
                    
                    
                    curCmd = checked_malloc(sizeof(struct command));
                    curCmd->type = SIMPLE_COMMAND;
                    state = SIMPLE_INIT;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
                
            case SUBSHELL_FINISH:
#ifdef Debug
                printf("subshell finish\n");
#endif
                //XIA: for subshell inside a subshell
                if((isEqual(token,")"))&&(isSub != 0)){
                    
                    if(cmdBuffer != NULL){
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
#ifdef Debug
                    printf("returning from subshell from subshell_finish!!!!\n");
#endif
                    state = SIMPLE_INIT;
                    return curCmd; 
                
                }
                else if(isEqual(token,"\n")){
#ifdef Debug
                    printf("fuckeme!!!!!!!!!!!\n");
                    printf("fucking type %d\n",curCmd->type);
#endif
                    //XIA: for subshell // if inside subShell then take it as a sequence command
                    if(isSub != 0){
                        //ignore \n//need to implement sequence command here!!!!!!!!
                    }
                    else//if not insdie subShell then return
                    {
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
#ifdef Debug 
                            printf("the second command is %d\n",curCmd->u.command[1]->type);
#endif
                        }
                        state = SIMPLE_INIT;
                        printf("fuckeme!!!!!!!!!!!!!!!!!!!!!!\n");
                        return curCmd;
                    }
                }
                else if(isEqual(token,"<")){
                    state = SIMPLE_INPUT;
                }
                else if(isEqual(token, ">")){
                    state = SIMPLE_OUTPUT;
                }
                else if(isCombineToken(token)){
#ifdef Debug
                    printf("combining for subshell\n");
#endif
                    if(cmdBuffer != NULL){
                        //printf("not null\n");
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
                    //printf("push\n");
                    //printf("current command %s\n",curCmd->u.word[0]);
                    cmdBuffer = push_command_buffer(curCmd, token);
                    //printf("after\n");
                    curCmd = checked_malloc(sizeof(struct command));
                    curCmd->type = SIMPLE_COMMAND;
                    //printf("cmd Buffer %s\n",cmdBuffer->u.command[0]->u.word[0]);
                    state = SIMPLE_INIT;
                }
                else if(i >= s->size - 1){
                    if(cmdBuffer != NULL){
                        cmdBuffer->u.command[1] = curCmd;
                        curCmd = cmdBuffer;
                    }
#ifdef Debug
                    printf("returning from subshell from subshell_finish when the pointing to the last token\n");
#endif
                    
                    return curCmd;    
                }                    
                
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                
                break;
            default:
                error(1,0,"state transition error\n @ line %d\n",s->curLineNum);
        }
        
    }
    
    return 0;
}



command_t 
push_command_buffer(command_t curCmd, char* token){
	command_t cmdbuffer =  checked_malloc(sizeof(struct command));
	cmdbuffer->type = get_command_type(token);
	cmdbuffer->u.command[0] = curCmd;
	return cmdbuffer;
}

int  
get_command_type(char* token){
	if(isEqual(token,";")){
		return SEQUENCE_COMMAND;
	}
	if(isEqual(token,"||")){
		return OR_COMMAND;
	}
	if(isEqual(token,"|")){
		return PIPE_COMMAND;
	}
	if(isEqual(token,"&&")){
		return AND_COMMAND;
	}	
	return -1;
}

command_t
read_command_stream (command_stream_t s)
{ 
    return parse_Command(s, 0);
}