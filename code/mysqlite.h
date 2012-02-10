#include "command-internals.h"
#define CALL_SQLITE(f)                                          \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_OK) {                                   \
            fprintf (stderr, "%s failed with status %d: %s\n",  \
                     #f, i, sqlite3_errmsg (db));               \
            exit (1);                                           \
        }                                                       \
    }                                                           \


#define CALL_SQLITE_EXPECT(f,x)                                 \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_ ## x) {                                \
            fprintf (stderr, "%s failed with status %d: %s\n",  \
                      #f, i, sqlite3_errmsg (db));               \
            exit (1);                                           \
        }                                                       \
    }                                                           \

//return -1 if not found
int query_cmd_id(char* cmdName);
//return NULL if not found
cmd_option_t query_option(int id, char* option);
//return -1 if not found, 0 for read, 1 for write, 2 for do nothing
int query_position(int id, int position);
int insert_cmd(char* cmd);
void insert_option(int id, cmd_option_t option);
void insert_arg(int id, int position, int op);
