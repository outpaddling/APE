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

#include <ctype.h>
#include <stdlib.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"


void    scroll_file(file, options, cut_buff, direction)
file_t *file;
opt_t *options;
buff_t  *cut_buff;
direc_t direction;

{
    int     lines;

    if ( !options->smooth_scroll )   /* 1/2 page up or down */
    {
	if (direction == SCROLL_FORWARD)
	    lines = MIN(TW_LINES(file->text) / 2, file->total_lines - file->curline); /* ;lkasjfk;lsjf;klsajfk;lsk;lf;kasfkasjfkasfk;asfjasfj */
	else
	    lines = -MIN(TW_LINES(file->text) / 2, file->curline + 1);
	file->topline += lines;
	update_win(file,options,cut_buff);
    }
    else    /* Scroll 1 line */
	switch(direction)
	{
	    case    SCROLL_FORWARD:
		if ( file->curline < file->total_lines)
		{
		    file->topline++;
		    tw_scroll_forward(file->text);
		    update_line(file,options,cut_buff,file->topline+TW_LINES(file->text)-1,file->leftcol);
		    update_cursor(file,options,cut_buff);
		}
		break;
	    case    SCROLL_REVERSE:
		if ( (int)file->curline >= 0 )
		{
		    file->topline--;
		    tw_scroll_reverse(file->text);
		    update_line(file,options,cut_buff,file->topline,file->leftcol);
		    update_cursor(file,options,cut_buff);
		}
		break;
	    default:
		break;
	}
}


void    move_up(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    int     x, y, newcol;

    synhigh_update(file,file->curline,options,cut_buff);
    if (file->curline != 0)
    {
	TW_FIND_CURSOR(file->text, y, x);
	--file->curline;

	newcol = MIN(file->line[file->curline].length, file->curcol);
	file->curchar = file->line[file->curline].buff + newcol;
    
	if (TW_CUR_LINE(file->text) == 0)
	    scroll_file(file, options, cut_buff, SCROLL_REVERSE);
	update_cursor(file,options,cut_buff);
    }
}


void    move_down(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    int     x, y, newcol;

    synhigh_update(file,file->curline,options,cut_buff);
    if (file->curline < file->total_lines - 1)
    {
	TW_FIND_CURSOR(file->text, y, x);
	++file->curline;

	/* Check line length before moving directly down */
	newcol = MIN(file->line[file->curline].length, file->curcol);
	file->curchar = file->line[file->curline].buff + newcol;

	/* Scroll if necessary */
	if (y == TW_LINES(file->text) - 1)
	    scroll_file(file, options, cut_buff, SCROLL_FORWARD);
	update_cursor(file,options,cut_buff);
    }
}


void    move_to_left_of_tab(file_t *file)

{
    file->curcol = ACTUAL_COL(file);
    while ( (file->curcol > 0) && (file->curchar[-1] == TAB_FILLER_CHAR) )
    {
	--file->curchar;
	--file->curcol;
    }
}


void    move_left(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    synhigh_update(file,file->curline,options,cut_buff);
    file->curcol = MIN(file->curcol, file->line[file->curline].length);
    if ( *file->curchar == '\t' )
	move_to_left_of_tab(file);
    if (file->curcol == 0)
    {
	synhigh_update(file,file->curline,options,cut_buff);
	if (file->curline > 0)
	{
	    move_up(file,options,cut_buff);
	    end_line(file,options,cut_buff);
	}
    }
    else
    {
	--file->curchar;
	file->curcol = file->curchar - file->line[file->curline].buff;
	update_cursor(file,options,cut_buff);
    }
}


void    end_line_no_redraw(file_t *file)

{
    file->curcol = file->line[file->curline].length;
    file->curchar = file->line[file->curline].buff + file->curcol;
}


void    end_line(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    synhigh_update(file,file->curline,options,cut_buff);
    end_line_no_redraw(file);
    update_cursor(file,options,cut_buff);
}


