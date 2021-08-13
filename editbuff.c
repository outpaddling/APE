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
#include <string.h>
#include <ctype.h>
#include <twintk.h>
#include <xtend/string.h>
#include "edit.h"
#include "protos.h"

extern term_t   *Terminal;

void    insert_char(file_t *file, int key, opt_t *options, buff_t *cut_buff)

{
    size_t  curline = file->curline;
    
    if ( file->line[curline].length == file->max_line_len)
    {
	stat_mesg("Cannot insert: line is at maximum length.");
	return;
    }

    file->curcol = ACTUAL_COL(file);

    if ( file->expand_tabs )
	insert_expand_tabs(file, key, options, cut_buff);
    else
	insert_real_tabs(file, key, options, cut_buff);
}


void    insert_expand_tabs(file_t *file, int key, 
				    opt_t *options, buff_t *cut_buff)

{
    size_t  curline = file->curline;
    buff_len_t  len = file->line[curline].length;

    // sprintw(2,50,"len = %u  buff_size = %u",len,LINE_BUFF_SIZE(len));
    
    /* Expand buffer if needed */
    expand_buff_if_needed(file, curline, len + BLOCK_SIZE);
    
    /* Inserting a TAB? */
    if (key == '\t')
    {
	/* Caution: mutually recursive call */
	insert_expanded_tab(file, options, cut_buff);
    }
    else
    {
	/* Redraw screen if needed before adding new char */
	check_column(file, options, cut_buff);
	
	/* If in insert mode or at end of line, increase line length.
	   Also, if terminal has insert support, this will not cause
	   a wrap or scroll when the cursor is at the right edge.
	*/
	if ((file->insert) || (file->curcol == file->line[curline].length))
	{
	    tw_insertc(file->text, key);
	    memmove(file->curchar + 1, file->curchar, len - file->curcol + 1);
	    ++file->line[curline].length;
	}
	else    /* Otherwise, just replace the char at curchar */
	    tw_putc(file->text, key);
	
	undo_stack_push(&file->undo_record, file->curline, ACTUAL_COL(file),
		  UNDO_ITEM_INSERT_CHAR, UNDO_ITEM_NO_TEXT);

	/* Advance cursor */
	*file->curchar = key;
	++file->curchar;
	++file->curcol;
	
	/* Redraw screen again after insert if needed */
	check_column(file, options, cut_buff);
	SET_MODIFIED(file);
    }
}


void    insert_tab_fillers(file_t *file,char *curchar,int count)

{
    int     curline = file->curline,
	    curcol = curchar - file->line[curline].buff;
    buff_len_t  len = file->line[curline].length;
    
    /* Expand buffer if needed */
    //sprintw(2,50,"Expanding buffer to %d",len+BLOCK_SIZE);
    expand_buff_if_needed(file, curline, len + BLOCK_SIZE);
    /* Buffer has been reallocated */
    curchar = file->line[curline].buff + curcol;
    
    /* Insert fillers */
    memmove(curchar + count, curchar, len - curcol + 1);
    file->line[curline].length += count;
    memset(curchar,TAB_FILLER_CHAR,count);
}


void    insert_real_tabs(file_t *file, int key, 
				opt_t *options, buff_t *cut_buff)

