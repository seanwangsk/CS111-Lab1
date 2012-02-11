#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include "mysqlite.h"
#include "alloc.h";
static sqlite3 * db = NULL;

void 
open_db(){
	CALL_SQLITE (open ("commands.db", & db));
}

void
close_db(){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	CALL_SQLITE(close(db));
	db = NULL;
}

int
query_cmd_id(char* cmdName){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	char * sql;
    sqlite3_stmt * stmt;
    sql = "SELECT _id FROM commands where name=?";
    CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
    CALL_SQLITE (bind_text (stmt, 1, cmdName, strlen(cmdName), SQLITE_STATIC));
	int s;
	s = sqlite3_step (stmt);
	if (s == SQLITE_ROW) {
	    int id =  sqlite3_column_int (stmt, 0);
	    CALL_SQLITE(finalize(stmt));
	    return id;
	}
	else {
	    CALL_SQLITE(finalize(stmt));
	    return -1;
	}
}

cmd_option_t
query_option(int id, char* option){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	char * sql;
    	sqlite3_stmt * stmt;
    	sql = "SELECT op,reqarg FROM options where _id =? and name = ?";
    	CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
    	CALL_SQLITE (bind_int (stmt, 1, id));
    	CALL_SQLITE (bind_text (stmt, 2, option, strlen(option), SQLITE_STATIC));
	int s;
	s = sqlite3_step (stmt);
	if (s == SQLITE_ROW) {
	     cmd_option_t myOption = checked_malloc(sizeof(struct cmd_option));
	     myOption->name = option;
	     myOption->op =sqlite3_column_int (stmt, 0);
	     myOption->reqarg = sqlite3_column_int(stmt,1);
		 myOption->known  = 1;						
	     CALL_SQLITE(finalize(stmt));
		 //printf("find\n");
	     return myOption;
	}
	else {
		//printf("not find\n");
	     CALL_SQLITE(finalize(stmt));
	     return NULL;
	}
}

int query_position(int id, int position){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	char* sql = "SELECT op FROM args where _id =? and pos = ?";
    	sqlite3_stmt * stmt;
    	CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
    	CALL_SQLITE (bind_int (stmt, 1, id));
    	CALL_SQLITE (bind_int (stmt, 2, position));
	int s;
	s = sqlite3_step (stmt);
	if (s == SQLITE_ROW) {
	     cmd_option_t myOption = checked_malloc(sizeof(struct cmd_option));
	     int op =  sqlite3_column_int (stmt, 0);
	     CALL_SQLITE(finalize(stmt));
	     return op;
	}
	else {
	     CALL_SQLITE(finalize(stmt));
	     return -1;
	}
}

int insert_cmd(char* cmd){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
   	char * sql;
    sqlite3_stmt * stmt;
   	sql = "INSERT INTO commands (name) VALUES (?)";
   	CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
   	CALL_SQLITE (bind_text (stmt, 1, cmd, strlen(cmd), SQLITE_STATIC));
	CALL_SQLITE_EXPECT(step(stmt),DONE);
	CALL_SQLITE(finalize(stmt));
	return query_cmd_id(cmd);
}

void insert_option(int id, cmd_option_t option){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	char * sql;
    sqlite3_stmt * stmt;
    sql = "INSERT INTO options (name,op,reqarg,_id) VALUES (?,?,?,?)";
    CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
    CALL_SQLITE (bind_text (stmt, 1, option->name, strlen(option->name), SQLITE_STATIC));
    CALL_SQLITE (bind_int (stmt, 2, option->op));
    CALL_SQLITE (bind_int (stmt, 3, option->reqarg));
    CALL_SQLITE (bind_int (stmt, 4, id));

	CALL_SQLITE_EXPECT(step(stmt),DONE);
	CALL_SQLITE(finalize(stmt));
}

void insert_arg(int id, int position, int op){
	if(db==NULL){
		error(1,0,"db not initialized");
	}
	char * sql;
    	sqlite3_stmt * stmt;
    	sql = "INSERT INTO args (pos,op,_id) VALUES (?,?,?)";
    	CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));
    	CALL_SQLITE (bind_int (stmt, 1, position));
    	CALL_SQLITE (bind_int (stmt, 2, op));
    	CALL_SQLITE (bind_int (stmt, 3, id));
	CALL_SQLITE_EXPECT(step(stmt),DONE);
	CALL_SQLITE(finalize(stmt));
}
