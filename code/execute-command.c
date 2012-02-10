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
#include <assert.h>
#define initsize 100
//#define N 4
#define DEBUG

extern file_tracker_t *trackers;
extern unsigned int tracker_index;           //the next empty slot for file trackers
extern size_t tracker_size;
extern int N;   //parallel limitation


cmd_queue_t cmds_to_exec;
cmd_queue_t cmds_to_exec_tail;

void cmd_file_analysis(command_t, file_array_t);
command_unit_t analyze_command(command_t c);
int isEqual(char* a, char* b);
void add_tracker(char *new);
void add_file_to_tracker(file_array_t);
int check_input_file_block(char **files);
int check_output_file_block(char **files);
void add_cmd_into_queue(command_unit_t cmd, int index, int op_type);
void check_file_block(command_unit_t cmd);
void execute_command_unit(command_unit_t cmd);
void check_file_block_withoutadd(command_unit_t cmd);
void block_wirte_file(command_unit_t cmd);
void block_read_file(command_unit_t cmd);
int findTrackerIndex(char *ptr);
void release_read_file(command_unit_t cmd);
void release_write_file(command_unit_t cmd);
void execute_command_list(void);


static int lineNum = 1;

//266214CA75

file_array_t
create_file_array(){
	file_array_t fArray = checked_malloc(sizeof(struct file_array));
	fArray->array = checked_malloc(initsize*sizeof(struct file));
	fArray->size = 0;
	return fArray;
}

