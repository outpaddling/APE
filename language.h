/* Language options */
#define LANG_NAME_LEN       40
#define LANG_ID_LEN         40
#define SYNTAX_CHECK_LEN    20
#define COMPILE_ONLY_LEN    10
#define BACKTRACE_LEN       10
#define EXE_SRC_LEN         20
#define OUTPUT_FLAG_LEN     20
#define ERR_FORMAT_LEN      80
#define LANGUAGE_OPTS       12
/* Underallocated on the assumption that most fields use far less than OPTION_LEN */
#define LANG_OPTS_LINE_LEN  OPTION_LEN*2
#define MAX_TAB_STOP        16
#define TAB_FILLER_CHAR     8
#define AUTO_WRAP           1
#define NO_AUTO_WRAP        0
#define USE_AUTO_WRAP(f)    (((f)->lang != NULL) && ((f)->lang->auto_wrap))

typedef struct lang lang_t;

struct lang
{
    pattern_t   *patterns[MAX_PATTERNS+1];  /* One extra for NULL end marker */
    lang_t      *next;
    int     auto_wrap;
    int     expand_tabs;
    int     tab_stops;
    char    lang_name[LANG_NAME_LEN+1];
    char    name_spec[SPEC_LEN+1];          /* Filespec for source files */
    char    id_comment[LANG_ID_LEN+1];      /* E.g. #!/bin/csh */
    char    compiler_cmd[TWC_FILENAME_LEN+1];
    char    compile_flags[OPTION_LEN+1];
    char    compile_only_flag[COMPILE_ONLY_LEN+1];  /* E.g. -c */
    char    compile_output_flag[OUTPUT_FLAG_LEN+1]; /* E.g. -o */
    char    syntax_check_flag[SYNTAX_CHECK_LEN+1];  /* E.g. -fsyntax-only */
    char    compile_to_asm_flag[OPTION_LEN+1];      /* E.g. -s */
    char    preprocess_only_flag[OPTION_LEN+1];
    char    link_flags[OPTION_LEN+1];
    char    debugger_cmd[TWC_FILENAME_LEN+1];
    char    debugger_flags[OPTION_LEN+1];
    char    debugger_backtrace_cmd[BACKTRACE_LEN+1];
    char    run_prefix[TWC_FILENAME_LEN+1];
    char    upload_prefix[TWC_FILENAME_LEN+1];
    char    executable_name[TWC_FILENAME_LEN+1];    /* Compiler output */
    char    executable_spec[EXE_SRC_LEN+1]; /* How exe name is determined */
    char    error_msg_format[ERR_FORMAT_LEN+1];
};


