/*
    Copyright (c) 1993-Present, Jason W. Bacon, Acadix Software Systems
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. Redistributions
    in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or
    other materials provided with the distribution. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

/* Includes needed in edit.h or protos.h */
#if !defined(__TIME_H__) && !defined(_TIME_H_)
#include <time.h>
#endif

#if !defined(_CTYPE_H_) && !defined(__CTYPE_H__)
#include <ctype.h>
#endif

#if !defined(_WINDOWS_H_)
#include "twintk.h"
#endif

#if !defined(_PARE_H_)
#include "pare.h"
#endif

#define APE_VERSION     "3.5.1"

#define EMPTY_FILE(f) (((f)->total_lines == 1) && ((f)->line[0].length == 0))

/* End full screen shell-out options */
#define EFS_PAUSE       1
#define EFS_NO_PAUSE    0

#define PROMPT_BEFORE_CLOSE     1
#define NO_PROMPT_BEFORE_CLOSE  0

#define APERC               "options.rc"

#define APE_BROWSER         "runsomebrowser"
#define APE_FIFO            ".ape_fifo"

/* Edit buffer */
#define MAX_LINES           256 /* Initial allocation */
#define DEFAULT_TAB_STOPS   8   /* Tab character behavior */
#define DEFAULT_INDENT_SIZE 4   /* Indentation level for program code */

#define OK                  0
#define NO_FILES            -1
#define NO_DIR              -1
#define CANT_BUILD          -1
#define CANT_OPEN           -1  /* For any functiion that can't open file */
#define NO_FREE_WIN         -1
#define CANT_LOAD           -1
#define NOMEM               -1
#define CANT_SAVE           -1

#define OPTION_LEN          1024
#define EXT_LEN             40
#define SPEC_LEN            80
#define TWC_SHORT_NAME_LEN  14
#define SEARCH_STR_LEN      80
#define SEARCH_WIN_LEN      30
#define CMD_LEN             256
#define MESG_LEN            80
#define MAX_ARGS            40
#define ERROR_LEN           128
#define ERROR_WIN_ROWS      6
#define YES_NO_LEN          3
#define BAR_HELP            "Hit <Esc>-letter (or Alt+letter) for pop-down menus."
#define OK_BUTTON           { "[ OK ]",NULL }
#define OK_CANCEL_BUTTON    { "[ OK ]","[ Cancel ]",NULL }
#define YES_NO_BUTTONS      { "[ Yes ]","[ No ]",NULL }
#define YES_NO_CANCEL_BUTTONS   { "[ Yes ]","[ No ]","[ Cancel ]",NULL }
#define YES_NO_ENUM         { "Yes", "No", NULL }
#define CANT_COMPILE_MSG    "Can't compile: unknown language."
#define NO_LANGUAGE_OPTS    -2
#define MENU_NO_SELECTION   -1

#include "macros.h"

/* Convert options flag to tw_new_win flag */
#define NOCOLOR(o)      BOOL_TO_NO_COLOR(MONO_MODE(o))
#define NOACS(o)        BOOL_TO_NO_ACS((o)->no_acs)

/* Color schemes */
enum { MONO_SCHEME, WHITE_SCHEME, BLUE_SCHEME, BLACK_SCHEME, CYAN_SCHEME, USER_DEFINED };
#define MONO_SCHEME_STR     "Monochrome"
#define WHITE_SCHEME_STR    "White theme"
#define BLUE_SCHEME_STR     "Blue theme"
#define BLACK_SCHEME_STR    "Black theme"
#define CYAN_SCHEME_STR     "Cyan theme"
#define USER_DEFINED_STR    "User defined"
#define MONO_MODE(o)        ((o)->color_scheme == MONO_SCHEME)

#ifndef MAX
#define MAX(a,b)        ( (a)>(b) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a,b)        ( (a)<(b) ? (a) : (b) )
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    -1  /* All bits 1 */
#endif

typedef unsigned char   uchar_t;
typedef signed char     schar_t;

#define ACTUAL_COL(file)    ( (int)((file)->curchar - \
			      (file)->line[(file)->curline].buff) )

