#include "command-internals.h"
#include "alloc.h"
#include <string.h>
#include <stdlib.h>
#define initsize 100

file_t
create_file(char* name){
	file_t f = checked_malloc(sizeof(struct file));
	f->name = name;
	f->op_type = 2;
	f->position = -1;
	f->option = NULL;
	f->update = 0;
	return f;
}

file_array_t
create_file_array(int maxsize){
	file_array_t newFileArray = checked_malloc(sizeof(struct file_array));
	newFileArray->array = checked_malloc(maxsize* sizeof(struct file));
	newFileArray->size = 0;
	return newFileArray;
}

void
add_arg_file(command_t cmd, struct file f){
	cmd->arg_files->array[cmd->arg_files->size] = f;
	cmd->arg_files->size++;
}

cmd_option_t
create_new_cmd_option(char* name){
	cmd_option_t t = checked_malloc(sizeof(struct cmd_option));
	t->name = name;
	t->known = 0;
	return t;
}

command_t
create_command(void){
	command_t cmd = checked_malloc(sizeof(struct command));
    cmd->type = SIMPLE_COMMAND;
	cmd->status = -1;
	cmd->input = 0;
	cmd->output = 0;
	return cmd;
}

command_stream_t
create_command_stream(void){
	command_stream_t stream = checked_malloc(sizeof(struct command_stream));
	stream->tokens = checked_malloc(initsize * sizeof(char*));
	stream->size = 0;
	stream->ptrIndex = 0;
	stream->maxsize = initsize * sizeof(char*);
	return stream;
}
int 
isEqual(char* a, char* b)
{
	int result = strcmp(a,b)?0:1;
	return result;
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


file_t
find_in_array(file_array_t farray, char* name){
	int i;
	for(i = 0; i < farray->size; i++){
		printf("examed file %s\n",farray->array[i].name);
		if(isEqual(name,farray->array[i].name)){
			return &(farray->array[i]);
		}
	}
	return NULL;
}