void    move_right(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    synhigh_update(file,file->curline,options,cut_buff);
    if (*file->curchar == '\0')
    {
	synhigh_update(file,file->curline,options,cut_buff);
	move_ret(file,options,cut_buff);
    }
    else
    {
	++file->curcol;
	++file->curchar;
	update_cursor(file,options,cut_buff);
    }
}


void    move_ret(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    char   *lptr;
    int     x, y;

    synhigh_update(file,file->curline,options,cut_buff);
    if (file->curline < file->total_lines - 1)
    {
	TW_FIND_CURSOR(file->text, y, x);
	++file->curline;
	lptr = file->line[file->curline].buff;
	file->curchar = lptr;
	while (isspace(*(file->curchar)))
	    ++file->curchar;
	file->curcol = file->curchar - lptr;
	if (TW_CUR_LINE(file->text) == TW_LINES(file->text) - 1)
	    scroll_file(file, options, cut_buff, SCROLL_FORWARD);
	update_cursor(file,options,cut_buff);
    }
}


void    move_unret(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    synhigh_update(file,file->curline,options,cut_buff);
    if (file->curline > 0)
    {
	--file->curline;
	file->curcol = 0;
	file->curchar = file->line[file->curline].buff;
	update_cursor(file,options,cut_buff);
    }
}


void    home_file_no_redraw(file_t *file, opt_t *options, buff_t *cut_buff)

{
    file->topline = 0;
    file->curline = 0;
    file->curcol = 0;
    file->curchar = file->line[0].buff;
}


void    home_file(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    home_file_no_redraw(file, options, cut_buff);
    update_cursor(file,options,cut_buff);
    update_win(file,options,cut_buff);
}


void    end_file_no_redraw(file_t *file)

{
    file->curline = file->total_lines - 1;
    file->topline = MAX((long)file->total_lines - TW_LINES(file->text), 0);
    file->curchar = file->line[file->curline].buff;
    file->curcol = 0;
}


void    end_file(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    end_file_no_redraw(file);
    update_cursor(file,options,cut_buff);
    update_win(file,options,cut_buff);
}


void    begin_line(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    synhigh_update(file,file->curline,options,cut_buff);
    file->curcol = 0;
    file->curchar = file->line[file->curline].buff;
    update_cursor(file,options,cut_buff);
}


void    page_up(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     lines_up;
    
    lines_up = MIN(TW_LINES(file->text)-1, file->topline);
    file->curline -= lines_up;
    file->topline -= lines_up;
    file->curchar = file->line[file->curline].buff +
	MIN(file->line[file->curline].length, file->curcol);
    update_cursor(file,options,cut_buff);
    if ( lines_up != 0 )
	update_win(file,options,cut_buff);
}


void    page_down(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     lines;

    /* Don't try to scroll if end of file is on screen */
    if (file->total_lines - file->topline > TW_LINES(file->text)-1)
    {
	lines = MIN(TW_LINES(file->text)-1,
	    file->total_lines - file->topline - TW_LINES(file->text));
	file->curline += lines;
	file->topline += lines;
	file->curchar = file->line[file->curline].buff +
	    MIN(file->line[file->curline].length, file->curcol);
	update_cursor(file,options,cut_buff);
	if ( lines != 0 )
	    update_win(file,options,cut_buff);
    }
}


void    next_word(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    if (isalnum(*file->curchar))
	while (ISIDENT(*file->curchar) && (*file->curchar != '\0'))
	    move_right(file,options,cut_buff);
    else
	move_right(file,options,cut_buff);
    while (isspace(*file->curchar) || ((*file->curchar == '\0') &&
				   (file->curline < file->total_lines - 1)))
	move_right(file,options,cut_buff);
}


void    last_word(file,options,cut_buff)
file_t *file;
opt_t *options;
buff_t  *cut_buff;

{
    move_left(file,options,cut_buff);
    while ((isspace(*file->curchar)) && ((file->curline > 0) || (file->curcol > 0)))
	move_left(file,options,cut_buff);
    if (isalnum(*file->curchar))
    {
	while ((ISIDENT(*file->curchar)) &&
	       ((file->curline > 0) || (file->curcol > 0)))
	    move_left(file,options,cut_buff);
	move_right(file,options,cut_buff);
    }
}