{
    size_t      curline = file->curline,
		count,
		ts = tab_stops(file);

    /* If on a tab, move left to the first space it covers */
    if ( *file->curchar == '\t' )
    {
	move_to_left_of_tab(file);
	sprintw(2,40,"insert moved to %d %d",file->curcol,ACTUAL_COL(file));
    }

    /* Redraw screen and relocate cursor if needed before adding new char
       Don't use update_cursor() or check_column(), since they call
       snap_to_tab().
    */
    update_cursor_no_snap(file,options,cut_buff);
    
    /* Inserting a TAB? */
    if (key == '\t')
    {
	count = ts - (ACTUAL_COL(file) % ts); /* Bytes to next tab stop */
	insert_tab_fillers(file,file->curchar,count);
	file->curchar += count;
	file->curcol += count;
	file->curchar[-1] = '\t';   /* Replace last filler with TAB */
	update_line(file,options,cut_buff,curline,file->leftcol);
	update_cursor(file,options,cut_buff);
    }
    else
    {
	/* If there are no fillers behind the tab, insert some. */
	/* Otherwise, just overwrite the filler with the new char. */
	if ( *file->curchar == '\t' )
	{
	    insert_tab_fillers(file,file->curchar,ts);
	    update_line(file,options,cut_buff,curline,file->leftcol);
	}

	if ( *file->curchar != TAB_FILLER_CHAR )
	{
	    if ((file->insert) || (file->curcol == file->line[curline].length))
	    {
		adjust_next_tab(file);
		update_line(file,options,cut_buff,curline,file->leftcol);
	    }
	}
	update_cursor_no_snap(file,options,cut_buff);

	tw_putc(file->text, key);
	*file->curchar = key;
	++file->curchar;
	++file->curcol;
    }
    
    /* Redraw screen again after insert if needed */
    check_column(file, options, cut_buff);
    SET_MODIFIED(file);
}


void    adjust_next_tab(file_t *file)

{
    char    *p;
    size_t  bytes;
    
    /* Shift bytes from here to next tab or eoln */
    for (p=file->curchar; (*p != '\0') &&
			(*p != TAB_FILLER_CHAR) &&
			(*p != '\t'); ++p)
	;
    bytes = p - file->curchar;
    
    if ( *p == '\t' )
	insert_tab_fillers(file,p,tab_stops(file));
    else if ( *p == '\0' )
	++bytes;    /* Include nul byte in memmove() */ 
    if ( *p != TAB_FILLER_CHAR )
	++file->line[file->curline].length;
    memmove(file->curchar + 1, file->curchar, bytes);
}


/**********************************************************
 * Expand a line buffer to accomodate newlen or more bytes
 * The argument newlen is NOT the new buffer size, but the
 * new length of the line.  The new buffer size is rounded
 * up to a multiple of BLOCK_SIZE.
 **********************************************************/

int     expand_buff_if_needed(file_t *file, size_t line, unsigned newlen)

{
    buff_len_t   col = file->curchar - file->line[line].buff;
    int     adjust_curchar;
    
    if ( newlen >= LINE_BUFF_SIZE(file->line[line].length) - 1 )
    {
	//sprintw(2,50,"Expanding buffer to %d",LINE_BUFF_SIZE(newlen));
	
	/* See if curchar is within this line */
	adjust_curchar = (col <= file->line[line].length);
    
	file->line[line].buff = (char *) realloc(file->line[line].buff,
					    LINE_BUFF_SIZE(newlen));
    
	/* Reset curchar to new line buffer if necessary */
	if (adjust_curchar)
	    file->curchar = file->line[line].buff + col;
	
	return 1;
    }
    else
	return 0;
}


void    insert_expanded_tab(file, options, cut_buff)
file_t *file;
opt_t  *options;
buff_t  *cut_buff;

{
    int     repeat, len, c, save_mode = file->insert;

    /* Override replace mode for tabs */
    file->insert = 1;

    /* Fortran 77 */
    if ((file->lang != NULL) &&
	(strcmp(file->lang->lang_name, "Fortran77") == 0))
    {
	if (file->curcol < 6)
	    repeat = 6 - file->curcol;
	else
	    repeat = options->indent_size - (file->curcol - 6) % options->indent_size;
    }
    else
	repeat = options->indent_size - file->curcol % options->indent_size;
    // col = file->curcol;
    len = file->line[file->curline].length;

    if (len + repeat >= file->max_line_len)
	return;

    /* Careful here - mutual recursion with insert_char() */
    for (c = 0; c < repeat; ++c)
	insert_char(file, ' ', options, cut_buff);
    file->insert = save_mode;
}


int     del_under(file_t *file,opt_t *options,buff_t *cut_buff)

