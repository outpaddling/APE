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
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <twintk.h>
#include <xtend/string.h>
#include "edit.h"
#include "protos.h"

extern term_t  *Terminal;

/***************************************************************
 * Print a message in the Swin window, blanking out a specified
 * number of columns first.
 ***************************************************************/

void    sprintw(int start_col, int field_len, char *format,...)

{
    extern win_t    *Swin;
    int     c, len;
    char    string[1024] = "";
    va_list arg_ptr;

    va_start(arg_ptr, format);

    vsnprintf(string, 1024, format, arg_ptr);
    
    if ( Swin == NULL )
    {
	fprintf(stderr,"%s\n",string);
	return;
    }
    
    if ((len = strlen(string)) > field_len)
    {
	len = field_len;
	string[field_len] = '\0';
    }

    /* Print message */
    tw_move_to(Swin, 0, start_col);
    tw_puts(Swin, string);

    /* Clear to end of field */
    for (c = len; c < field_len; ++c)
	tw_putc(Swin, ' ');
    tw_move_to(Swin,0,start_col+len);
    va_end(arg_ptr);
}


void    stat_mesg(mesg)
char    *mesg;

{
    sprintw(2,TWC_ST_LEN,mesg);
}


/**********************************************
 * Create global windows for edit status, etc.
 **********************************************/

void    global_wins(options)
opt_t *options;

{
    extern win_t    *Swin, *Bar_win;
    extern term_t   *Terminal;

    /* Set up menu bar on top of screen */
    Bar_win = tw_new_win(Terminal, 1, TCOLS(Terminal), 0, 0,
	NO_AUTO_SCROLL|NOCOLOR(options)|NOACS(options));
    draw_menu_bar(options);

    /* Set up status bar at bottom of screen */
    Swin = tw_new_win(Terminal, 1, TCOLS(Terminal)-1, TLINES(Terminal) - 1, 0,
	NO_AUTO_SCROLL|NOCOLOR(options)|NOACS(options));
    draw_status_bar(options);
}


void    draw_menu_bar(options)
opt_t *options;

{
    char    menu_bar[256];
    extern win_t    *Bar_win;
    
    iclear_image(&Bar_win->image);
    snprintf(menu_bar, 256, "  .File  .Edit  .Search  .Build  .Options  .Custom  %*s  .Help       ",
	TW_COLS(Bar_win)-59," ");
    imove_to(&Bar_win->image,0,0);
    tw_high_print(Bar_win,options->bar_fg,options->bar_bg,
	options->bar_hl_fg,options->bar_hl_bg,menu_bar);
    TW_REDRAW_WIN(Bar_win);   /* Should be able to use TW_RESTORE_WIN */
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *
 *  Bugs:
 *      Should take file_t argument so it can draw the correct edit
 *      mode and row, col instead of just printing <insert>
 ***************************************************************************/

void    draw_status_bar(opt_t *options)

{
    extern term_t   *Terminal;
    extern win_t    *Swin;

    if ( TCOLOR_TERM(Terminal) && !MONO_MODE(options) )
    {
	tw_set_foreground(Swin,options->status_fg);
	tw_set_background(Swin,options->status_bg);
    }
    else if ( THAS_REVERSE_MODE(Terminal) )
	TW_SET_MODES(Swin, REVERSE_MODE);
    else
	TW_SET_MODES(Swin, HIGHLIGHT_MODE);
	
    sprintw(2, TW_COLS(Swin), BAR_HELP);
    sprintw(TW_COLS(Swin)-22,8,"<insert>");
    tmove_to(Terminal,TLINES(Terminal)-1,TCOLS(Terminal)-2);
    tinsert_ch(Terminal,' ');
}


win_t    *panel_win(int rows, int cols,
		    int start_row, int start_col, opt_t *options)

{
    extern term_t   *Terminal;
    win_t   *win;
    
    win = tw_new_win(Terminal, rows, cols,
	    start_row, start_col,
	    NO_AUTO_SCROLL | SHADOW | NOACS(options) | NOCOLOR(options));
    tw_shadow(win);
    set_popup_color(win, options);
    TW_SET_BORDER(win, options->border);
    tw_draw_border(win);
    TW_RESTORE_WIN(win);
    return win;
}


win_t   *centered_panel_win(int rows, int cols, opt_t *options)

{
    return panel_win(rows, cols, TWC_CENTER_WIN, TWC_CENTER_WIN, options);
}


int     panel_get_string(file_t *file, opt_t *options,
	    size_t len, const char *prompt, const char *help,
	    tw_str_t string_type, char *string)

{
    int     status;
    win_t   *win = NULL;
    tw_panel_t panel = TWC_PANEL_INIT;

    /* Set up input window */
    win = centered_panel_win(7, 65, options);
    tw_init_string(&panel, 2, 3, len, 40, string_type,
		(char *)prompt, (char *)help, string);

    status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
    tw_del_win(&win);
    
    TW_RESTORE_WIN(file->window);
    
    return status;
}

