/* Custom menu */
#define MAX_CUSTOM_ITEMS    14
#define FIXED_CUSTOM_ITEMS  6

typedef struct
{
    char    text[TWC_MENU_TEXT_LEN+1];
    char    command[CMD_LEN+1];
    char    directory[PATH_MAX+1];
    char    run_mode[RUN_MODE_LEN+1];
    int     echo_command;
    int     hot_key;
}   cust_t;

#define CUST_INIT   { "", "", ".", "Foreground", 0, P_WAIT }
#define NEW_ITEM    1
#define OLD_ITEM    0


