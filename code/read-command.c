// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "alloc.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifndef initsize
#define initsize 100
#endif
//#define Debug 0 //for Debug printf

enum token_state{
	NORMAL,
	IN_COMMENT,
	WAIT_FOR_AND,
	WAIT_FOR_OR
};

int isNormalChar(char);
void init_buffer(char**, char**);
void change_last_token(command_stream_t, char*);
void add_token(command_stream_t, char*,int);
int isCombineToken(char* token);
int is_first_prior(enum command_type type1, enum command_type type2);
int get_command_type(char* token);

//command_t parseSubshell(command_stream_t);
//command_t parse_Command(command_stream_t, int);

char* 
toString(char c){
	char* tmp = checked_malloc(2*sizeof(char));
	tmp[0] = c;
	tmp[1] = '\0';
	return tmp;			
}


//convert the input file char stream into token stream, ignore the comments
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{

  command_stream_t cmdStm = create_command_stream();   

  enum token_state state = NORMAL;

  int curLineNum = 1;

  char* buffer;
  char* ptr;
  size_t bufferSize = initsize*sizeof(char);
  init_buffer(&buffer,&ptr);

  char cGet;
    do{
	cGet =get_next_byte(get_next_byte_argument);
	
	if(state == IN_COMMENT){
		if(cGet == '\n'){
			state = NORMAL;
			add_token(cmdStm,"\n",1);
			curLineNum++;
		}
	}
	else if(state==WAIT_FOR_AND && cGet!='&'){
		error(1,0,"error @ line %d\n",curLineNum);
	}
	else{ //not in comment status
	switch(cGet)
		{
			case '#': //step add_token(cmdStm,buffer);
				add_token(cmdStm,buffer,ptr-buffer);
				init_buffer(&buffer,&ptr);
				state = IN_COMMENT;
				break;
			case '&':
				if(state == WAIT_FOR_AND){ //&&
					add_token(cmdStm,buffer,ptr-buffer);
					init_buffer(&buffer,&ptr);
					add_token(cmdStm,"&&",2);
					state = NORMAL;
				}
				else{
					state = WAIT_FOR_AND;
				}
				break;
			case '|':
				if(state == WAIT_FOR_OR){ //&&
					change_last_token(cmdStm,"||");
					state = NORMAL;
				}
				else{
					add_token(cmdStm,buffer,ptr-buffer);
					init_buffer(&buffer,&ptr);
					add_token(cmdStm,"|",1);
					state = WAIT_FOR_OR;
				}
				break;
			case ' ':	//blank char
			case '\t':
				add_token(cmdStm,buffer,ptr-buffer);
				init_buffer(&buffer,&ptr);
				state = NORMAL;
				break;
			case EOF:
				break;
			case '\n':
				curLineNum++;
			case '>': //special char
			case '<':
			case '(':
			case ')':
			case ';':
				add_token(cmdStm,buffer,ptr-buffer);
				init_buffer(&buffer,&ptr);
				add_token(cmdStm,toString(cGet),1); //also add themselves
				state = NORMAL;
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
					error(1,0,"parse error @ %d with illegal charactor %c\n",curLineNum,cGet);
				}
		}
	}
  }while(cGet!=EOF);
  cmdStm->ptrIndex = 0; //init the ptr
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
	memset(*buffer,'\0',initsize*sizeof(char));
	*ptr = *buffer;
}

void
change_last_token(command_stream_t s,char* token){
	s->tokens[s->ptrIndex-1] = token;
}
void 
add_token(command_stream_t s, char* token, int length){
	if(length>0){
		s->tokens[s->ptrIndex] = token;
		s->ptrIndex++;
		if(s->ptrIndex >= (s->maxsize)/sizeof(char*)){
			char** reStream = checked_grow_alloc(s->tokens,&(s->maxsize));
			s->tokens = reStream;
		}
		s->size++;
	}
}

//============ command parse ==========

enum command_state{
	SIMPLE_INIT,
	SIMPLE_NO,
	SIMPLE_INPUT,
	SIMPLE_INPUT_FINISH,
	SIMPLE_OUTPUT,
	SIMPLE_OUTPUT_FINISH,
    SUBSHELL_FINISH,
};

enum arg_belong{
	MAIN,
	OPTION
};

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
		isEqual(token,"&&"));// || 
		//isEqual(token,";"));
}
command_t push_command_buffer(command_t, char*);

command_t parse_Command(command_stream_t, int);

command_t complete_command(command_t, command_t*);

command_t create_command(void);


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
    print_command(subCmd->u.subshell_command);
    
#endif
    return subCmd;
}

struct Arg_Status{
	int argPos;
	cmd_option_t waitOption;
	enum arg_belong argBelong;
	int cmdId;
};