#define SET_MODIFIED(f)\
    (f)->modified = TRUE;\
    (f)->line[(f)->curline].needs_update = TRUE

typedef enum { SCROLL_FORWARD, SCROLL_REVERSE } direc_t;

typedef enum { OBJECT, ASSEMBLY } out_t;

#include "synhigh.h"

#include "language.h"

/* Global options */
typedef struct
{   
    time_t  loaded;     /* Time options were loaded from .aperc */
    
    /* Build options linked list */
    lang_t  *lang_head;
    
    /* Window borders */
    border_t    border;     /* Border characters for pop-up windows */
    
    /* Booleans: Keep use_delays first and expand_tabs last */
    int     use_delays;     /* Use terminfo delays */
    int     seek_make;      /* Look for makefile at startup time */
    int     reverse_menu;   /* Use reverse video to highlight menu items */
    int     use_html;       /* Use HTML docs instead of "man" */
    int     scroll_bars;    /* Display scroll bars on edit window */
    /* For now, ape must be restarted before drawing newly enabled scroll_bars.
       The whole text window must be deleted, recreated and redrawn
       in order to add scroll bars on the fly.
       */
    int     scroll_bars_at_startup;
    int     no_acs;         /* Use standard border chars instead of ACS */
    int     case_sensitive; /* Non-zero if search is currently CS */
    int     search_forward; /* Non-zero if currently searching forward */
    int     show_column;    /* If non-zero, cursor column is shown */
    int     smooth_scroll;  /* If true, scroll window, else page */
    int     terminfo_mouse; /* Enable terminfo mouse? */
    int     trap_noise;     /* Refuse input if line noise detected */
    int     expand_tabs;    /* Convert tabs to spaces on input */
    /* End booleans */
    
    /* Integers: Keep tab_stops first and edit_border last */
    unsigned int    indent_size;      
    unsigned int    max_files;      /* Max files open at once */
    unsigned int    color_scheme;
    unsigned int    bar_fg;
    unsigned int    bar_bg;
    unsigned int    bar_hl_fg;
    unsigned int    bar_hl_bg;
    unsigned int    menu_fg;
    unsigned int    menu_bg;
    unsigned int    menu_hl_fg;
    unsigned int    menu_hl_bg;
    unsigned int    status_fg;
    unsigned int    status_bg;
    unsigned int    title_fg;
    unsigned int    title_bg;
    unsigned int    text_fg;
    unsigned int    text_bg;
    unsigned int    edit_border;    /* Character for title line of edit window */

    /* Do we still need this, or should we use the INSTALL_PREFIX macro? */
    char    install_prefix[PATH_LEN+1]; /* Where HTML docs etc. are kept */
    char    browser[TWC_FILENAME_LEN+1]; /* HTML browser */
    char    shell[PATH_LEN+1];          /* Shell to pass commands to */
    char    ishell[PATH_LEN+1];         /* For interactive sub-shell */
    char    file_spec[TWC_SPEC_LEN+1];  /* File listing mask for list */
    char    include_path[PATH_LEN+1];   /* Additional headers to search */
    char    lib_path[PATH_LEN+1];       /* Additional libraries to search */
}   opt_t;

/* File structure */

#define BITS                6           /* Default block size of 64 bytes */
#define BLOCK_SIZE          (1<<BITS)
/* FIXME: Why is l+1 needed?  Problem began suddenly Dec 2008 */
#define LINE_BUFF_SIZE(l)   (buff_len_t)((((l) >> BITS) + 1) << BITS)
#define ALLOC_LINE(file,l,len)\
	(file)->line[l].length = (len);\
	(file)->line[l].buff = malloc(LINE_BUFF_SIZE(len+1));\
	(file)->line[l].compiler_line = LINE_NOT_COMPILED;\
	(file)->line[l].needs_update = TRUE;\
	(file)->line[l].start_pattern = NO_PATTERN;

#define LINE_NOT_COMPILED    -1

typedef unsigned short  buff_len_t; /* Check fscanf()s before changing */