{
    int     needspace = 0;
    file->curcol = ACTUAL_COL(file);
    char    *ptr = file->curchar;

#ifdef sun
    /*
     *  Solaris has a strange glitch where resetting the SIGWINCH
     *  signal caused a del char every other time.  This is a temporary
     *  hack until the problem is resolved.
     */
    extern int  Resized;
    
    if ( Resized )
    {
	Resized = 0;
	return 0;
    }
#endif

    SET_MODIFIED(file);

    /* If at end of line, combine with next line */
    if (*ptr == '\0')
    {
	if (file->curline != file->total_lines - 1)
	{
	    /* Add a space between the lines being joined
	     * if there isn't already one there (for documents)
	     * unless the next line is blank.
	     */
	    needspace = (ptr != file->line[file->curline].buff) &&
			!isspace(ptr[-1]) &&
			(file->curline < file->total_lines - 1) &&
			!strblank(file->line[file->curline+1].buff);
	    if ( needspace )
		insert_char(file,' ',options,cut_buff);
	    combine_lines(file,options,cut_buff);
	    if ( needspace )
		move_left(file,options,cut_buff);
	}
	return 0;
    }
    else                        /* Not combining lines */
    {
	if ( file->expand_tabs )
	    return del_under_expand_tabs(file,options,cut_buff);
	else
	    return del_under_real_tabs(file,options,cut_buff);
    }
}


int     del_under_real_tabs(file_t *file,opt_t *options,buff_t *cut_buff)

{
    //sprintw(2,50,"del_under_with_real_tabs() is not implemented.");
    char    *p = file->curchar;
    int     c;

    if ( *p == '\t' )
    {
	del_tab(file,p);
    }
    else
    {
	/* Find next tab or eoln */
	while ( (*p != TAB_FILLER_CHAR) && (*p != '\t') && (*p != '\0') )
	    ++p;
	
	/* Shift chars to tab or eoln */
	memmove(file->curchar,file->curchar+1,p-file->curchar);
	
	switch(*p)
	{
	    case    '\0':
		/* No tabs beyond deletion point.  Just reduce line. */
		--file->line[file->curline].length;
		break;
	    case    '\t':
		/* No fillers behind tab.  Add one. */
		p[-1] = TAB_FILLER_CHAR;
		break;
	    case    TAB_FILLER_CHAR:
		/* If there's room for another filler, add one.  Otherwise,
		   delete them all. */
		for (c=0; p[c] == TAB_FILLER_CHAR; ++c)
		    ;
		if ( c < tab_stops(file)-1 )
		    p[-1] = TAB_FILLER_CHAR;
		else
		{
		    /* Delete all fillers */
		    int col = p - file->line[file->curline].buff;
		    /* Line was already shifted above, so use p-1 */
		    memmove(p-1,p+c,file->line[file->curline].length - col);
		    file->line[file->curline].length -= (c+1);
		}
		break;
	}
    }
    
    /* Redraw necessary chars */
    /* This is inefficient - rework to just redraw necessary chars */
    update_line(file,options,cut_buff,file->curline,file->leftcol);
    update_cursor(file,options,cut_buff);
    return 0;
}


void    del_tab(file_t *file,char *p)

{
    char    *buff = file->line[file->curline].buff,
	    *first_filler;
    int     col = ACTUAL_COL(file);
    
    /* Move text after tab to location of first tab filler */
    for (first_filler = p-1; (first_filler >= buff) &&
		    (*first_filler == TAB_FILLER_CHAR); --first_filler)
	;
    ++first_filler; /* Loop above ends one back from target */
    memmove(first_filler,p+1,file->line[file->curline].length - col);
    file->line[file->curline].length -= (p - first_filler + 1);
    file->curchar = first_filler;
    file->curcol = ACTUAL_COL(file);
}


int     del_under_expand_tabs(file_t *file,opt_t *options,buff_t *cut_buff)

{
    char   *ptr = file->curchar;
    int     len, last, y, x;

    len = --file->line[file->curline].length;
    strlbasecpy(ptr, file->line[file->curline].buff, ptr + 1,
	       LINE_BUFF_SIZE(len + 1));
    last = file->leftcol + TW_COLS(file->text) - 2;
    tw_del_ch(file->text);
    if (len > last)         /* Bring in character from beyond right
			     * margin */
    {
	TW_FIND_CURSOR(file->text, y, x);
	tw_move_to(file->text, y, last-file->leftcol);
	tw_putc(file->text, file->line[file->curline].buff[last]);
	tw_move_to(file->text, y, x);
    }
    return 0;
}


