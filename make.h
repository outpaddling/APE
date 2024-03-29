#define MAKEFILE_MAX_VARS       2048
#define MAKEFILE_MAX_TARGETS    1024
#define MAKEFILE_LINE_MAX       1024
#define MAKEFILE_TOKEN_MAX      128
#define MAKE_CMD_MAX            128

typedef struct
{
    char    make_directory[APE_PATH_MAX+1];
    char    make_cmd[MAKE_CMD_MAX+1];
    char    makefile[APE_PATH_MAX+1];
    char    executable[APE_PATH_MAX+1];
    char    run_prefix[APE_PATH_MAX+1];
    char    run_cmd[APE_CMD_MAX+1];
    char    make_args[OPTION_LEN+1];
    char    run_args[OPTION_LEN+1];
    char    *make_var_names[MAKEFILE_MAX_VARS];
    char    *make_var_values[MAKEFILE_MAX_VARS];
    char    *make_targets[MAKEFILE_MAX_TARGETS];
    int     make_vars;
}   proj_t;

#define PROJ_INIT   { "", "", "", "" } 
#define RUN_MODE_LEN    10
#define MAKE_FIELDS     5
#define ACTIVE_PROJ(p)  ((p)->makefile[0] != '\0')