/*
 * There's one of these for every line in every open file, so keep
 * the fields small and avoid arranging them in a way that will cause
 * padding.
 */
typedef struct
{
    char        *buff;          /* 4 bytes */
    size_t      compiler_line;  /* 4 or 8 bytes */
    buff_len_t  length;         /* 2 bytes */
    char        needs_update;   /* Has this line been modified? */
    schar_t     start_pattern;  /* char is unsigned by default in gcc ppc */
				/* Assuming buff_len_t is short? */
}   line_t;

/* One less than a multiple of BLOCK_SIZE */
#define MAX_LINE_LEN ((size_t)(1 << (sizeof(buff_len_t)<<3))-1)

#include "undo_stack.h"

typedef struct
{
    /* Window info */
    win_t   *window;        /* Outer window boundary */
    win_t   *text;          /* Writable area of window */
    win_t   *vscroll_bar;
    win_t   *hscroll_bar;

    /* Buffer manipulation info */
    line_t  *line;
    size_t  max_lines;      /* Number of line pointers available */
    size_t  total_lines;    /* Total lines in the file */
    size_t  max_line_len;   /* 72 for Fortran, 132 otherwise */
    size_t  curline;        /* Index of current line */
    size_t  topline;        /* First line on the current screen */
    size_t  old_curline;
    size_t  leftcol;        /* Leftmost column */
    size_t  old_leftcol;
    size_t  curcol;         /* Current column */
    size_t  old_col;        /* Most recent previous column */
    size_t  start_line;     /* Starting line for search and replace */
    size_t  start_col;      /* Starting column for search and replace */
    int     search_wrapped; /* 0 on new search, 1 when back to start */
    char    *curchar;       /* Pointer to current char in buff */
    
    int     modified;       /* True if buffer modified since last save */
    int     lang_rebuild;   /* True if language options have changed */
    int     saved_once;     /* True if file has been saved at least once */
    int     read_only;      /* File cannot be saved */
    int     insert;         /* Insert (1) or replace (0) mode */
    time_t  save_time;      /* Time of last save */
    int     crypt;          /* True if file is saved encrypted */
    int     notabs;         /* Needed for LaTeX verbatim, etc. 
				Obsoleted by expand_tabs. */
    int     expand_tabs;
    lang_t  *lang;          /* Language options */
    
    undo_t  undo_record;

    /* Location and appearance of highlighted group symbol */
    size_t  high_line;
    size_t  high_col;
    int     high_modes;
    char    high_fg;
    char    high_bg;
    
    /* General file info */
    char    cwd[PATH_LEN+1];                /* Working directory for file */
    char    run_directory[PATH_LEN+1];
    char    source[TWC_FILENAME_LEN+1];     /* Name of source file */
    char    executable[TWC_FILENAME_LEN+1]; /* Name of executable */
    char    short_src[TWC_SHORT_NAME_LEN+1];
    char    run_cmd[CMD_LEN+1];
    unsigned char   line_style;             /* Bit mask for CR, NL */
}   file_t;

#define LINE_STYLE_CR   0x01
#define LINE_STYLE_NL   0x02

#define FILE_NOT_OPEN   -1
#define FILE_USES_CR(f)     ((f)->line_style & LINE_STYLE_CR)
#define FILE_USES_NL(f)     ((f)->line_style & LINE_STYLE_NL)
#define LINE_STYLE_UNSET(f) ((f)->line_style == 0)

#define SET_LINE_STYLE(f,s) ((f)->line_style |= (s))

/* Buffer for marking an area for deletion or copy */

typedef struct
{
    size_t      start_line;     /* End points of marked area */
    size_t      end_line;
    buff_len_t  start_col;
    buff_len_t  end_col;
    size_t      start_top;      /* Where to go after delete */
    size_t      end_top;
    int         deleted;        /* Boolean flag true after delete */
    size_t      marked_lines;
    size_t      start_line_len;
    FILE        *fp;
    file_t      *file;
    size_t      old_line;       /* Holds previous position for highlighting */
    buff_len_t  old_col;
}   buff_t;

typedef struct
{
    char    filename[PATH_LEN+1];
    FILE    *fp;
}   err_t;

