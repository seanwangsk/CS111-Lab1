// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "alloc.h"

#define initsize 20//some problem

#define DEBUG

extern file_tracker_t *trackers;
extern int tracker_index;           //the next empty slot for file trackers
extern size_t tracker_size;



char ** cmd_input_analysis(command_t c);
void read_command(command_t c);
char ** cmd_output_analysis(command_t c);
int isEqual(char* a, char* b);
void add_tracker(char *new);
void add_file_to_tracker(char **files);
int check_input_file_block(char **files);
int check_output_file_block(char **files);
void add_cmd_into_queue(command_unit_t cmd, int index);
void check_file_block(command_unit_t cmd);
void execute_command_unit(command_unit_t cmd);

/*
void 
init_buffer(char** buffer, char**ptr){
	*buffer = checked_malloc(initsize*sizeof(char));
	memset(*buffer,'\0',initsize*sizeof(char));
	*ptr = *buffer;
}

void
init_command_unit_t(command_unit_t* c){
	(*c) = checked_malloc(sizeof(struct command_stream));
    (*c)->fileRead = checked_malloc(initsize*sizeof(char*));
    (*c)->fileWrite = checked_malloc(initsize*sizeof(char*));
    (*c)->readNum = 0;
    (*c)->writeNum = 0;
}
*/
int
command_status (command_t c)
{
  return c->status;
}

/*
	return 1 as success, 0 as fail
*/
int
exec_cmd (command_t c){
	if(c->type == AND_COMMAND){
		return (exec_cmd(c->u.command[0]) && exec_cmd(c->u.command[1]));		
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
		//if simple pipe
		int pipefd[2];
		pipe(pipefd);
		pid_t p;
		if((p=fork())==0){ //child thread
			dup2(pipefd[0],0);
			close(pipefd[1]);
			exit(exec_cmd(c->u.command[1]));
		}
		else if(p>0){	//father
			//int a = dup(1);
			dup2(pipefd[1],1);
			close(pipefd[0]);
			exec_cmd(c->u.command[0]);
			close(pipefd[1]); //finish pipeing
			close(1);
			//dup2(a,1);		
			int status;
			if(wait(&status)>0){
				if(WIFEXITED(status)){
					return WEXITSTATUS(status);
				}
				else if(WIFSIGNALED(status)){
					return WTERMSIG(status);
				}
				else{
					perror("Command execution is interrupted\n");
					return 0;
				}
			}
			else{
				perror("Cannot get pipeline return\n");
				return 0;
			}
		}
		else{	//cannot create child process
			perror("fork");
			return 0;
		}
	}
	else if(c->type == SUBSHELL_COMMAND){
		return exec_cmd(c->u.subshell_command);
	}
	else if(c->type == SIMPLE_COMMAND){
		//the end of words should be NULL
		if(c->input!=0){ //has input 
			int iFD = open(c->input,O_RDONLY);
			if(iFD==-1){
				perror("Open Input File");
				return 0; //fail
			}
			dup2(iFD,0);
			close(iFD);
		}
		if(c->output!=0){ //has output
			int oFD = open(c->output,O_WRONLY | O_TRUNC | O_CREAT/*, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR*/);
			if(oFD==-1){
				perror("Open Output File");
				return 0;
			}
			dup2(oFD,1);
			close(oFD);
		}

		pid_t p;
		if((p=fork())==0){ //child thread
			execvp(c->u.word[0],c->u.word);
			perror("execvp"); //something wrong happen so this line is executed
			return 0;
		}
		else if(p>0){	//father
			int status;
			int pid = wait(&status);
			if(pid>0){
				if(WIFEXITED(status)){
					return !WEXITSTATUS(status);
				}
				else if(WIFSIGNALED(status)){
					return !WTERMSIG(status);
				}
				else{
					perror("Command execution is interrupted");
					return 0;
				}
			}
			else{
				perror("wait");
				return 0;
			}
		}
		else{	//cannot create child process
			perror("fork");
			return 0;
		}
	}
	else{
		error(1,0,"cannot recognize command type as %d\n",c->type);
	}
	return 0;
}

