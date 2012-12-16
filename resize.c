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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include "twintk.h"
#include "bacon.h"
#include "edit.h"
#include "protos.h"

file_t          *File;
int             *Aw;
opt_t           *Options;
buff_t          *Cut_buff;
win_t           *File_list;
extern int      Shelled_out;
int             Resized = 0;

void    win_resize()

{
    extern win_t    *Swin, *Bar_win;
    extern term_t   *Terminal;
    static volatile int  too_small = 0;
    int     c, min_lines=24, min_cols=80;
    static char    msg[]="APE: Resize your terminal to at least %d x %d...";

/* Linux signal() function is old-fashioned, and needs to be reset */
#if defined(linux) || defined(sun) // sun causes char deletions and termination!
    signal(SIGWINCH,(sig_t)win_resize);
#endif

    /*
     *  If a resize occurs while running a subprocess, we don't want to
     *  redraw the APE screen immediately.  This will be done when the
     *  subprocess exits anyway.
     */
    
    if ( Shelled_out )
	return;
    
    Resized = 1;
    
    /* Reopen terminal in resized window */
    del_image(&Terminal->image,0);
    resize_terminal(Terminal);

    /* Check screen size */
    if ( File_list != NULL )
    {
	min_lines = TW_LINES(File_list)+4;
	min_cols = TW_COLS(File_list)+1;
    }
    too_small = (Terminal->lines < min_lines) || (Terminal->columns < min_cols);
    if ( too_small )
    {
	/* Display pause message */
	tclear_screen(Terminal);
	tmove_to(Terminal,TLINES(Terminal)/2-1,MAX(0,
		(int)(TCOLS(Terminal)-sizeof(msg))/2));
	tprintf(Terminal,msg,min_cols, min_lines);
	TFLUSH_OUT(Terminal);
	
	/* Wait for resize signal */
	sigpause(SIGWINCH);
	return;
    }

    /* Resize all file windows, menu bar, status window, and terminal */
    tw_del_win(&Swin);
    tw_del_win(&Bar_win);
    
    /* Re-create global windows */
    global_wins(Options);
    
    /*
     *  Unlist basic windows like menus, file text, etc. so that they
     *  don't get redrawn again by tw_redraw_all(), which is used to redraw
     *  non-standard windows like popups, file browsers, etc.
     */
    tw_unlist_win(Swin);
    tw_unlist_win(Bar_win);
    
    /* Re-create all file windows */
    for (c=0; c<Options->max_files; ++c)
    {
	if ( File[c].window != NULL )
	{
	    tw_del_win(&File[c].text);
	    if ( Options->scroll_bars )
	    {
		tw_del_win(&File[c].hscroll_bar);
		tw_del_win(&File[c].vscroll_bar);
	    }
	    tw_del_win(&File[c].window);
	    create_edit_win(File+c,Options);
	    tw_unlist_win(File[c].window);
	    tw_unlist_win(File[c].text);
	    if ( Options->scroll_bars )
	    {
		tw_unlist_win(File[c].hscroll_bar);
		tw_unlist_win(File[c].vscroll_bar);
	    }
	    if ( c != *Aw ) /* Do aw later so it's the last drawn */
	    {
		edit_border(File+c,Options);
		// Why was this here?  Is there a reason to redraw
		// file windows that aren't active?
		//update_win(File+c,Options,Cut_buff);
	    }
	}
    }
    
    /* Redraw screen */
    edit_border(File+*Aw,Options);
    update_win(File+*Aw,Options,Cut_buff);

    /* 
     *  Redraw any windows marked for redraw.  FIXME: This is inefficient 
     *  and annoying terminal I/O since some functions above redraw
     *  individual windows.
     */
    tw_redraw_all(Terminal);
    if ( File_list != NULL )
	TW_SYNC_CURSOR(File_list);
    TFLUSH_OUT(Terminal);
    
    /* FIXME: Re-list windows in case tw_redraw_all() is used elsewhere */
}


void    setup_resize(file,aw,options,cut_buff)
file_t  file[];
int     *aw;
opt_t   *options;
buff_t  *cut_buff;

{
    File = file;
    Aw = aw;
    Options = options;
    Cut_buff = cut_buff;
}

