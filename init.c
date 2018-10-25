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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <twintk.h>
#include <bacon.h>
#include "edit.h"
#include "protos.h"


void    load_keymap(terminal)
term_t *terminal;

{
    char    *term_type;
    
    /* Pop-down menu esc sequences */
    taddseq(terminal, "\033f", FILE_TWC_MENU);
    taddseq(terminal, "\033e", EDIT_TWC_MENU);
    taddseq(terminal, "\033s", SEARCH_TWC_MENU);
    taddseq(terminal, "\033b", BUILD_TWC_MENU);
    taddseq(terminal, "\033o", OPTIONS_TWC_MENU);
    taddseq(terminal, "\033c", CUSTOM_TWC_MENU);
    taddseq(terminal, "\033h", HELP_TWC_MENU);
    taddseq(terminal, "\033k", RESUME_SEARCH);
    
    /* Pop down menus with ALT keys */
    taddseq(terminal, "\346", FILE_TWC_MENU);
    taddseq(terminal, "\345", EDIT_TWC_MENU);
    taddseq(terminal, "\363", SEARCH_TWC_MENU);
    taddseq(terminal, "\342", BUILD_TWC_MENU);
    taddseq(terminal, "\357", OPTIONS_TWC_MENU);
    taddseq(terminal, "\343", CUSTOM_TWC_MENU);
    taddseq(terminal, "\350", HELP_TWC_MENU);

    /* Various Esc followed by other keys */
    taddseq(terminal, "\033a", PAGE_TOP);
    /*taddseq(terminal, "\033g", GOTO_LINE);*/
    taddseq(terminal, "\033i", COMPILE_PROG);
    taddseq(terminal, "\033l", LAST_LINE);
    taddseq(terminal, "\033m", MARK_AREA);
    taddseq(terminal, "\033n", NEXT_ERROR);
    taddseq(terminal, "\033r", RUN_PROG);
    taddseq(terminal, "\033w", UPLOAD_PROG);
    taddseq(terminal, "\033p", PASTE_AREA);
    taddseq(terminal, "\033t", FIRST_LINE);
    taddseq(terminal, "\033u", UNDO);
    taddseq(terminal, "\033y", SYNTAX_CHECK);
    taddseq(terminal, "\033x", BUILD_PROG);
    taddseq(terminal, "\033\033", SAVE_FILE);
    taddseq(terminal, "\033/", QUICK_FIND);
    taddseq(terminal, "\033\r", INVOKE_MACRO);
    taddseq(terminal, "\033?", KEY_HELP);
    taddseq(terminal, "\033j", JOIN_LINES);
    taddseq(terminal, "\0331", MARK_COL1);
    
    term_type = getenv("TERM");
    if (term_type == NULL)
    {
	fprintf(stderr, "Terminal not defined in environment.\n");
	exit(1);
    }
}


/*
 * Process command line arguments
 */
 
int     do_args(argc, argv, files, options)
int     argc;
char   *argv[];
file_t  files[];
opt_t  *options;

{
    extern int  errno;
    int     aw = NO_FILES, arg, a;
    char    startup_dir[PATH_LEN + 1],
	    *buttons[2] = OK_BUTTON;

    if (argc > 1)
    {
	if ( getcwd(startup_dir, PATH_LEN) == NULL )
	{
	    popup_mesg(strerror(errno),buttons,options);
	}
	for (arg = MIN(argc - 1, options->max_files); arg > 0; --arg)
	{
	    if (*argv[arg] != '-')
	    {
		chdir(startup_dir);     /* Open file may do a chdir() */
		if ((a = open_file(files, argv[arg], options, OPEN_FLAG_NORMAL)) != CANT_OPEN)
		    aw = a;
	    }
	}
	if (aw == NO_FILES)
	    aw = open_file(files, "untitled", options, OPEN_FLAG_NORMAL);
    }
    else
	aw = open_file(files, "untitled", options, OPEN_FLAG_NORMAL);
    return aw;
}


/* 
 * Check for a makefile and offer to set up the project
 */
 
int     check_makefile(project,makefile,file,options)
proj_t  *project;
char    *makefile;
file_t  *file;
opt_t   *options;

{
    struct stat st;
    /* Need msg2 static sized array so message can be modified */
    /* Constant initializers to pointers are in read-only segment */
    char        msg[100],
		*buttons[3] = YES_NO_BUTTONS;
    int         retcode = 0;
    
    snprintf(msg, 99, "Use \"%s?\"", makefile);
    if ( stat(makefile,&st) == 0 )
    {
	msg[5] = *makefile;
	if ( tolower(popup_mesg(msg,buttons,options)) == 'y' )
	{
	    init_makefile(project,makefile,file,options);
	    retcode = 1;
	}
    }
    else
	retcode = -1;   /* File does not exist */
    TW_RESTORE_WIN(file->window);
    return retcode;
}