struct Arg_Status
init_arg_status(int appId){
	struct Arg_Status as;
	as.argPos = 0;
	as.waitOption=NULL;
	as.argBelong=MAIN;
	as.cmdId = appId;
	return as;
}

void
wait_arg_finished(struct Arg_Status *argStatus){
	argStatus->argBelong = MAIN;
	free(argStatus->waitOption);
	argStatus->waitOption=NULL;
}

void
detect_no_arg_option(struct Arg_Status * argStatus){
	if(argStatus->waitOption != NULL){
		//the former wait option has been clarified(no argument)
		assert(argStatus->argBelong==OPTION);
		if(!argStatus->waitOption->known){
			argStatus->waitOption->reqarg = 0;
			argStatus->waitOption->op = 2;
			insert_option(argStatus->cmdId,argStatus->waitOption);
		}
		else{
			//check
		}
		wait_arg_finished(argStatus);
	}
}


command_t
parse_Command(command_stream_t s, int isSub)
{
#ifdef Debug     
    printf("get into parse_command and isSubshell = %d\n",isSub);
#endif
	command_t curCmd = create_command();
    command_t cmdBuffer = NULL;
    
    int haveCmd = 0;
    
    unsigned int curCmdWordIndex;
    size_t curCmdWordMax;
    
    
    enum command_state state = SIMPLE_INIT;   
    //SK: for argument parsing
	struct Arg_Status argStatus = init_arg_status(-1);
    
    while(s->ptrIndex < s->size){
        char* token = s->tokens[s->ptrIndex];
		s->ptrIndex++;
#ifdef Debug     
        //printf("#%d token is %s\n",s->curLineNum,token);
        //printf("i index = %d\n",s->ptrIndex);
        //printf("curOffset = %d\n",curOffset);
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
                    print_command(curCmd);
#endif
                    state = SUBSHELL_FINISH;
                }
                else if(isEqual(token,")"))
                {
					if(haveCmd!=0){//inside subshell and already have command in it
							curCmd = complete_command(curCmd,&cmdBuffer);
#ifdef Debug
								printf("returning from subshell from SIMPLE_INIT\n");
#endif
								return curCmd;
					}
					// else if ; is the end symbol
							else {
								error(1,0,"error @ line %d\n, illegal symbol \")\"",s->curLineNum);
					}

                }
                else if(isEqual(token, "\n")){
                    	state = SIMPLE_INIT;	
		    //}
                }
                else if(isNormalToken(token)){
					if(haveCmd!=0 && cmdBuffer == NULL){
						curCmd = complete_command(curCmd,&cmdBuffer);
						cmdBuffer = push_command_buffer(curCmd,";");
						curCmd = create_command();
						//haveCmd = 0;
					}
                    curCmd->u.word = checked_malloc(initsize*sizeof(char*));
                    curCmdWordMax = initsize * sizeof(char*);
                    curCmd->u.word[0] = token;
					curCmd->u.word[1] = NULL;
				 	curCmd->arg_files = create_file_array(initsize);
                    curCmdWordIndex = 1; 
					int cmdId = query_cmd_id(token);
					if(cmdId<0){ //not found
						cmdId = insert_cmd(token);
					}
					curCmd->cmdId = cmdId;
					argStatus = init_arg_status(cmdId);
                    state = SIMPLE_NO;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                } 
                break;
            case SIMPLE_NO:
#ifdef Debug
                printf("SIMPLE_NO\n");
#endif
                //XIA: for subshell
                if((isEqual(token,")"))&&(isSub != 0)){
					detect_no_arg_option(&argStatus);
                    curCmd = complete_command(curCmd,&cmdBuffer);
#ifdef Debug
                    printf("returning from subshell from SIMPLE_NO\n");
#endif
                    return curCmd; 
                }
                else if(isEqual(token,"\n")||isEqual(token,";")){
					detect_no_arg_option(&argStatus);
					curCmd = complete_command(curCmd,&cmdBuffer);
					state = SIMPLE_INIT;
                    //XIA: for subshell
                    if(isSub != 0){
                       //ignore \n
						haveCmd = 1;
                    }
                    else{
                        return curCmd;
                    }
                }
                else if(isEqual(token,"<")){
					detect_no_arg_option(&argStatus);
                    state = SIMPLE_INPUT;
                }
                else if(isEqual(token, ">")){
					detect_no_arg_option(&argStatus);
                    state = SIMPLE_OUTPUT;
                }
                else if(isNormalToken(token)){
#ifdef Debug                
                    printf("normal word in SIMPLE_NO\n");
#endif
					//SK: Add argument analysis here
					assert(curCmdWordIndex>0);//the first one is the application name, not an argument
					if(token[0]=='-'){
						char* c_ptr = token+1;
						assert(*c_ptr);//suppose always char follows -
						while(*c_ptr && *c_ptr!='-'){	//single - 
							detect_no_arg_option(&argStatus);
							argStatus.argBelong = OPTION;
							argStatus.waitOption = query_option(argStatus.cmdId,toString(*c_ptr));
							if(argStatus.waitOption==NULL){//unknown
								argStatus.waitOption = create_new_cmd_option(toString(*c_ptr));
							}
							c_ptr++;
						}
						//last token treatment
						if(*c_ptr){ //not quit because end
							assert(*c_ptr == '-'); //--situation
							c_ptr++;
							if(*c_ptr){
								detect_no_arg_option(&argStatus);
								argStatus.argBelong = OPTION;
								argStatus.waitOption = query_option(argStatus.cmdId,c_ptr);
								if(argStatus.waitOption==NULL){//unknown
									argStatus.waitOption = create_new_cmd_option(c_ptr);
								}
							}
						}
					}else{ //normal argument, maybe file
						//file_t curArg = checked_malloc(sizeof(struct file));
						//curArg
						file_t f = create_file(token);
						if(argStatus.argBelong == OPTION){
							//belong to the argument treatment						
							assert(argStatus.waitOption!=NULL);
							if(argStatus.waitOption->known){
								if(argStatus.waitOption->reqarg && argStatus.waitOption->op<2){
									f->op_type = argStatus.waitOption->op;
									f->option = argStatus.waitOption->name;
									f->position = -1;
									add_arg_file(curCmd,*f);
								}
								else{
									f->position = argStatus.argPos;
									int fop = query_position(argStatus.cmdId,argStatus.argPos);
									if(fop<0){ //unknown
										f->op_type = 2;
										add_arg_file(curCmd,*f);
									}
									else if(fop < 2){ //0 or 1, this arg should be afile
										f->op_type = fop;
										add_arg_file(curCmd,*f);
									}//otherwise this is just not a file
									argStatus.argPos++;
								}
							}
							else{
								f->option = argStatus.waitOption->name;
								f->op_type = 2;
								add_arg_file(curCmd,*f);
							}
							wait_arg_finished(&argStatus);
						}
						else{
							assert(argStatus.waitOption==NULL);
							f->position = argStatus.argPos;
							int fop = query_position(argStatus.cmdId,argStatus.argPos);
							if(fop<0){ //unknown
								f->op_type = 2;
								add_arg_file(curCmd,*f);
							}
							else if(fop < 2){ //0 or 1, this arg should be afile
								f->op_type = fop;
								add_arg_file(curCmd,*f);
							}//otherwise this is just not a file
							argStatus.argPos++;
						}
						free(f);
						argStatus.argBelong = MAIN;
					}
                    state = SIMPLE_NO;
                    if(curCmdWordIndex + 1>= curCmdWordMax){
                        curCmd->u.word = checked_grow_alloc(curCmd->u.word,&curCmdWordMax);
                    }
                    curCmd->u.word[curCmdWordIndex] = token;
		    		curCmd->u.word[curCmdWordIndex+1] = NULL;
                    curCmdWordIndex++;
                }
                else if(isCombineToken(token)){
					detect_no_arg_option(&argStatus);
#ifdef Debug
                    printf("combining in SIMPLE_NO\n");
#endif
                    curCmd = complete_command(curCmd,&cmdBuffer);
                    cmdBuffer = push_command_buffer(curCmd, token);
                    curCmd = create_command();
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
                if(isEqual(token,")"))
                {
                    curCmd = complete_command(curCmd,&cmdBuffer);
#ifdef Debug
                    printf("returning from subshell from SIMPLE_INPUT_FINISH\n");
#endif
                    
                    return curCmd; 
                }
                else if(isEqual(token,">")){
                    state = SIMPLE_OUTPUT;
                }
                else if(isEqual(token,"\n")||isEqual(token,";")){
                    //XIA: for subshell
                    if(isSub == 0 )
                    {
                        curCmd = complete_command(curCmd,&cmdBuffer);
                        return curCmd;
                        state = SIMPLE_INIT;
                    }
                    else // inside subshell
                    {
                        curCmd = complete_command(curCmd,&cmdBuffer);
                        state = SIMPLE_INIT;
                        haveCmd = 1;
                    }
                }
                else if(isCombineToken(token)){
                    curCmd = complete_command(curCmd,&cmdBuffer);
                    cmdBuffer = push_command_buffer(curCmd, token);			
                    curCmd = create_command();
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
                /*
                if(isEqual(token,"\n")){
                    if(isSub == 0){
                        if(cmdBuffer != NULL){
                            cmdBuffer->u.command[1] = curCmd;
                            curCmd = cmdBuffer;
                        }
                    }
                }*/

                if(isEqual(token,")"))
                {
                    curCmd = complete_command(curCmd,&cmdBuffer);
#ifdef Debug
                    printf("returning from subshell from SIMPLE_OUTPUT_FINISH\n");
#endif
                    
                    return curCmd; 
                }
                else if(isEqual(token,"\n")||isEqual(token,";")){
                    if(isSub == 0)
                    {
                        curCmd = complete_command(curCmd,&cmdBuffer);
                        state = SIMPLE_INIT;
                        return curCmd;
                    }
                    else // inside subshell;
                    {
                        curCmd = complete_command(curCmd,&cmdBuffer);
                        state = SIMPLE_INIT;
                        haveCmd = 1;
                    }
                        
                }
                else if(isCombineToken(token)){
                    curCmd = complete_command(curCmd,&cmdBuffer);
                    cmdBuffer = push_command_buffer(curCmd, token);                 
                    curCmd = create_command();
                    state = SIMPLE_INIT;
                }
                else{
                    error(1,0,"error @ line %d\n",s->curLineNum);
                }
                break;
                
            case SUBSHELL_FINISH:
                //XIA: for subshell inside a subshell
                if((isEqual(token,")"))&&(isSub != 0)){                    
                    curCmd = complete_command(curCmd,&cmdBuffer);
                    state = SIMPLE_INIT;
                    return curCmd;                 
                }
                else if(isEqual(token,"\n")){
                    //XIA: for subshell // if inside subShell then take it as a sequence command
                    if(isSub != 0){
                        //ignore \n//need to implement sequence command here!!!!!!!!
			curCmd = complete_command(curCmd,&cmdBuffer);
			cmdBuffer = push_command_buffer(curCmd, ";");                 
                    	curCmd = create_command();
                    }
                    else//if not insdie subShell then return
                    {
                        curCmd = complete_command(curCmd,&cmdBuffer);
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
                else if(isCombineToken(token)){
#ifdef Debug
                    printf("combining for subshell\n");
#endif
                    curCmd = complete_command(curCmd,&cmdBuffer);
                    cmdBuffer = push_command_buffer(curCmd, token);
                    curCmd = create_command();
                    state = SIMPLE_INIT;
                }
                else if(s->ptrIndex >= s->size - 1){
                    curCmd = complete_command(curCmd,&cmdBuffer);
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
    //End of file treatment 
	detect_no_arg_option(&argStatus);
    s->curLineNum--; // because function getc will return an additional \n
    if(isSub==0)
    {
	    if((state == SIMPLE_INPUT_FINISH ||
		state == SIMPLE_OUTPUT_FINISH ||
		state == SIMPLE_NO  ||
		state == SUBSHELL_FINISH
		))
	    {
	    	curCmd = complete_command(curCmd,&cmdBuffer);
		return curCmd;
	    }
	    else if(state == SIMPLE_INIT){
	    	if(cmdBuffer!=NULL){ //combine situation
		//	if(cmdBuffer->type == SEQUENCE_COMMAND){	//single command;
		//		curCmd = cmdBuffer->u.command[0];
		//		free(cmdBuffer);
		//		return curCmd;
		//	}
		//	else
		//	{
	    			error(1,0,"error @ line %d, incomplete script\n",s->curLineNum);
			
		}
		else{
			return 0;	
		}
	    }
	   else{
    	       error(1,0,"error @ line %d, incomplete script\n",s->curLineNum);
           }
    }
    else{
    	error(1,0,"error @ line %d, incomplete script\n",s->curLineNum);
    }
    return 0;
}


command_t
complete_command(command_t curCmd, command_t *cmdBuffer){
	command_t cmd_finished = curCmd;	
	if(*cmdBuffer != NULL){
                        (*cmdBuffer)->u.command[1] = curCmd;
			command_t cmdBef = (*cmdBuffer)->u.command[0];
			if(cmdBef->type != SIMPLE_COMMAND && cmdBef->type != SUBSHELL_COMMAND)	
			{
				if(is_first_prior((*cmdBuffer)->type, cmdBef->type)){
					//adjust structure
					(*cmdBuffer)->u.command[0] = cmdBef->u.command[1];
					cmdBef->u.command[1] = *cmdBuffer;
					cmd_finished = cmdBef;
				}
				else{
					cmd_finished = *cmdBuffer;
				}
			}
			else{
                        	cmd_finished = *cmdBuffer;
			}
        }
	*cmdBuffer = NULL;
	return cmd_finished;
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

int is_first_prior(enum command_type type1, enum command_type type2){
	int priority[4] = {2,1,2,3};
	return (priority[type1]>priority[type2]);
}

command_t
read_command_stream (command_stream_t s)
{ 
    return parse_Command(s, 0);
}