void read_command(command_t c)
{
   
    command_unit_t cur_command_unit = checked_malloc(sizeof(struct command_unit));
    cur_command_unit->readNum = 0;
    cur_command_unit->writeNum = 0;
    cur_command_unit->fileRead = NULL;
    cur_command_unit->fileWrite = NULL;
    cur_command_unit->block = 0;
    cur_command_unit->cmd = &c;
    
    
    char ** input_files = cmd_input_analysis(c);
 
#ifdef DEBUG
    char **tmp = input_files;
    while((*tmp)!=NULL)
    {
        printf("input files include:%s\n",(*tmp));
        tmp++;
    }
#endif
    
    char ** output_files = cmd_output_analysis(c);
   
#ifdef DEBUG
    tmp = output_files;
    while((*tmp)!=NULL)
    {
        printf("output files include:%s\n",(*tmp));
        tmp++;
    }
#endif
    
    cur_command_unit->fileRead = input_files;
    cur_command_unit->fileWrite = output_files;
    
    
    //add the files into the trackers
    add_file_to_tracker(input_files);
    add_file_to_tracker(output_files);
    
#ifdef DEBUG
    int i = 0;
    for(i=0;i<tracker_index;i++)
        printf("file_trackers includes: %s\n",trackers[i]->fileName);
#endif
    
    check_file_block(cur_command_unit);

#ifdef DEBUG
    printf("blocked file count: %d\n\n",cur_command_unit->block);
#endif
    
    //if nothing blocks execute the command unit
    if(cur_command_unit->block == 0)
        execute_command_unit(cur_command_unit);
        
}