void    combine_lines(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    buff_len_t   len;
    size_t  line,
	    curline = file->curline;
    char   *next;

    /* Make sure combined lines will fit */
    len = file->line[curline].length + file->line[curline + 1].length;
    if (len >= file->max_line_len)
	stat_mesg( "Lines are too long to combine.");
    else
    {
	/* Remove leading spaces from 2nd line */
	for (next = file->line[curline + 1].buff; isspace(*next); ++next)
	    --len;

	/* Expand buffer if needed */
	if ( expand_buff_if_needed(file, curline, len) )
	    file->curcol = file->curchar - file->line[curline].buff;

	strlbasecpy(file->curchar, file->line[curline].buff, next, LINE_BUFF_SIZE(len));
	file->line[curline].length = len;
	delete_lines(file, curline + 1, curline + 1);

	/* Update text window */
	update_line(file, options, cut_buff, curline, file->leftcol);
	line = TW_CUR_LINE(file->text);
	if (line != TW_LINES(file->text) - 1)
	{
	    tw_move_to(file->text, line + 1, 0);
	    tw_del_line(file->text);
	    update_line(file, options, cut_buff, file->topline + TW_LINES(file->text) - 1, file->leftcol);
	}
	update_cursor(file,options,cut_buff);
    }
}


void    del_left(file_t *file,opt_t *options,buff_t *cut_buff)

{
    /* Top of file? */
    if ((file->curline == 0) && (file->curcol == 0))
	return;

    if ( file->expand_tabs )
	del_left_expand_tabs(file,options,cut_buff);
    else
	del_left_real_tabs(file,options,cut_buff);
}


void    del_left_real_tabs(file_t *file, opt_t *options,
		    buff_t *cut_buff)

{
    sprintw(2,50,"del_left_with_real_tabs() is not implemented.");
}


void    del_left_expand_tabs(file_t *file, opt_t *options,
		    buff_t *cut_buff)

{
    int     c, repeat, save_mode = file->insert;
    char   *ptr;

    /* If line is blank from beginning, delete to previous tab stop */
    for (ptr = file->line[file->curline].buff;
	isspace(*ptr) && (ptr != file->curchar); ++ptr)
	;
    
    if (ptr == file->curchar)
    {
	file->insert = 1;       /* Always delete when backspace-tabbing */
	file->curcol = ptr - file->line[file->curline].buff;
	if (file->max_line_len == 72)   /* Fortran 77 */
	{
	    if (file->curcol == 0)
		repeat = 1;
	    else if (file->curcol <= 7)
		repeat = file->curcol;
	    else
		repeat = (file->curcol - 7) % options->indent_size + 1;
	}
	else
	    repeat = (file->curcol == 0) ? 1 :
		(file->curcol - 1) % options->indent_size + 1;
    }
    else
	repeat = 1;

    /* Delete 1 char or to previous tab stop */
    for (c = 0; c != repeat; ++c)
    {
	/* Don't redraw syntax highlighting after every char deletion */
	file->line[file->curline].needs_update = FALSE;
	
	move_left(file, options, cut_buff);
	if (file->insert)
	{
	    if ( del_under(file,options,cut_buff) )
		move_right(file,options,cut_buff);
	}
	else
	{
	    *file->curchar = ' ';
	    tw_puts(file->text, " \b");
	    SET_MODIFIED(file); /* Also handled by del_under */
	}
    }
    file->insert = save_mode;
}


/*******************************************************************
 * Insert a new line into the file buffer, adjusting the starting
 * position to match the previous line.  This function is intended
 * as a keyboard interface - it makes judgements about where to
 * start the new line which are not desirable when merging a file
 * or pasting a cut area.
 *******************************************************************/

void    new_line(file, options, cut_buff, time_diff)
file_t *file;
opt_t  *options;
buff_t  *cut_buff;
unsigned long   time_diff;