void
add_without_dup(file_array_t fArray, char* name, int op_type){
	int i;
	for(i = 0; i<fArray->size; i++){
		if(isEqual(fArray->array[i].name,name)){
			fArray->array[i].op_type = fArray->array[i].op_type | op_type;  
		        return;
		}
	}
	//not duplicated
	fArray->array[fArray->size].name = name;
	fArray->array[fArray->size].op_type = op_type;
	fArray->size++; 
}


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
		return  exec_cmd(c->u.command[0]) || exec_cmd(c->u.command[1]);
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
			int result = exec_cmd(c->u.command[1]);
			close(pipefd[0]);
			close(0);
			exit(1);
		}
		else if(p>0){	//father
			pid_t p2; 
			if((p2=fork())==0){
				dup2(pipefd[1],1);
				close(pipefd[0]);
				exec_cmd(c->u.command[0]);
				close(pipefd[1]); //finish pipeing
			//	close(1);
				exit(1);
			}
			else{
				close(pipefd[0]);
				close(pipefd[1]);
				int status;
				if(waitpid(p2,&status,0)>0){
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
		
		pid_t p;
		if((p=fork())==0){
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
			execvp(c->u.word[0],c->u.word);
			perror(c->u.word[0]); //something wrong happen so this line is executed
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

command_unit_t 
analyze_command(command_t c)
{
    command_unit_t cur_command_unit = checked_malloc(sizeof(struct command_unit));
    cur_command_unit->block = 0;
    cur_command_unit->cmd = c;
    cur_command_unit->dependFiles = create_file_array();
    cmd_file_analysis(c, cur_command_unit->dependFiles);
   
    //add the files into the trackers
    add_file_to_tracker(cur_command_unit->dependFiles);
	
    //if nothing blocks execute the command unit
    check_file_block(cur_command_unit);

    unsigned int i;
    for(i=0;i<tracker_index;i++){
#ifdef DEBUG
	printf("name is %s, ",trackers[i]->fileName);
	printf("reading is %d, writing is %d, ", trackers[i]->reading, trackers[i]->writing);
#endif
	cmd_queue_t ptr = trackers[i]->q_head; 	
	while(ptr!=NULL){
#ifdef DEBUG
		printf("blocked command is %d, ",ptr->cmdNum);
#endif
		ptr = ptr->next;
	}
#ifdef DEBUG
	printf("\n");
#endif
    }
#ifdef DEBUG
    printf("=================================\n");
#endif        
    return cur_command_unit;
}

//find out the index of the file_tracker that has the same name as the ptr 
int findTrackerIndex(char *ptr)
{
    unsigned int i=0;
    for(i=0;i<tracker_index;i++)
    {
        if(isEqual(trackers[i]->fileName,ptr))
            return i;
    }
    error(1,0,"could not find the file in the trackers\n");
}
  

void release_command_occupation(command_unit_t cmd){
          
            //search inside the queues of files to check if there is other command_unit that is blocked
            int i;
            for(i = 0; i<cmd->dependFiles->size;i++)
            {
		struct file curFile = cmd->dependFiles->array[i];
		if(curFile.op_type == 1) //if curfile is write
		{
		        int i = findTrackerIndex(curFile.name);			
			//assert(trackers[i]->writing==1);
			//assert(trackers[i]->reading==0);
			trackers[i]->writing--;
		        //find the next command unit in the queue. see if it is block by writing
			while(trackers[i]->q_head !=NULL && trackers[i]->q_head->cmd_unit->block==0){
				trackers[i]->q_head = trackers[i]->q_head->next;
			}
			cmd_queue_t cmdWait = trackers[i]->q_head;
		        cmd_queue_t cmdWait_before = NULL;
			int read_before = 0;
		        while(cmdWait!=NULL)
		        {   	
			    //printf("for %s, for %d, op type %d\n",curFile.name,cmdWait->cmdNum, cmdWait->type);
			    if(cmdWait->type == 1){	//the next command is write
				if(read_before){	//there's read command before in the queue, so we are still blocked
					break;
				}
				else{ //current cmdWait is the head
					assert(cmdWait == trackers[i]->q_head);				
		            		cmdWait->cmd_unit->block--;
					if(cmdWait->cmd_unit->block==0){
						cmdWait = cmdWait->next;
					}
					break;
				}
			    }
			    else{
				read_before=1;
				cmdWait->cmd_unit->block--;	
				
				//printf("for %s, block-- for %d, now is %d\n",curFile.name,cmdWait->cmdNum, cmdWait->cmd_unit->block);
				if(cmdWait->cmd_unit->block==0){
					if(cmdWait_before==NULL){
						//printf("1unlocking reading %d\n",cmdWait->cmdNum);
						assert(cmdWait == trackers[i]->q_head);	
						trackers[i]->q_head = cmdWait->next;
					}
					else{
						//printf("2unlocking reading %d\n",cmdWait->cmdNum);
						assert(cmdWait!=cmdWait_before);
						cmdWait_before->next = cmdWait->next;
					}					
				}
				else{
					cmdWait_before = cmdWait;
				}
		            	cmdWait=cmdWait->next;
			    }
		        }
		}
		else{ //Read
			
			int i = findTrackerIndex((curFile.name));   
			assert(trackers[i]->writing == 0);
			trackers[i]->reading--;
			//might be multi read
			if(trackers[i]->reading==0){     
				while(trackers[i]->q_head !=NULL && trackers[i]->q_head->cmd_unit->block==0){
						trackers[i]->q_head = trackers[i]->q_head->next;
				}

				//assumption: for the file being read, the first one of the queue must be a write
				if(trackers[i]->q_head !=NULL){	
					
					//printf("for file %s\n",trackers[i]->fileName);
					//printf("%d in queue\n",trackers[i]->q_head->cmdNum);
					//assert(trackers[i]->q_head->type == 1);
					trackers[i]->q_head->cmd_unit->block--;  //must be a writing command
					if(trackers[i]->q_head->cmd_unit->block==0){
						trackers[i]->q_head = trackers[i]->q_head->next;
					}
				}
			}
		}
            }
            
}

void
check_file_block(command_unit_t cmd)
{   

   //check if there is writing file block
   int j;
   for(j = 0; j < cmd->dependFiles->size;j++){
	    if(cmd->dependFiles->array[j].op_type == 1) //File that I write
	    {
		//find the file inside file tracker
		int i = findTrackerIndex(cmd->dependFiles->array[j].name);
		if(trackers[i]->writing > 0 || trackers[i]->reading > 0){
		    cmd->block++;
#ifdef DEBUG
		    printf("command %d, for %s, writing blocked\n",lineNum,trackers[i]->fileName);
#endif
	       	    add_cmd_into_queue(cmd,i,1);
		}
		else{
			assert(trackers[i]->writing ==0);
			trackers[i]->writing = 1; //occupy
		}
	    }
	    else{	//File that I read
		//find the file inside file tracker
		int i = findTrackerIndex(cmd->dependFiles->array[j].name);
		//see if there is a command writing to this file
		int cannotRead = 0;
		
		if(trackers[i]->writing > 0){
#ifdef DEBUG
		    printf("command %d, for %s, reading blocked because writing\n",lineNum,trackers[i]->fileName);
#endif
		    cmd->block++;
		    cannotRead =1;
		}
		else
		{
		   //see if the cmd in the queue needs to write to the file
		    cmd_queue_t queue_ptr = trackers[i]->q_head; 
		    while(queue_ptr!=NULL)
		    {
		        if(queue_ptr->type==1){
#ifdef DEBUG
				printf("command %d, for %s, reading blocked because in queue writing\n",lineNum,trackers[i]->fileName);
#endif
				cmd->block++;
				cannotRead =1;
				break;
			}
		        queue_ptr = queue_ptr->next;
		    }
		}
		if(cannotRead){
	       		add_cmd_into_queue(cmd,i,0);//add the cmd into the queue of the file
	       	}
		else{
			trackers[i]->reading ++;			
		}
	}
    }
}

void
mark_files(command_unit_t cmd)
{   
   //check if there is writing file block
   int j;
   for(j = 0; j < cmd->dependFiles->size;j++){
	    if(cmd->dependFiles->array[j].op_type == 1) //File that I write
	    {
		//find the file inside file tracker
		int i = findTrackerIndex(cmd->dependFiles->array[j].name);
		trackers[i]->writing++; //occupy
	    }
	    else{	//File that I read
		//find the file inside file tracker
		int i = findTrackerIndex(cmd->dependFiles->array[j].name);
		trackers[i]->reading ++;
        }
    }
}


//add command_unit into the queue of file
void   
add_cmd_into_queue(command_unit_t cmd, int index,int type)
{
    if (trackers[index]->q_head == NULL) {
        struct cmd_queue *newGuy = checked_malloc(sizeof(struct cmd_queue));
        newGuy->cmd_unit = cmd;
        newGuy->next = NULL;
	newGuy->type = type;
	newGuy->cmdNum = lineNum;
        trackers[index]->q_head = newGuy;
    }
    else{
        struct cmd_queue *ptr = trackers[index]->q_head;
        
        while((ptr->next)!=NULL)
        {
            ptr = ptr->next;
        }
        
        struct cmd_queue *newGuy = checked_malloc(sizeof(struct cmd_queue));
        newGuy->cmd_unit = cmd;
        newGuy->next = NULL;
	newGuy->type = type;
	newGuy->cmdNum = lineNum;
        ptr->next = newGuy;
    }
    
}


//add a new file_tracker into the trackers
void
add_file_to_tracker(file_array_t files)
{
    int i;
    for(i=0;i<files->size;i++)
    {
#ifdef DEBUG
	printf("in add_file_to_tracker, file is %s\n", files->array[i].name);
#endif
	int dup = 0;
        unsigned int j = 0;
        for(j=0;j<tracker_index;j++)
        {
            if(isEqual(trackers[j]->fileName,files->array[i].name))
            {
                dup = 1;
                break;
            }
        }
        
        if(dup == 0){
	   add_tracker(files->array[i].name);
	}
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


void
cmd_file_analysis(command_t c, file_array_t dependFiles)
{   
    if(c->type == SIMPLE_COMMAND)
    {
        if(c->input != 0)
        {
	    add_without_dup(dependFiles, c->input, 0);
        }
	if(c->output != 0)
        {
             add_without_dup(dependFiles, c->output, 1);
        }
        int i = 1;
        while( c->u.word[i]!= NULL)
        {
            add_without_dup(dependFiles, c->u.word[i], 0);
            i++;
        }
    }   
    else if(c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND || c->type == SEQUENCE_COMMAND)
    {
        cmd_file_analysis(c->u.command[0],dependFiles);
	cmd_file_analysis(c->u.command[1],dependFiles);       
    }   
    else if(c->type == SUBSHELL_COMMAND)
    {
        if(c->input != 0)
        {
            add_without_dup(dependFiles, c->input, 0);
        }
        if(c->output != 0)
        {
            add_without_dup(dependFiles, c->output, 1);
        }	
        cmd_file_analysis(c->u.subshell_command,dependFiles);
    }
    else
    {
        perror("Invalid Command Types\n");
    }
}

void
add_to_cmds_to_exec(command_unit_t cmd_u){
	cmd_queue_t newGuy = checked_malloc(sizeof(struct cmd_queue));
        newGuy->cmd_unit = cmd_u;
        newGuy->next = NULL;
	newGuy->cmdNum = lineNum;
	if(cmds_to_exec == NULL) //if the whole command link is null
	{
		cmds_to_exec = newGuy;
		cmds_to_exec_tail = cmds_to_exec;
	}
	else
	{
		cmds_to_exec_tail->next = newGuy;
		cmds_to_exec_tail = cmds_to_exec_tail->next;
	}
}



void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	if(time_travel == 0){   //normal mode
		exec_cmd(c);	
	}
    else                    //time travel mode
    {   
	//SK:analyze the command and return command unit
	//Modify the name "read_command" to "analyze_command". avoiding bias with read-command.c
	//Will return the command_unit_t;
	
        command_unit_t cmd_u = analyze_command(c);
	//add the command into whole commands queue
	add_to_cmds_to_exec(cmd_u);	
	lineNum++;
    }   
}

void
clear_read_write(){
	unsigned int i;	
	for(i=0;i<tracker_index;i++){
		trackers[i]->reading=0;
		trackers[i]->writing=0;
	}
}

void
execute_command_list(){
	clear_read_write();
    int para = 0;     //indicate how many commands are parallel executing
	while(cmds_to_exec!=NULL){
		//iterate to find the unblocked command to execute
		cmd_queue_t queue_ptr = cmds_to_exec;
		while(queue_ptr!=NULL){
            if(para >= N)
                break;
            
			if(queue_ptr->cmd_unit->block==0 && queue_ptr->pid==0/*not current running*/){
                para++; //add 1 indicating one more command is executing 
#ifdef DEBUG
				printf("cmd %d is running\n",queue_ptr->cmdNum);	
                printf("there are %d commands are parallel executing \n",para);
#endif
                mark_files(queue_ptr->cmd_unit);
				pid_t pid;
				if((pid=fork())==0){	//child
					int returnV = exec_cmd(queue_ptr->cmd_unit->cmd);
					_exit(returnV);
				}
				else if(pid>0){
					queue_ptr->pid = pid;  //bind the process pid to the command, for WAITPID to determine which command has finished
				}
				else{
					error(1,0,"fork error");
				}
			}
			queue_ptr=queue_ptr->next;
		}
		//every finished child will return to this point
		//ret_pid indicate which command (child) is finished 
		pid_t ret_pid = waitpid(-1,NULL,0); 
        para--; 
		queue_ptr = cmds_to_exec;
		cmd_queue_t queue_ptr_before = NULL;	//the previous node for queue_ptr, used to delete node from queue
		while(queue_ptr!=NULL){
			if(queue_ptr->pid==ret_pid){
#ifdef DEBUG
				printf("%d is releasing\n",queue_ptr->cmdNum);
#endif
				release_command_occupation(queue_ptr->cmd_unit);
				//release node from linked list
				if(queue_ptr_before==NULL){	//the node is the head
					cmds_to_exec = cmds_to_exec->next;
					//free memory
				}
				else{
					queue_ptr_before->next = queue_ptr->next;
					//free memory
				}
				break;
			}
			queue_ptr_before = queue_ptr; //store the current location in before pointer
			queue_ptr=queue_ptr->next;	//move to the next
		}
#ifdef DEBUG
		printf("finished\n");
#endif
	}
}