void execute_command_unit(command_unit_t cmd)
{
    
#ifdef DEBUG
    printf("enter execute_command_unit\n");
#endif
    //block all the file command unit needs to write
    char **ptr = cmd->fileWrite;
    while(*ptr)
    {
        
        int i = 0;
        for(i=0;i<tracker_index;i++)
        {
            if(isEqual(trackers[i]->fileName,(*ptr)))
                break;
        }
        
        if(!isEqual(trackers[i]->fileName,(*ptr)))
            perror("could not find the file in the trackers");
        else
        {
            //mark the file
            trackers[i]->writing++;
            
            //delete the commmand unit from the files queue
            struct cmd_queue *findCmd = trackers[i]->q_head;

            
            if((*findCmd).cmd_unit == cmd)//q_head is pointing to cmd
            {
                printf("!!!!!!!!!!!!!!!!!111111\n");
                trackers[i]->q_head = findCmd->next;
            }
            else
            {
                
                printf("//////////////\n");
                do {
                    
                    struct cmd_queue *tmp = findCmd->next;
                    if(tmp->cmd_unit==cmd)
                    {
                        printf("wtf");
                        findCmd->next=(findCmd->next)->next;
                    }
                    
                    findCmd = findCmd->next;
                } while (findCmd != NULL);
                
            }
        }
        ptr++;
    }
    
    //reading++ for all the file that command unit needs to read
    ptr = cmd->fileRead;
    while(*ptr)
    {
        
        int i = 0;
        for(i=0;i<tracker_index;i++)
        {
            if(isEqual(trackers[i]->fileName,(*ptr)))
                break;
        }
        
        if(!isEqual(trackers[i]->fileName,(*ptr)))
            perror("could not find the file in the trackers");
        else
        {
            //mark the file
            trackers[i]->reading++;
            
            //delete the commmand unit from the files queue
            struct cmd_queue *findCmd = trackers[i]->q_head;
            
            
            if((*findCmd).cmd_unit == cmd)//q_head is pointing to cmd
            {
                trackers[i]->q_head = findCmd->next;
            }
            else
            {
                ////////////////////////////////////////////might be problem here
                //printf("//////////////\n");
                do {
                    
                    struct cmd_queue *tmp = findCmd->next;
                    if(tmp->cmd_unit==cmd)
                    {
                        printf("wtf");
                        findCmd->next=(findCmd->next)->next;
                    }
                    
                    findCmd = findCmd->next;
                } while (findCmd != NULL);
                
            }
        }
        ptr++;
    }
   
   
#ifdef DEBUG
    printf("begin parallel excution\n");
#endif
    //begin parallel excution
    int pid = fork();
    if(pid == 0)
    {
        int grandchild = fork();
        if(grandchild == 0)
        {
            
            
            command_t *temp = cmd->cmd;
            ////////////////////////////////////////////////////////////
            //print_command(*temp);   
            exec_cmd(*temp);
            exit(0);
        }
        else
        {
            int status;
            wait(&status);
            
            //writing--
            ptr = cmd->fileWrite;
            while(*ptr)
            {
                
                int i = 0;
                for(i=0;i<tracker_index;i++)
                {
                    if(isEqual(trackers[i]->fileName,(*ptr)))
                        break;
                }
                
                if(!isEqual(trackers[i]->fileName,(*ptr)))
                    perror("could not find the file in the trackers");
                else
                {
                    //mark the file
                    trackers[i]->writing--; //writing--
                    
                    //delete the commmand unit from the files queue
                    struct cmd_queue *findCmd = trackers[i]->q_head;
                    
                    
                    if((*findCmd).cmd_unit == cmd)//q_head is pointing to cmd
                    {
                        printf("!!!!!!!!!!!!!!!!!111111\n");
                        trackers[i]->q_head = findCmd->next;
                    }
                    else
                    {
                        /////////////////////////////////////////////////可能有问题
                        printf("//////////////\n");
                        do {
                            
                            struct cmd_queue *tmp = findCmd->next;
                            if(tmp->cmd_unit==cmd)
                            {
                                printf("wtf");
                                findCmd->next=(findCmd->next)->next;
                            }
                            
                            findCmd = findCmd->next;
                        } while (findCmd != NULL);
                        
                    }
                }
                ptr++;
            }
            
            //reading--
            ptr = cmd->fileRead;
            while(*ptr)
            {
                
                int i = 0;
                for(i=0;i<tracker_index;i++)
                {
                    if(isEqual(trackers[i]->fileName,(*ptr)))
                        break;
                }
                
                if(!isEqual(trackers[i]->fileName,(*ptr)))
                    perror("could not find the file in the trackers");
                else
                {
                    //mark the file
                    trackers[i]->reading--;
                    
                    //delete the commmand unit from the files queue
                    struct cmd_queue *findCmd = trackers[i]->q_head;
                    
                    
                    if((*findCmd).cmd_unit == cmd)//q_head is pointing to cmd
                    {
                        printf("!!!!!!!!!!!!!!!!!111111\n");
                        trackers[i]->q_head = findCmd->next;
                    }
                    else
                    {
                        /////////////////////////////////////////////////可能有问题
                        printf("//////////////\n");
                        do {
                            
                            struct cmd_queue *tmp = findCmd->next;
                            if(tmp->cmd_unit==cmd)
                            {
                                printf("wtf");
                                findCmd->next=(findCmd->next)->next;
                            }
                            
                            findCmd = findCmd->next;
                        } while (findCmd != NULL);
                        
                    }
                }
                ptr++;
            }     
            ///////////////////////////////////////////////////////might be problem here
            //search inside files to check if there is other command_unit that is blocked
            ptr = cmd->fileWrite;
            while(*ptr)
            {
                
                int i = 0;
                for(i=0;i<tracker_index;i++)
                {
                    if(isEqual(trackers[i]->fileName,(*ptr)))
                        break;
                }
                
                if(!isEqual(trackers[i]->fileName,(*ptr)))
                    perror("could not find the file in the trackers");
                else
                {
                    //find the next command unit in the queue. see if it is block by writing
                    struct cmd_queue *findCmd = trackers[i]->q_head;
                    while(findCmd!=NULL)
                    {
                        ((*findCmd).cmd_unit)->block = 0;
                        check_file_block((*findCmd).cmd_unit);
                        if((*findCmd).cmd_unit->block == 0)
                            execute_command_unit((*findCmd).cmd_unit);
                        findCmd=findCmd->next;
                    }
                    
                }
                ptr++;
            }
            
            //search inside files to check if there is other command_unit that is blocked
            ptr = cmd->fileRead;
            while(*ptr)
            {
                
                int i = 0;
                for(i=0;i<tracker_index;i++)
                {
                    if(isEqual(trackers[i]->fileName,(*ptr)))
                        break;
                }
                
                if(!isEqual(trackers[i]->fileName,(*ptr)))
                    perror("could not find the file in the trackers");
                else
                {
                    //find the next command unit in the queue. see if it is block by writing
                    struct cmd_queue *findCmd = trackers[i]->q_head;
                    while(findCmd!=NULL)
                    {
                        ((*findCmd).cmd_unit)->block = 0;
                        check_file_block((*findCmd).cmd_unit);
                        if((*findCmd).cmd_unit->block == 0)
                            execute_command_unit((*findCmd).cmd_unit);
                        findCmd=findCmd->next;
                    }
                    
                }
                ptr++;
            }

            
            exit(0);
        }
    }
     
}

