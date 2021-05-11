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
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"


extern term_t   *Terminal;

int     edit_menu (files, aw, cut_buff, options, event)
file_t  files[];
int    *aw;
buff_t *cut_buff;
opt_t  *options;
event_t *event;

{
    extern term_t   *Terminal;
    int     ch, start_row = 1;
    win_t  *edit_pop;
    static char *edit_text[] =
    {
	".Begin area (Ctrl+c or F4)",
	"Begin area at column .1 (Esc-1)",
	".Cut area (Ctrl+x)",
	"C.Opy area (Ctrl+c or F4)",
	".Paste area (Ctrl+v)",
	"C.Ancel area (Ctrl+c or F4)",
	TWC_HLINE,
	".New macro",
	/* "New macro .Submenu", */
	/*".Modify macro",*/
	".Replace macro",
	"Remo.ve macro",
	".Edit macro text",
	".Invoke macro (F9 or Esc-Enter)",
	TWC_HLINE,
	// ".Undo (Ctrl+z)",
	".Goto line (Ctrl+g)",
	//".Format paragraph (F11)",
	""};

    edit_pop = tw_menu (Terminal, 1, 6, edit_text, &options->border,
		      options->no_acs, MONO_MODE(options),
		      options->menu_fg, options->menu_bg,
		      options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr (edit_pop, REVERSE_MODE, options->menu_fg, options->menu_bg,
		   BOLD_MODE, options->menu_hl_fg, options->menu_hl_bg);
    ch = tw_get_item (edit_pop, edit_text, event, &start_row,
		    options->reverse_menu,NULL);
    switch (ch)
    {
    case '1':
	begin_line (files + *aw, options, cut_buff);
	/* No break */
    case 'b':                   /* Begin area */
	begin_area (files + *aw, cut_buff);
	break;
    case 'o':                   /* End area */
	end_area (files + *aw, options, cut_buff);
	break;
    case 'c':                   /* Delete marked area */
	cut_area (files + *aw, options, cut_buff);
	break;
    case 'p':                   /* Paste yanked or deleted text */
	paste_area (files + *aw, options, cut_buff, 0);
	break;
    case 'a':                   /* Cancel area */
	cancel_area (files+*aw,cut_buff, options);
	break;
    case 'u':
	// Disabled until undo feature is finished
	// file_undo (files + *aw, options, cut_buff);
	break;
    case 'g':                   /* Goto line */
	gotoline (files + *aw, options, cut_buff);
	break;
    case 'n':
	macro_new_item (files, aw, options, cut_buff, "");
	break;
    case 's':
	macro_new_submenu (files, aw, options, cut_buff, "");
	break;
    case 'r':
	macro_replace (files, aw, options, cut_buff, event);
	break;
    case 'v':
	macro_remove (files + *aw, options, event, NULL);
	break;
    case 'e':
	macro_edit(files, aw, options, cut_buff, event);
	break;
    case 'i':
	ch = macro_menu (files, aw, options, event, MACRO_EXPAND);
	break;
#if 0
    case 'f':
	format_paragraph (files + *aw, options, cut_buff);
	break;
#endif
    default:
	break;
    }
    tw_del_win (&edit_pop);
    return (ch);
}


/***************************************************************
 * Mark beginning of an area for later cut and paste operations
 ***************************************************************/

void    begin_area (file, cut_buff)
file_t *file;
buff_t *cut_buff;

{
    cut_buff->old_line = cut_buff->start_line = file->curline;
    cut_buff->old_col = cut_buff->start_col = file->curchar - file->line[file->curline].buff;
    cut_buff->start_top = file->topline;
    cut_buff->deleted = 0;
    cut_buff->file = file;
    stat_mesg ("Ctrl+a or F4 to mark end point");
}


/**********************************************************************
 * Mark end of area and copy area marked into the cut buffer for later
 * pasting.
 **********************************************************************/

void    end_area (file, options, cut_buff)
file_t *file;
buff_t *cut_buff;
opt_t   *options;

{
    char    *ok_button[2] = OK_BUTTON;
    
    /* Check if area has been started */
    if (!area_started (cut_buff))
    {
	popup_mesg ("You must begin an area first.",ok_button,options);
	TW_RESTORE_WIN(file->window);
	return;
    }

    if (file != cut_buff->file)
    {
	sprintw (2, TWC_ST_LEN, "Error: Area was started in %s.", cut_buff->file->source);
	return;
    }

    /* Mark end area */
    cut_buff->end_line = file->curline;
    cut_buff->end_col = file->curchar - file->line[file->curline].buff;
    cut_buff->end_top = file->topline;

    /* Swap the current position with the starting position if it is earlier */
    arrange_ends (cut_buff);

    /* Copy area into cut buffer */
    pull_area (file, cut_buff);
}


/***********************************************************************
 * Copy the marked area into a simple text buffer, denoting end of line
 * with the '\n' character.
 ***********************************************************************/

void    pull_area (file, cut_buff)
file_t *file;
buff_t *cut_buff;

{
    char   *cur_ptr, *end_ptr;
    int     line;

    /* Copy the marked area into the buffer */
    if (cut_buff->fp != NULL)
    {
	fclose (cut_buff->fp);
	cut_buff->fp = NULL;
    }

    if ((cut_buff->fp = tmpfile ()) == NULL)
    {
	sprintw (2, TWC_ST_LEN, "Could not open buffer.");
	return;
    }

    line = cut_buff->start_line;
    cur_ptr = file->line[line].buff + cut_buff->start_col;
    end_ptr = file->line[cut_buff->end_line].buff + cut_buff->end_col;
    cut_buff->marked_lines = 0;
    cut_buff->start_line_len = file->line[line].length - cut_buff->start_col;

    /* Pull text from marked area into the cut buffer */
    while (cur_ptr != end_ptr)
    {
	if (*cur_ptr == '\0')
	{
	    putc ('\n', cut_buff->fp);
	    cur_ptr = file->line[++line].buff;
	    ++cut_buff->marked_lines;
	}
	else
	    putc (*cur_ptr++, cut_buff->fp);
    }
    rewind (cut_buff->fp);
    stat_mesg ("Del: Ctrl+d  Paste: Esc-p  Cancel: Ctrl+a or F4");
}


/***********************************************************************
 * Ensure that the starting point of a marked area is before the ending
 * area so that cut and paste operations have a consistant structure
 * to deal with.
 ***********************************************************************/

void    arrange_ends (cut_buff)
buff_t *cut_buff;

{
    int     swap_needed, temp;

    /* Determine if start point is later in file than end */
    swap_needed = (cut_buff->start_line > cut_buff->end_line) ||
	((cut_buff->start_line == cut_buff->end_line) &&
	 (cut_buff->start_col > cut_buff->end_col));

    /* Swap start and end markers if necessary */
    if (swap_needed)
    {
	temp = cut_buff->end_line;
	cut_buff->end_line = cut_buff->start_line;
	cut_buff->start_line = temp;

	temp = cut_buff->end_col;
	cut_buff->end_col = cut_buff->start_col;
	cut_buff->start_col = temp;

	temp = cut_buff->end_top;
	cut_buff->end_top = cut_buff->start_top;
	cut_buff->start_top = temp;
    }
}


/*************************************************************************
 * Delete all text in the marked area.  Text will remain backed up in the
 * cut buffer.
 *************************************************************************/

void    cut_area (file, options, cut_buff)
file_t *file;
opt_t  *options;
buff_t *cut_buff;

{
    char   *start_point, *end_point, *ok_button[2] = OK_BUTTON;
    int     start_line, end_line;
    buff_len_t   len;

    if (!area_started (cut_buff))
    {
	popup_mesg ("Cannot delete: no area marked.",ok_button,options);
	TW_RESTORE_WIN(file->window);
	return;
    }
    if (cut_buff->deleted)
    {
	popup_mesg ("Area already deleted.", ok_button, options);
	TW_RESTORE_WIN(file->window);
	return;
    }
    if (!area_ended (cut_buff))
	end_area(file, options, cut_buff);

    /* Initialize these after end_area() is called! */
    start_line = cut_buff->start_line, end_line = cut_buff->end_line;
    
    start_point = file->line[start_line].buff + cut_buff->start_col;
    end_point = file->line[end_line].buff + cut_buff->end_col;
    len = cut_buff->start_col + (file->line[end_line].length - cut_buff->end_col);

    /* Check line length */
    if (len > file->max_line_len)
    {
	popup_mesg ("Cannot delete - merged lines are too long.",
		ok_button, options);
	TW_RESTORE_WIN(file->window);
	return;
    }

    /* Extend line buffer if needed */
    expand_buff_if_needed (file, start_line, len);
    start_point = file->line[start_line].buff + cut_buff->start_col;

    /* Concatenate portions of first and last lines */
    strlbasecpy (start_point, end_point, file->line[start_line].buff, LINE_BUFF_SIZE (len));
    file->line[start_line].length = strlen (file->line[start_line].buff);

    /* Delete lines between */
    delete_lines (file, start_line + 1, end_line);
    cut_buff->deleted = 1;

    /* Relocate cursor at the seam */
    file->curline = start_line;
    file->curcol = cut_buff->start_col;
    file->topline = cut_buff->start_top;
    file->curchar = start_point;
    SET_MODIFIED (file);
    unmark_area (cut_buff);
    update_win (file, options, cut_buff);
    stat_mesg ("Area deleted.");
}


void    delete_lines (file, first, last)
file_t *file;
int     first, last;

{
    int     lines = last - first + 1, l;

    /* Free memory from deleted lines */
    for (l = first; l <= last; ++l)
	free (file->line[l].buff);

    /* Shift up lines below deleted portion */
    for (l = last + 1; l <= file->total_lines; ++l)
	file->line[l - lines] = file->line[l];
    file->total_lines -= lines;
}


/************************************************************************
 * Restore the edit buffer to it's original state after creating an
 * opening for inserting lines.  Useful when a paste cannot be completed
 * due to memory limits.
 ************************************************************************/

void    restore_buff (file, new_lines)
file_t *file;
int     new_lines;

{
    stat_mesg ("Cannot paste: Out of memory.");

    /* Free allocated lines */
    delete_lines (file, file->curline, file->curline + new_lines);
}


/******************************************************************
 * Create room for inserting a paste block.  Shift lines below the
 * insert area down, and allocate memory for new lines.
 ******************************************************************/

int     make_room (file, start_line, new_lines)
file_t *file;
int     start_line, new_lines;

{
    int     c;

    /* Extend line pointer array if needed */
    while (new_lines + file->total_lines >= file->max_lines-2)
    {
	if (more_lines (file) == NOMEM)
	{
	    stat_mesg ("Cannot insert new line: Out of memory.");
	    return (NOMEM);
	}
    }

    /* Move lines down to make room for new lines */
    for (c = file->total_lines; c >= start_line; --c)
	file->line[c + new_lines] = file->line[c];
    file->total_lines += new_lines;
    return (0);
}


/**************************************************************
 * Split the current line and place the latter part at the end
 * of the new paste block.  Does not deal with indents, etc.
 * and is not suitable for calling from new_line().
 **************************************************************/

void    split_curline (file, new_lines)
file_t *file;
int     new_lines;

{
    int     len, lastline = file->curline + new_lines;

    len = strlen (file->curchar);
    ALLOC_LINE(file,lastline,len);
    if (file->line[lastline].buff == NULL)
    {
	stat_mesg ("split_curline(): Out of memory.");
	return;
    }
    strlcpy (file->line[lastline].buff, file->curchar, LINE_BUFF_SIZE (len));

    /* Mark end of first line at split point */
    file->line[file->curline].length -= len;
    *file->curchar = '\0';
}


/*******************************************************************
 * Insert lines into the new paste block area, which was previously
 * allocated.
 *******************************************************************/

void    insert_lines (file, cut_buff, indent)
file_t *file;
buff_t *cut_buff;
int     indent;

{
    FILE   *fp = cut_buff->fp;
    size_t  line = file->curline + 1,
	    len = indent,
	    line_count = 1;
    int     ch;
    char    temp[MAX_LINE_LEN + 1];

    //sprintw(2, 50, "Indent = %d", indent);
    //tgetc(Terminal);
    
    /* If new line, blank it up to indent */
    memset (temp, ' ', indent);
    
    /* Go to start of second line */
    rewind (fp);
    while (!feof (fp) && (ch = getc (fp) != '\n'))
	;

    /* Insert all complete lines */
    while ( (line_count < cut_buff->marked_lines) &&
	((ch = getc(fp)) != EOF) )
    {
	//sprintw(2, 50, "%ld %lx", line_count, cut_buff->marked_lines);
	//tgetc(Terminal);
	if (ch == '\n') 
	{
	    /* End current line */
	    temp[len] = '\0';
	    ALLOC_LINE(file,line,len);
	    strlcpy (file->line[line].buff, temp, len+1);

	    /* Start new line */
	    ++line;
	    ++line_count;
	    len = indent;       /* Indent if necessary */
	}
	else    /* This clause executes first in most cases */
	    temp[len++] = ch;
    }
    
    //sprintw(2, 50, "temp = %s", temp);
    //tgetc(Terminal);
    
    /* Insert partial last line if present */
    //sprintw(2, 50, "Inserting partial last line.");
    insert_partial_line (file, cut_buff, line, 0, indent);

    /* Indent bottom line if necessary */
}


/*
 * Insert part of a line from cut_buff
 */

void    insert_partial_line (file, cut_buff, line, col, indent)
file_t *file;
buff_t *cut_buff;
int     line, col, indent;

{
    int     c, ch;
    int     newlen;
    FILE   *fp = cut_buff->fp;
    char    temp[MAX_LINE_LEN + 1];

    memset(temp, ' ', indent);
    
    /* Get partial line from cut buffer */
    for (c = indent; (c < MAX_LINE_LEN) && (ch = getc (fp)) != EOF; ++c)
	temp[c] = ch;

    /* Extend line buffer if needed */
    newlen = c + file->line[line].length;
    expand_buff_if_needed (file, line, newlen);

    /* Insert text */
    memmove (file->line[line].buff + col + c, file->line[line].buff + col,
	     file->line[line].length + 1 - col);
    memcpy (file->line[line].buff + col, temp, c);
    file->line[line].length = newlen;
    rewind (fp);
}


/*******************************************************************
 * Copy text from the cut buffer back into the file buffer starting
 * at the current cursor position.
 * The current implementation is very inefficient.  It should and
 * will be converted to a single-shift and direct insert method.
 *******************************************************************/

void    paste_area (file, options, cut_buff, auto_indent)
file_t *file;
opt_t  *options;
buff_t *cut_buff;
int     auto_indent;

{
    int     len1, len2, indent = 0;
    char    *ok_button[2] = OK_BUTTON;
    size_t  first,last;

    /* Make sure there is an area to paste */
    if (cut_buff->fp == NULL)
    {
	popup_mesg ("Cannot paste: No area marked.", ok_button, options);
	TW_RESTORE_WIN(file->window);
	return;
    }

    if (cut_buff->marked_lines > 0)
    {
	/* See if split lines will fit into max_line_len */
	len1 = cut_buff->start_line_len + file->curcol;
	len2 = cut_buff->end_col +
		(file->line[file->curline].length - ACTUAL_COL(file));
	if ((len1 >= file->max_line_len) || (len2 >= file->max_line_len))
	{
	    sprintw(2,TWC_ST_LEN,"%d+%d %d+%d",cut_buff->start_line_len,
		    file->curcol,cut_buff->end_col,
		    (file->line[file->curline].length - file->curcol));
	    popup_mesg ("Cannot paste: Combined lines too long.",
		    ok_button, options);
	    TW_RESTORE_WIN(file->window);
	    return;
	}

	/* Shift lines down to create opening for lines in buffer */
	if (make_room (file, file->curline + 1, cut_buff->marked_lines) == NOMEM)
	    return;

	/* Split the current line, placing latter part at end of paste */
	split_curline (file, cut_buff->marked_lines);

	/*
	 *  Indentation is the maximum of the current column and the indent level
	 *  of the previous non-blank line.
	 */
	if (auto_indent && (file->curline != 0))
	    indent = file->curchar - file->line[file->curline].buff;

	/* Insert cut lines starting with 2nd */
	insert_lines (file, cut_buff, indent);

	/* Append first cut line into current */
	append_partial_first_line (file, cut_buff, indent);
    }
    else    /* Only part of 1 line */
    {
	len1 = cut_buff->end_col - cut_buff->start_col +
		file->line[file->curline].length;
	if (len1 >= file->max_line_len)
	{
	    //sprintw(2,TWC_ST_LEN,"%d",len1);
	    popup_mesg ("Cannot paste: Combined lines too long.",
		    ok_button, options);
	    TW_RESTORE_WIN(file->window);
	    return;
	}

	/* Insert partial last line if present */
	file->curcol = file->curchar - file->line[file->curline].buff;
	insert_partial_line (file, cut_buff, file->curline, file->curcol, 0);
    }

    SET_MODIFIED (file);
    synhigh_tag_which_lines(file,file->curline,file->curline+cut_buff->marked_lines-1,
		    &first,&last);
    synhigh_tag_lines(file, first,last);
    if (cut_buff->file != NULL) /* Macro? */
	unmark_area (cut_buff);
    update_win (file, options, cut_buff);
}


void    append_partial_first_line (file, cut_buff, indent)
file_t *file;
buff_t *cut_buff;
int     indent;

{
    int     curline = file->curline, ch, c;
    buff_len_t   len, col;
    char    temp[MAX_LINE_LEN + 1];

    /* Get text to append */
    rewind (cut_buff->fp);
    for (c = 0; ((ch = getc (cut_buff->fp)) != '\n') && (ch != EOF); ++c)
	temp[c] = ch;
    temp[c] = '\0';

    /* Extend line buffer if needed */
    col = file->curchar - file->line[curline].buff;

    /* Append */
    if (col == 0)
    {
	len = col + c + indent;
	expand_buff_if_needed (file, curline, len);
	memset (file->curchar, ' ', indent);
	strlcpy (file->curchar + indent, temp, LINE_BUFF_SIZE (len));
    }
    else
    {
	len = col + c;
	expand_buff_if_needed (file, curline, len);
	strlbasecpy (file->curchar, temp, file->line[curline].buff, LINE_BUFF_SIZE (len));
    }
    file->curcol = col;
    file->line[curline].length = len;
}


void    unmark_area (cut_buff)
buff_t *cut_buff;

{
    cut_buff->start_line = cut_buff->end_line = -1;
    cut_buff->old_line = cut_buff->old_col = -1;
    sprintw (2, TWC_ST_LEN, "Area unmarked.");
}


/********************************
 * Re-initialize the cut-buffer.
 ********************************/

void    cancel_area (file, cut_buff, options)
file_t  *file;
buff_t *cut_buff;
opt_t  *options;
{
    if (cut_buff->fp != NULL)
    {
	fclose (cut_buff->fp);
	init_cut_buff (cut_buff);
	stat_mesg ("Area cancelled.");
	/* Slow, but does the job for now */
	/* If marked file is on screen, update */
	if ( cut_buff->file == file )
	    update_win (cut_buff->file, options, cut_buff);
    }
}


/*******************************************
 * Return true if an area has been started.
 *******************************************/

int     area_started (cut_buff)
buff_t *cut_buff;

{
    return ((long)cut_buff->start_line != -1);
}


/******************************************************
 * Return true if an area has been marked off (ended).
 ******************************************************/

int     area_ended (cut_buff)
buff_t *cut_buff;

{
    return ((long)cut_buff->end_line != -1);
}


/***************************
 * Initialize a cut-buffer.
 ***************************/

void    init_cut_buff (cut_buff)
buff_t *cut_buff;

{
    cut_buff->start_line = cut_buff->end_line = -1;
    cut_buff->start_col = cut_buff->end_col = -1;
    cut_buff->deleted = 0;
    cut_buff->old_line = cut_buff->old_col = -1;
    cut_buff->fp = NULL;
}


/*
 * Determine if a position is further from the starting point of a
 * cut buffer.  Return > 0 if further, 0 if the same, < 0 if closer.
 */

int     further (cut_buff, new_line, new_col)
buff_t *cut_buff;
int     new_line, new_col;

{
    int     dol = ABS ((int) (cut_buff->old_line - cut_buff->start_line)),
	    dnl = ABS ((int) (new_line - cut_buff->start_line)), doc = ABS ((int) (cut_buff->old_col - cut_buff->start_col)),
	    dnc = ABS ((int) (new_col - cut_buff->start_col));

    if (dnl != dol)
	return dnl - dol;
    else if (new_line == cut_buff->start_line)
	return dnc - doc;
    else if (new_line > cut_buff->start_line)
	return new_col - cut_buff->old_col;
    else
	return cut_buff->old_col - new_col;
}


/*
 * Highlight or unhighlight a portion of the marked area following
 * cursor movement.
 */

void    adjust_highlight (file, cut_buff, options)
file_t *file;
buff_t *cut_buff;
opt_t  *options;

{
    int     stat;

    /* New area just started? */
    if ((long)cut_buff->old_line == -1)
    {
	cut_buff->old_line = file->curline;
	cut_buff->old_col = ACTUAL_COL (file);
    }

    /* If moving further from starting point, highlight */
    stat = further (cut_buff, file->curline, ACTUAL_COL (file));
    if (stat > 0)
    {
	/*sprintw(2,50,"Highlighting %d,%d to %d,%d",
	   cut_buff->old_line,cut_buff->old_col,
	   file->curline,ACTUAL_COL(file)); */
	set_highlight (file, options);
	rewrite (file, options, cut_buff, cut_buff->old_line, cut_buff->old_col,
		 file->curline, ACTUAL_COL (file));
	unset_highlight (file, options);
    }
    else if (stat < 0)
    {
	/*sprintw(2,50,"Unhighlighting %d,%d to %d,%d",
	   cut_buff->old_line,cut_buff->old_col,
	   file->curline,ACTUAL_COL(file)); */
	unset_highlight (file, options);
	rewrite (file, options, cut_buff, cut_buff->old_line, cut_buff->old_col,
		 file->curline, ACTUAL_COL (file));
    }

    /* Record last position for next time */
    cut_buff->old_line = file->curline;
    cut_buff->old_col = ACTUAL_COL (file);
}


void    set_highlight (file, options)
file_t *file;
opt_t  *options;

{
    if (TCOLOR_TERM (file->text->terminal) && !MONO_MODE(options))
    {
	tw_set_foreground (file->text, options->text_bg);
	tw_set_background (file->text, options->text_fg);
    }
    else
	TW_SET_MODES (file->text, HIGHLIGHT_MODE);
}


void    unset_highlight (file, options)
file_t *file;
opt_t  *options;

{
    if (TCOLOR_TERM (file->text->terminal) && !MONO_MODE(options))
    {
	tw_set_foreground (file->text, options->text_fg);
	tw_set_background (file->text, options->text_bg);
    }
    else
	TW_SET_MODES (file->text, NORMAL_MODE);
}


/*
 * Can probably eliminate line1 & col1 since they come from cut_buff
 */

void    rewrite (file, options, cut_buff, line1, col1, line2, col2)
file_t *file;
opt_t  *options;
buff_t *cut_buff;
size_t  line1, line2;
buff_len_t   col1, col2;

{
    int     l, ch;
    buff_len_t   right_margin, c;

    /* Rearrange line,col tuples if necessary - modify arrange_ends */
    rearrange (&line1, &col1, &line2, &col2);

    /* Adjust in case of page left or right */
    if (col1 < file->leftcol)
	col1 = file->leftcol;
    if (col2 >= file->leftcol + TW_COLS (file->text))
	col2 = file->leftcol + TW_COLS (file->text) - 1;

    right_margin = file->leftcol + TW_COLS (file->text);

    if (line1 == line2)
    {
	if ((line1 < file->topline) ||
	    (line1 >= file->topline + TW_LINES (file->text)))
	    return;
	if (col1 < right_margin)
	{
	    tw_move_to (file->text, line1 - file->topline, col1 - file->leftcol);
	    for (c = col1; (c < col2) && ((ch = file->line[line2].buff[c]) != '\0'); ++c)
		tw_putc (file->text, ch);
	}
    }
    else
    {
	/* Adjust in case part of highlighting region is off screen */
	if (line1 < file->topline)      /* Pageup occured */
	{
	    line1 = file->topline;
	    col1 = 0;
	}
	if (line2 >= file->topline + TW_LINES (file->text))
	{
	    line2 = file->topline + TW_LINES (file->text) - 1;
	    col2 = file->line[line2].length;
	}

	/* Redraw first line */
	if (col1 < right_margin)
	{
	    tw_move_to (file->text, line1 - file->topline, col1 - file->leftcol);
	    for (c = col1; ((ch = file->line[line1].buff[c]) != '\0') && (c < right_margin); ++c)
		tw_putc (file->text, ch);
	}

	/* Redraw whole lines */
	for (l = line1 + 1; l < line2; ++l)
	{
	    tw_move_to (file->text, l - file->topline, 0);
	    for (c = file->leftcol; ((ch = file->line[l].buff[c]) != '\0') && (c < right_margin); ++c)
		tw_putc (file->text, ch);
	}

	/* Redraw last line */
	tw_move_to (file->text, line2 - file->topline, 0);
	for (c = file->leftcol; (c < col2) && ((ch = file->line[line2].buff[c]) != '\0'); ++c)
	    tw_putc (file->text, ch);
    }
    update_cursor (file, options, cut_buff);
}


/*
 * Rearrange line and col pairs so line1,col1 is the earlier position
 */

void    rearrange (line1, col1, line2, col2)
size_t *line1, *line2;
buff_len_t  *col1, *col2;

{
    int     temp;

    if ((*line1 > *line2) || ((*line1 == *line2) && (*col1 > *col2)))
    {
	temp = *line1;
	*line1 = *line2;
	*line2 = temp;
	temp = *col1;
	*col1 = *col2;
	*col2 = temp;
    }
}


/*
 * Turn highlighting on or off when entering or leaving a marked area.
 */

int     get_area (file, cut_buff, line1, col1, line2, col2)
file_t *file;
buff_t *cut_buff;
size_t *line1, *line2;
buff_len_t  *col1, *col2;

{
    /* See if an area is started */
    if (!area_started (cut_buff))
	return 0;

    /* See if we're in the right file for starters */
    if (file != cut_buff->file)
	return 0;

    /* Get starting and ending points of area */
    *line1 = cut_buff->start_line;
    *col1 = cut_buff->start_col;

    if (area_ended (cut_buff))
    {
	*line2 = cut_buff->end_line;
	*col2 = cut_buff->end_col;
    }
    else
    {
	*line2 = file->curline;
	*col2 = ACTUAL_COL (file);
    }
    rearrange (line1, col1, line2, col2);
    return 1;
}


/*
 * Determine if line,col is within the marked area delimited by
 * line1,col1 and line2,col2
 */

int     in_area (file, cut_buff, line, col, line1, col1, line2, col2)
file_t *file;
buff_t *cut_buff;
size_t  line, *line1, *line2;
buff_len_t   col, *col1, *col2;

{
    int     status = 0;

    /* Check buffer for marked area */
    if ((cut_buff != NULL) && area_ended (cut_buff))
    {
	/* Get starting and ending points of marked area */
	if (get_area (file, cut_buff, line1, col1, line2, col2))
	{
	    /* Check if starting point is within marked area */
	    if ( *line1 == *line2 )
		status = (col >= *col1) && (col < *col2);
	    else
		status = ((line > *line1) && (line < *line2)) ||
			((line == *line1) && (col >= *col1)) ||
			((line == *line2) && (col <= *col2));
	}
    }
    return status;
}


int     in_marked_area(file,cut_buff,line,col)
file_t  *file;
buff_t  *cut_buff;
size_t  line;
buff_len_t   col;

{
    size_t  l1, l2;
    buff_len_t   c1, c2;
    
    return in_area(file,cut_buff,line,col,&l1,&c1,&l2,&c2);
}


int     cursor_in_area(file,cut_buff)
file_t  *file;
buff_t  *cut_buff;

{
    size_t  l1,l2;
    buff_len_t   c1,c2;
    
    return in_area(file,cut_buff,file->curline,ACTUAL_COL(file),
	    &l1,&c1,&l2,&c2);
}