{
    int     line, col, already_inserted = 0;
    size_t  curline = file->curline;
    //extern term_t *Terminal;
    
    //sprintw(2,50, "new_line(): curline = %d", curline);
    //tgetc(Terminal);
    
    /* Update current line before we go */
    synhigh_update(file,curline,options,cut_buff);
    
    if ( new_line_buff(file,curline,options,time_diff) == NOMEM )
	return;

    /* Update window pointers */
    check_column(file, options, cut_buff);

    /* Make sure new line is in screen */
    TW_FIND_CURSOR(file->text, line, col);
    if (line == TW_LINES(file->text) - 1)
    {
	move_down(file, options, cut_buff);       /* Cause a scroll */
	move_up(file, options, cut_buff);
	TW_FIND_CURSOR(file->text, line, col);
	if (!options->smooth_scroll)
	    already_inserted = 1;
    }

    if (col > 0)                /* Splitting line? */
    {
	/* tw_move_to(file->text,line+1,0); *//* Why? */
	if (!already_inserted)
	{
	    /* Insert a new line below current line */
	    if ( line < TW_LINES(file->text)-1 )
		tw_move_to(file->text,line+1,col);
	    tw_insert_line(file->text);
	    tw_move_to(file->text,line,col);
	}
	update_line(file, options, cut_buff, file->curline, file->leftcol);    /* First half of split line */
	update_line(file, options, cut_buff, file->curline + 1, file->leftcol);        /* Second half */
    }
    else if (!already_inserted)
    {
	tw_insert_line(file->text);       /* Not splitting line */
    }

    tw_move_to(file->text, line, col);
    move_ret(file, options, cut_buff);

    /* Retag in case new line is within a token */
    file->line[curline].needs_update = TRUE;
    check_language(file,options,cut_buff);
    SET_MODIFIED(file);
    return;
}


int     new_line_buff(file,curline,options,time_diff)
file_t  *file;
size_t  curline;
opt_t   *options;
unsigned long   time_diff;

{
    buff_len_t   indent, len;
    char    *newptr, *oldptr, *p;
    
    /* Open a line in the pointer array */
    if (make_room(file, curline + 1, 1) == NOMEM)
	return NOMEM;

    /* If subsequent keystroke events were too close together for
       a human typist to have sent, assume it's a paste operation and
       disable auto-indent so it gets pasted as-is. It's possible that
       someone could type return this soon after another key, but
       extremely unlikely, and the only consequence would be having
       to manually indent the line. */
    if ( time_diff < 10000 )
    {
	indent = 0;
	len = strlen(file->line[curline].buff);
    }
    else
    {
	/* Figure out indent of current line */
	for (p=file->line[curline].buff, indent=0; isspace(*p); ++p, ++indent)
	    ;
	
	/* Increase indent if last character on previous line is '{', but
	   not if the cursor is on it. */
	for (p=file->line[curline].buff + file->line[curline].length - 1;
	    isspace(*p) && (p>file->line[curline].buff); --p)
	    ;
	if ( (*p == '{') && (*file->curchar != '{') )
	    indent += options->indent_size;
	len = indent + strlen(file->curchar);
    }

    /* Allocate the new line */
    ALLOC_LINE(file,curline+1,len);
    
    /* Align new line under first graphic char on current line */
    for (newptr=file->line[curline+1].buff; indent>0; ++newptr, --indent)
	*newptr = ' ';

    /* Don't copy leading whitespace from current line */
    for (oldptr = file->curchar; isspace(*oldptr); ++oldptr)
	;

    /* Copy part of current line to new line */
    strlbasecpy(newptr, file->line[curline + 1].buff, oldptr, LINE_BUFF_SIZE(len));
    *(file->curchar) = '\0';    /* Mark new end of old line */
    file->curcol = file->curchar - file->line[curline].buff;
    file->line[curline].length = file->curcol;
    file->line[curline + 1].length = strlen(file->line[curline + 1].buff);
    return 0;
}