void
check_file_block(command_unit_t cmd)
{
    //check if there is reading file block
    char **ptr = cmd->fileRead;
    while(*ptr)
    {
        //find the file inside file tracker
        int i = 0;
        for(i=0;i<tracker_index;i++)
        {
            if(isEqual(trackers[i]->fileName,(*ptr)))
                break;
        }
        
        if(!isEqual(trackers[i]->fileName,(*ptr)))
            perror("cannot find the file inside file tracker");
        else
        {
            if(trackers[i]->writing > 0)
                //dosomething;
                cmd->block++;

            
            add_cmd_into_queue(cmd,i);
        }
        
        
        ptr++;
    }
    
    //check if there is writing file block
    ptr = cmd->fileWrite;
    while(*ptr)
    {
        
        //find the file inside file tracker
        int i = 0;
        for(i=0;i<tracker_index;i++)
        {
            if(isEqual(trackers[i]->fileName,(*ptr)))
                break;
        }
        
        if(!isEqual(trackers[i]->fileName,(*ptr)))
            perror("cannot find the file inside file tracker");
        else
        {
            if(trackers[i]->writing > 0 || trackers[i]->reading > 0)
                cmd->block++;
            
              add_cmd_into_queue(cmd,i);
        }
        
        
        ptr++;
    }

}

//add command_unit into the queue of file
void   
add_cmd_into_queue(command_unit_t cmd, int index)
{
#ifdef DEBUG
    printf("add command into queue\n");
#endif
    if (trackers[index]->q_head == NULL) {
        struct cmd_queue *newGuy = checked_malloc(sizeof(struct cmd_queue));
        newGuy->cmd_unit = cmd;
        newGuy->next = NULL;
        trackers[index]->q_head = newGuy;
        
        //printf("%d\n",&cmd);
        //printf("initialize q_head\n");
    }
    else{
#ifdef DEBUG
        printf("awesome dude\n");
#endif
        struct cmd_queue *ptr = trackers[index]->q_head;
        
        while((ptr->next)!=NULL)
        {
            ptr = ptr->next;
        }
        
        struct cmd_queue *newGuy = checked_malloc(sizeof(struct cmd_queue));
        newGuy->cmd_unit = cmd;
        newGuy->next = NULL;
        ptr->next = newGuy;
    }
}


//add a new file_tracker into the trackers
void
add_file_to_tracker(char **files)
{
    
    char **ptr = files;
    while(*ptr)
    {
        int dup = 0;
        int i = 0;
        for(i=0;i<tracker_index;i++)
        {
            if(isEqual(trackers[i]->fileName,(*ptr)))
            {
                dup = 1;
                break;
            }
        }
        
        if(dup == 0)
            add_tracker(*ptr);
        
        ptr++;
    }
}

void 
add_tracker(char *new)      
{
    file_tracker_t newTracker = checked_malloc(sizeof(struct file_tracker));
    newTracker->fileName = new;
    newTracker->writing = 0;
    newTracker->reading = 0;
    newTracker->q_head = NULL;
    
    
    if(tracker_index == 0)   //file_trackers is empty then initialize
    {
        trackers = checked_malloc(initsize*sizeof(file_tracker_t*));
        tracker_size = initsize*sizeof(file_tracker_t*);
        //memset(trackers,'\0',initsize*sizeof(file_tracker_t*));
    }
    else if(tracker_index>=tracker_size/sizeof(struct file_tracker_t*))
    {
        //grow the memory for tracker if necessary
        file_tracker_t *renewTracker = checked_grow_alloc(trackers, &tracker_size);
        trackers = renewTracker;
    }
    
    trackers[tracker_index] = newTracker;
    tracker_index++;
    
}