void    page_top(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     x, y;

    file->topline = file->curline;
    TW_FIND_CURSOR(file->text, y, x);
    update_win(file,options,cut_buff);
}


void    back_tab(file,options,cut_buff)
file_t  *file;
opt_t *options;
buff_t  *cut_buff;

{
    do
	move_left(file,options,cut_buff);
    while ( file->curcol % options->indent_size );
}


void    gotoline(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     status;
    long    linenum = 0;
    win_t   *win;
    tw_panel_t panel = TWC_PANEL_INIT;

    win = centered_panel_win(7, 65, options);
    tw_init_long(&panel, 2, 3, 0, file->total_lines,
		"Go to line? ","", &linenum);

    status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
    tw_del_win(&win);
    TW_RESTORE_WIN(file->window);

    if ( (status != TWC_INPUT_CANCEL) && (linenum >= 1) )
    {
	move_to(file,options,cut_buff,linenum-1,0L);
	update_win(file,options,cut_buff);
    }
}


void    move_to(file,options,cut_buff,linenum,col)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;
long    linenum,col;

{
    file->curline = MIN(linenum,file->total_lines-1);
    file->curcol = MIN(col,(long)file->line[file->curline].length);
    file->curchar = file->line[file->curline].buff + file->curcol;
    
    /* Need to scroll? */
    if ( (linenum < file->topline) ||
	 (linenum >= file->topline + TW_LINES(file->text)) )
	file->topline = MAX(file->curline - (int)TW_LINES(file->text) / 2, 0);
    update_cursor(file,options,cut_buff);
}


/*
 * Place cursor in correct screen location, assuming topline hasn't changed.
 */
 
void    update_cursor(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    check_column(file,options,cut_buff);
    //sprintw(40,20,"Updating to %d %d",file->leftcol,ACTUAL_COL(file));
    tw_move_to(file->text,file->curline-file->topline,
		ACTUAL_COL(file)-file->leftcol);
}


/*
 * Place cursor in correct screen location, assuming topline hasn't changed.
 */
 
void    update_cursor_no_snap(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    check_column_no_snap(file,options,cut_buff);
    //sprintw(40,20,"Updating to %d %d",file->leftcol,ACTUAL_COL(file));
    tw_move_to(file->text,file->curline-file->topline,
		ACTUAL_COL(file)-file->leftcol);
}


/****************************************************************************
 * Description: 
 *  If we landed on a TAB_FILLER_CHAR, advance to the tab.
 ***************************************************************************/

void    snap_to_tab(file_t *file)

{
    /* Landed on a TAB filler? */
    //sprintw(2,20,"landed on %d",*file->curchar);
    while (*file->curchar == TAB_FILLER_CHAR)
	++file->curchar;
}


int     check_column(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    snap_to_tab(file);
    return check_column_no_snap(file,options,cut_buff);
}


/* 
 * Page left or right if the new column requires it
 */

int     check_column_no_snap(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     cur_col,
	    text_width = TW_COLS(file->text) - 1,   // Win width - scroll bar
	    moved = 0;
    
    cur_col = ACTUAL_COL(file);
    
    /*
     *  Always page to far left if cursor is within screen width.
     *  It's annoying to have to move left to see the first cols of a line.
     */
    if ( (cur_col < text_width) && (file->leftcol > 0) )
    {
	file->leftcol = 0;
	moved = 1;
	update_win(file,options,cut_buff);
    }
    /* Moved beyond left side of window? */
    else if ( cur_col < file->leftcol )
    {
	/* file->leftcol = cur_col - (cur_col % (text_width/2)); */
	file->leftcol = cur_col;
	moved = 1;
	update_win(file,options,cut_buff);
    }

    /* Moved beyond right side of window? */
    else if ( cur_col > file->leftcol + text_width - 1 )
    {
	/* file->leftcol = MIN(cur_col - cur_col % (text_width/2),
		file->max_line_len-text_width/2); */
	file->leftcol = cur_col - text_width + 1;
	moved = 1;
	update_win(file,options,cut_buff);
    }
    file->old_col = cur_col;
    return moved;
}