#define ERR_INIT    { ".ape_compile_errors", NULL }

#include "make.h"

#include "custom.h"

/* Standard control-keys */

/* Ctrl+a is reserved for comm programs e.g. minicom */
#define BEGIN_LINE      '\002'  /* Ctrl+b */
#define MARK_AREA       '\003'  /* Ctrl+c */
#define DELETE_TWC_CHAR '\004'  /* Ctrl+d */
#define END_LINE        '\005'  /* Ctrl+e */
#define QUICK_FIND      '\006'  /* Ctrl+f */
#define GOTO_LINE       '\007'  /* Ctrl+g */
#define BACKSPACE       '\010'  /* Ctrl+h */
#define TAB             '\011'  /* Ctrl+i */
#define NL              '\012'  /* Ctrl+j */
#define DEL_TO_EOLN     '\013'  /* Ctrl+k */
#define UPDATE_WIN      '\014'  /* Ctrl+l */
#define CR              '\015'  /* Ctrl+m */
#define FIND_NEXT       '\016'  /* Ctrl+n */
#define DEL_WORD        '\017'  /* Ctrl+o */
#define PREVIOUS_WORD   '\020'  /* Ctrl+p */
#define RESUME          '\021'  /* Ctrl+q */
#define TOGGLE_INS      '\022'  /* Ctrl+r */
#define KEY_SAVE_FILE   '\023'  /* Ctrl+s */
#define TOGGLE_FILE     '\024'  /* Ctrl+t */
#define PREVIOUS_PAGE   '\025'  /* Ctrl+u */
#define KEY_PASTE_AREA  '\026'  /* Ctrl+v */
#define NEXT_WORD       '\027'  /* Ctrl+w */
#define CUT_AREA        '\030'  /* Ctrl+x */
#define NEXT_PAGE       '\031'  /* Ctrl+y */
#define UNDO            '\032'  /* Ctrl+z */

/* Key sequence codes */
typedef enum { UNDEFINED = 512,

/* Menus */
FILE_TWC_MENU, EDIT_TWC_MENU, SEARCH_TWC_MENU, BUILD_TWC_MENU, OPTIONS_TWC_MENU, CUSTOM_TWC_MENU,
HELP_TWC_MENU, PAGE_TOP, 

/* Cursor motion */
NEXT_ERROR, FIRST_LINE, LAST_LINE,

/* Edit functions */
DELETE_ARGUMENT, JOIN_LINES, PASTE_AREA,
MARK_COL1,

/* Hot-keys */
RUN_PROG, UPLOAD_PROG, BUILD_PROG, COMPILE_PROG, SYNTAX_CHECK, SAVE_FILE,
CONTEXT_HELP, RESUME_SEARCH, INVOKE_MACRO, POSITION_CURSOR,

/* Mouse stuff */
VBAR_CLICK, HBAR_CLICK,

/* Error key codes */
NO_MACROS

} keyf_t;


/* Menu border characters */
typedef struct
{
    char    top_left[4];
    char    top[4];
    char    top_right[4];
    char    right[4];
    char    bottom_right[4];
    char    bottom[4];
    char    bottom_left[4];
    char    left[4];
    char    edit_border[4];
}   bord_t;


#define SCREEN_OPTS     29
#define MITW_OPTIONS    11
#define SYNTAX_OPTIONS  32

/* group symbol matching */
#define ISCLOSING(c)    (__Syms[c] & 0x01)
#define ISOPENING(c)    (__Syms[c] & 0x02)
#define ISGROUP(c)      (__Syms[c])
#define MATCHING_OPEN_TWC_CHAR(c)   ( ((c)==')') ? '(' : (c) == '}' ? '{' : '[')
#define MATCHING_CLOSE_TWC_CHAR(c)   ( ((c)=='(') ? ')' : (c) == '{' ? '}' : ']')
#define TOKEN_MATCH(p,c)    ((*p) == (c) &&\
			     !((p[1] == '\'') && (p[-1] == '\'')) &&\
			     !((p[1] == '"') && (p[-1] == '"')))

#include "machdep.h"