// return the file names without duplication
char ** 
add_without_dup(char** file1, char** file2)
{
    
    char **file = checked_malloc(initsize*sizeof(char*));
    memset(file, '\0', initsize*sizeof(char*));
    char **ptr = file;
    char **tmp1 = file1;
    
    while(*tmp1)
    {
        (*ptr) = (*tmp1);
        /*
#ifdef DEBUG
        printf("ptr:%s\n",(*ptr));
#endif
         */
        ptr++;
        tmp1++;
        
    }
    
    char **tmp2 = file2;
    int dup = 0;
    
    while(*tmp2)
    {
        tmp1 = file;
        dup = 0;
        /*
#ifdef DEBUG
        printf("tmp2:%s\n",*tmp2);
#endif
         */
        while(*tmp1)
        {
            /*
#ifdef DEBUG
            printf("tmp1:%s\n",*tmp1);
#endif
             */
            if(isEqual((*tmp1),(*tmp2)))
            {
                dup = 1;
                break;
            }   
            tmp1++;
        }
        
        if(dup == 0)
        {
            (*ptr) = (*tmp2);
            ptr++;
        }
        tmp2++;
    }   

    return file;

}

char ** 
cmd_input_analysis(command_t c)
{
    char ** inputFiles = checked_malloc(initsize*sizeof(char*));
    memset(inputFiles,'\0',initsize*sizeof(char*));
    char ** ptr = inputFiles;
    int fileNum = 0;
    
    if(c->type == SIMPLE_COMMAND)
    {
        if(c->input != NULL)
        {
            inputFiles[0] = c->input;
            fileNum++;
        }
        int i = 1;
        while( c->u.word[i]!= NULL)
        {
            inputFiles[fileNum] = c->u.word[i];
            i++;
            fileNum++;
        }
        return inputFiles;
    }   
    
    else if(c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND || c->type == SEQUENCE_COMMAND)
    {
        char **file1 = cmd_input_analysis(c->u.command[0]);
        char **file2 = cmd_input_analysis(c->u.command[1]);
        char **file = add_without_dup(file1,file2);
        return file;
    }   
    else if(c->type == SUBSHELL_COMMAND)
    {
        if(c->input != NULL)
        {
            inputFiles[0] = c->input;
        }
        char **tmp = cmd_input_analysis(c->u.subshell_command);
        return add_without_dup(tmp,inputFiles);
    }
    else
    {
        perror("Invalid Command Types\n");
    }
    
    return 0;
}




char ** 
cmd_output_analysis(command_t c)
{
    char ** outputFiles = checked_malloc(initsize*sizeof(char*));
    memset(outputFiles,'\0',initsize*sizeof(char*));
    char ** ptr = outputFiles;
    
    if(c->type == SIMPLE_COMMAND)
    {
        if(c->output != NULL)
        {
            outputFiles[0] = c->output;
        }
#ifdef DEBUG
       // printf("outputfiles%s\n",outputFiles[0]);
#endif
        return outputFiles;
    }   
    
    else if(c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND || c->type == SEQUENCE_COMMAND)
    {
        char **file1 = cmd_output_analysis(c->u.command[0]);
        char **file2 = cmd_output_analysis(c->u.command[1]);
        char **file = add_without_dup(file1,file2);
        return file;
    }   
    else if(c->type == SUBSHELL_COMMAND)
    {
        //printf("inside");
        if(c->output != NULL)
        {
            outputFiles[0] = c->output;
        }
        char **tmp = cmd_output_analysis(c->u.subshell_command);
        return add_without_dup(tmp,outputFiles);
    }
    else
    {
        perror("Invalid Command Types\n");
    }
    
    
    return 0;
}

/*
 function analyze: tree searching
 1)   Input
 
 2)   Output
 
 3)   Argument
 
 4)   Notice: every type of command can have input and output
 
 5)   Only simple command could have argument
 
*/

/*
 Function execute_command_unit
 Mark the occupation of file
 Fork
 Child: Fork
 Grandchild： execute command_unit
 Child: wait
 For all related files
 Then clear the occupation
 Reduce occupy_counter of command on List Head
 Check occupy counter ==0?
 If yes
 Execute_command_unit(the command)
 If no
 Just pass
 */

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	int a = dup(0);
	int b = dup(1);
	int cc = dup(2);
	if(time_travel == 0){   //normal mode
		fflush(stderr);
		exec_cmd(c);	
	}
    else                    //time travel mode
    {
        read_command(c);
    }
    dup2(a,0);
	dup2(b,1);
	dup2(cc,2);
	fflush(stderr);
    
}
