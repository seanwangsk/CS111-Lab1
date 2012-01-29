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
		printf("1 is %d, 2 is %d\n",pipefd[0],pipefd[1]);
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
			printf("hello\n");
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
			int oFD = open(c->output,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
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

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	if(time_travel == 0){ //normal mode
		pid_t p;
		if((p = fork())==0){
			printf("==================\n");
			exec_cmd(c);	
		}
		else if(p>0){
			int status;
			wait(&status);
		}
	}

}