void    cut_line(file,options,cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    char    *ok_button[2] = OK_BUTTON;
    
    if ( area_started(cut_buff) )
    {
	popup_mesg("You must end area before cutting.", ok_button, options);
	return;
    }

    /* Update file buffer */
    move_to(file, options, cut_buff, file->curline, 0);
    begin_area(file, cut_buff);
    if ( file->curline == file->total_lines - 1)
	move_to(file, options, cut_buff, file->curline,
	    file->line[file->curline].length);
    else
	move_to(file, options, cut_buff, file->curline + 1, 0);
    end_area(file, options, cut_buff);
    cut_area(file, options, cut_buff);
}


void    del_word(file, options, cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;

{
    begin_area(file, cut_buff);
    while (isspace(*file->curchar))
	move_right(file, options, cut_buff);
    if (isdigit(*file->curchar))
	while (isdigit(*file->curchar))
	    move_right(file,options,cut_buff);
    else if (isalpha(*file->curchar) || (*file->curchar == '_'))
	while (ISIDENT(*file->curchar))
	    move_right(file,options,cut_buff);
    else
	move_right(file,options,cut_buff);

    while (isspace(*file->curchar))
	move_right(file,options,cut_buff);
    end_area(file, options, cut_buff);
    cut_area(file, options, cut_buff);
}


void    cut_to_end(file_t *file, opt_t *options, buff_t *cut_buff)

{
    size_t  left_col = file->leftcol;
    
    begin_area(file, cut_buff);
    move_to(file, options, cut_buff, file->curline,
	file->line[file->curline].length);
    end_area(file, options, cut_buff);
    file->leftcol = left_col;
    cut_area(file, options, cut_buff);
}


void    indent_area(file,options,cut_buff,cols)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;
int     cols;

{
    long    line, start_line, end_line;
    buff_len_t   newlen, len;

    /* Determine lines to be indented */
    start_line = cut_buff->start_line;
    end_line = cut_buff->end_line;
    if ( start_line == end_line )
	return;
    if ( cut_buff->end_col == 0 )
    {
	--end_line;
	sprintw(2,TWC_ST_LEN,"Set end_line to %d",end_line);
    }
    if ( cut_buff->start_col == file->line[cut_buff->start_line].length )
	++start_line;
    
    /* Make sure longest line can be tabbed */
    for (line=start_line; line<=end_line; ++line)
    {
	if ( file->line[line].length >= file->max_line_len-cols )
	    return;
    }
    
    /* Tab all lines in buffer */
    for (line=start_line; line<=end_line; ++line)
    {
	len = file->line[line].length;
	newlen = len + cols;
	expand_buff_if_needed(file,line,newlen);
	memmove(file->line[line].buff+cols,file->line[line].buff,len+1);
	memset(file->line[line].buff,' ',cols);
	file->line[line].length += cols;
    }
    
    /* Update screen */
    update_win(file,options,cut_buff);
    file->modified = 1;
}


void    unindent_area(file,options,cut_buff,cols)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;
int     cols;

{
    int     line, start_line, end_line, len, actual_cols;
    char    *p, *done;
    
    /* Determine lines to be unindented */
    start_line = cut_buff->start_line;
    end_line = cut_buff->end_line;
    if ( start_line == end_line )
	return;
    if ( cut_buff->end_col == 0 )
	--end_line;
    if ( cut_buff->start_col == file->line[cut_buff->start_line].length )
	++start_line;
	
    /* Make sure all lines can be untabbed */
    for (line=start_line; line<=end_line; ++line)
    {
	/* Line must begin with a space char */
	p=file->line[line].buff;
	if ( (*p != ' ') && (*p != '\0') )
	    return;
	
	/* Reduce cols to minimum leading whitespace from all lines */
	done = p + cols;
	while ( (p < done) && (*p == ' ') )
	    ++p;
	if ( (p - file->line[line].buff < cols) && (*p != '\0') )
	    cols = p - file->line[line].buff;
    }
    
    /* Untab all lines in buffer */
    for (line=start_line; line<=end_line; ++line)
    {
	if ( *file->line[line].buff != '\0' )
	{
	    /* How much to move? */
	    if ( file->line[line].length < cols )
	    {
		/* Line is blank with < cols spaces.  Just empty it. */
		actual_cols = file->line[line].length;
		len = file->line[line].length + 1;
	    }
	    else
	    {
		/* Line has >= cols chars.  Shift them all. */
		actual_cols = cols;
		len = file->line[line].length + cols + 1;
	    }
	    memmove(file->line[line].buff,
		file->line[line].buff + actual_cols, len);
	    file->line[line].length -= actual_cols;
	}
    }
    
    /* Check cursor position */
    if ( ACTUAL_COL(file) > (unsigned)file->line[file->curline].length )
	file->curchar = file->line[file->curline].buff +
			file->line[file->curline].length;
    
    /* Update screen */
    update_win(file,options,cut_buff);
    file->modified = 1;
}


void    auto_wrap(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    char    *p, *start_line, *end_line;
    size_t  curline = file->curline, count;
    
    /* Find last space in line */
    start_line = file->line[curline].buff;
    end_line = start_line+TW_COLS(file->text)-4;
    for (p=end_line-1; !isspace(*p) && (p>start_line); --p)
	;
    
    /* If space found, break line */
    if ( p != start_line )
    {
	/* Count how many characters are being moved */
	count = end_line - p - 1;  /* How far back to the space */
	file->curchar = p;
	file->curcol = p - start_line;
	
	/* Split line */
	new_line(file, options,cut_buff,'\r');

	/* Move to proper position in new line */
	file->curcol += count;
	file->curchar += count;
	update_cursor(file,options,cut_buff);
    } 
}


void    join_lines(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     col = ACTUAL_COL(file);
    
    end_line(file,options,cut_buff);
    del_under(file,options,cut_buff);
    move_to(file,options,cut_buff,file->curline,col);
}


int     get_next_word(file,line,pp,word)
file_t  *file;
size_t  *line;
char    **pp, **word;

{
    int     len;
    char    *p = *pp;
    size_t  l = *line;
    
    /* Skip leading whitespace */
    while ( isspace(*p) && (*p != '\0') )
	++p;
    
    /* Get word */
    *word = p;
    for (len=0; !isspace(*p) && (*p != '\0'); ++p, ++len)
	;
    if ( *p == '\0' )
	p = file->line[++l].buff;
    else
	*p = '\0';
    
    *pp = p;
    *line = l;
    return len;
}


/****************************************************************************
 * Title:
 * Author:
 * Created:
 * Modifications:
 * Arguments:
 * Return values:   0 for success
 * Description:
 *      Reformat a paragraph to equalize the line lengths.
 *      Must be done blind, since paragraph may extend beyond the bottom
 *      of the screen.
 ****************************************************************************/

/* FIXME: This feature is badly broken. */

int     format_paragraph(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    int     indent, right_margin, c, needspace;
    size_t  curline;

    /* Use indent of first line in paragraph */
    for (indent=0; isspace(file->line[file->curline].buff[indent]); ++indent)
	;
    right_margin = MIN(indent,4);
    
    /* Combine and trim lines to end of paragraph */
    while ( !strblank(file->line[file->curline].buff) &&
	    (file->curline < file->total_lines - 1) )
    {
	curline = file->curline;
	/* Fill up the current line */
	while ( (file->line[curline].length < TW_COLS(file->text) - right_margin) &&
		(curline < file->total_lines-1) &&
		!strblank(file->line[curline+1].buff) )
	{
	    end_line(file,options,cut_buff);
	    /* Add a space between the lines being joined */
	    /* if there isn't already one there (for documents) */
	    needspace = (file->curchar != file->line[file->curline].buff) &&
			!isspace(file->curchar[-1]);
	    if ( needspace )
		insert_char(file,' ',options,cut_buff);
	    combine_lines(file,options,cut_buff);
	}
	
	/* Find a place to break the line */
	if ( file->line[curline].length > TW_COLS(file->text) - right_margin )
	{
	    for (c=TW_COLS(file->text) - right_margin; 
		!isspace(file->line[curline].buff[c]); --c)
		;
	    file->curcol = c;
	    file->curchar = file->line[curline].buff + c;
	    new_line(file,options,cut_buff,'\r');
	}
	else
	    break;
    }
    return 0;
}


int     tab_stops(file_t *file)

{
    return DEFAULT_TAB_STOPS;
}