/*
 * See if APE is running on the proper host
 */
 
void    check_hostname()

{
    static char legal_host[PATH_LEN + 1] = "Change this";
    char    this_host[PATH_LEN + 1];

    gethostname(this_host, PATH_LEN);
    if (strcmp(this_host, legal_host) != 0)
    {
	fprintf(stderr, "Warning: APE is not legally installed on %s.\n",
		this_host);
	fputs("Please review the license agreement.\n", stderr);
	fputs("Press return to continue...\n", stderr);
	getchar();
    }
}


void    check_args(argv)
char    *argv[];

{
    char    **p;
    
    for (p=argv; *p != NULL; ++p)
    {
	if ( **p == '-' )
	{
	    switch(*(*p+1))
	    {
		case    'h':
		default:
		    usage("%s [-h[elp]] [filespec]\n",*argv);
		    break;
	    }
	}
    }
}


int     init_xterm()

{
    char    *display_name, *window_str, home[PATH_LEN+1],
	    xdefaults[PATH_LEN+1];
    struct stat st;
    
    /* Set up .Xdefaults if it doesn't exist */
    get_home_dir(home,PATH_LEN);
    snprintf(xdefaults,PATH_LEN,"%s/.Xdefaults",home);
    if ( stat(xdefaults,&st) == -1 )
	if ( spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"xapedefaults",NULL) != 0 )
	{
	    fprintf(stderr,
		"Installation problem: Unable to run \"xapedefaults\".\nPress return to continue...\n");
	    fflush(stderr);
	    getchar();
	}
	
    /* See if we're in the X11 env */
    if ( (display_name=getenv("DISPLAY")) == NULL )
	return -1;
    
    /* Get Window ID from env set by xterm */
    if ( (window_str = getenv("WINDOWID")) == NULL )
	return -1;
    else
	return strtol(window_str,NULL,10);
}


void    register_signal_handlers()

{
    /* Request notification when background jobs end */
#ifdef SCO_SV
    sigset(SIGALRM, SIG_IGN);
    /*sigset(SIGCHLD, (sig_t)notify);*/
    sigset(SIGWINCH, (sig_t)win_resize);
    sigset(SIGINT, SIG_IGN);    /* Protect editor when running child */
    sigset(SIGTERM, (sig_t)kamakaze);
    signal(SIGQUIT, SIG_IGN);
#else
    signal(SIGALRM, SIG_IGN);
    /*signal(SIGCHLD, (sig_t)notify);*/
    signal(SIGWINCH, (sig_t)win_resize);
    signal(SIGINT, SIG_IGN);    /* Protect editor when running child */
    signal(SIGTERM, (sig_t)kamakaze);
    signal(SIGQUIT, SIG_IGN);
#endif
}


void    setup_terminal()

{
    extern term_t   *Terminal;
    
    if ((Terminal = init_term(stdin, stdout, stderr,
			      NULL, MOUSE_ON)) == NULL)
    {
	fprintf(stderr, "APE: Cannot use \"%s\" terminal.\n",
		Terminal->term_type);
	exit(1);
    }
    if ((TLINES(Terminal) < 24) || (TCOLS(Terminal) < 80))
    {
	fprintf(stderr, "APE: Sorry, terminal must have at least 24 lines and 80 columns.\n");
	exit(1);
    }

    /* Set terminal screen attributes */
    if ( Terminal->ena_acs != NULL )
	raw_print(Terminal,Terminal->ena_acs);
    tset_modes(Terminal, NORMAL_MODE);
    tset_foreground(Terminal, WHITE);
    tset_background(Terminal, BLACK);
    tclear_screen(Terminal);
    TCURSOR_NORMAL(Terminal);

#if 0
    /* Initialize xterm mouse if possible */
    if ( (Terminal->windowid = init_xterm()) != -1 )
    {
	define_cursor(XC_left_ptr);
    }
#endif
    TFLUSH_OUT(Terminal);
}


#if 0
void    define_cursor(cursor_font)
int     cursor_font;

{
    char    font[10];
    
    snprintf(font,9,"%d",cursor_font);
    
    /* Change mouse cursor appearance */
    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"defcursor",font,NULL);
}
#endif


void    init_compiler_lines(files,options)
file_t  files[];
opt_t   *options;

{
    size_t  c, f;
    
    for (f=0; f<options->max_files; ++f)
	if ( files[f].window != NULL )
	    for (c=0; c<files[f].total_lines; ++c)
		files[f].line[c].compiler_line = c+1;
}

