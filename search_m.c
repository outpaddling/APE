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
#include <xtend.h>
#include "edit.h"
#include "protos.h"

#define DIREC   8
#define CSENS   9


int     search_menu(file, string, options, cut_buff, event)
file_t *file;
char   *string;
opt_t  *options;
buff_t *cut_buff;
event_t *event;

{
    extern term_t   *Terminal;
    win_t  *search_pop;
    int     ch = 'f', start_row = 1;
    static char search_direction[] = ".Direction: Forward ", case_sensitive[] = ".Case sensitive: No ", *search_text[] = {
	".Find (Esc-/)",
	"R.Epeat last find (Ctrl+n or F3)",
	".Search and replace",
	".Resume search and replace (Esc-k)",
	TWC_HLINE,
	"Search .Header files",
	"Search .Libraries",
	TWC_HLINE,
	NULL,
	NULL,
    ""};

    if (search_text[DIREC] == NULL)
    {
	search_text[DIREC] = search_direction;
	search_text[CSENS] = case_sensitive;
    }
    search_pop = tw_menu(Terminal, 1, 12, search_text, &options->border,
		       options->no_acs, MONO_MODE(options),
		       options->menu_fg, options->menu_bg,
		       options->menu_hl_fg, options->menu_hl_bg);
    display_search_direction(search_pop, options);
    display_case_sens(search_pop, options);

    do
    {
	ch = tw_get_item(search_pop, search_text, event, &start_row,
		       options->reverse_menu,NULL);
	switch (ch)
	{
	case 'e':
	    if (*string != '\0')
	    {
		move_search(file, string, options, cut_buff, file->curline,
			    file->curchar - file->line[file->curline].buff,
			    TW_LINES(file->text));
		break;
	    }
	case 'f':
	    find_string(file, string, options, cut_buff);
	    break;
	case 's':
	    replace_string(file, string, options, cut_buff, FALSE);
	    break;
	case 'r':
	    replace_string(file, string, options, cut_buff, TRUE);
	    break;
	case 'd':
	    if ( event->type != BUTTON1_RELEASE )
	    {
		if ((options->search_forward = !options->search_forward))
		    strlcpy(search_direction + 12, "Forward ", 8);
		else
		    strlcpy(search_direction + 12, "Backward", 9);
		tw_highlight_menu_item(search_pop, search_text[DIREC], DIREC + 1,
				   TW_NO_COLOR(search_pop), options->reverse_menu,
				 TW_NORMAL_FG(search_pop), TW_NORMAL_BG(search_pop),
			   TW_HIGHLIGHT_FG(search_pop), TW_HIGHLIGHT_BG(search_pop),
				     TW_NORMAL_MODE(search_pop),
				     TW_HIGHLIGHT_MODE(search_pop),
				     ' ', ' ');
		display_search_direction(search_pop, options);
	    }
	    break;
	case 'c':
	    if ( event->type != BUTTON1_RELEASE )
	    {
		if ((options->case_sensitive = !options->case_sensitive))
		    strlcpy(case_sensitive + 17, "Yes", 3);
		else
		    strlcpy(case_sensitive + 17, "No ", 3);
		tw_highlight_menu_item(search_pop, search_text[DIREC],
			    DIREC + 1, TW_NO_COLOR(search_pop),
			    options->reverse_menu, TW_NORMAL_FG(search_pop),
			    TW_NORMAL_BG(search_pop),
			    TW_HIGHLIGHT_FG(search_pop),
			    TW_HIGHLIGHT_BG(search_pop),
			    TW_NORMAL_MODE(search_pop),
			    TW_HIGHLIGHT_MODE(search_pop),
			    ' ', ' ');
		display_case_sens(search_pop, options);
	    }
	    break;
	case 'h':
	    grep_headers(file, options);
	    break;
	case 'l':
	    search_libs(file, options);
	    break;
	default:
	    break;
	}
    } while (strchr("cd", ch) != NULL);
    /* Special case for search menu */
    if ((ch != KEY_RIGHT) && (ch != KEY_LEFT))
	stat_mesg(BAR_HELP);
    tw_del_win(&search_pop);
    TW_RESTORE_WIN(file->window);
    return (ch);
}


void    display_case_sens(search_pop, options)
win_t  *search_pop;
opt_t  *options;

{
    imove_to(&search_pop->image, CSENS + 1, 2);
    if (options->case_sensitive)
	tw_high_print(search_pop, options->menu_fg, options->menu_bg,
	  options->menu_hl_fg, options->menu_hl_bg, ".Case sensitive: Yes");
    else
	tw_high_print(search_pop, options->menu_fg, options->menu_bg,
	  options->menu_hl_fg, options->menu_hl_bg, ".Case sensitive: No ");
    tw_redraw_line(search_pop, CSENS + 1, 17, 21);
}


/*
 * Show search direction in the search_menu
 */

void    display_search_direction(search_pop, options)
win_t  *search_pop;
opt_t  *options;

{
    imove_to(&search_pop->image, DIREC + 1, 2);
    if (options->search_forward)
	tw_high_print(search_pop, options->menu_fg, options->menu_bg,
		    options->menu_hl_fg, options->menu_hl_bg,
		    ".Direction: Forward ");
    else
	tw_high_print(search_pop, options->menu_fg, options->menu_bg,
		    options->menu_hl_fg, options->menu_hl_bg,
		    ".Direction: Backward");
    tw_redraw_line(search_pop, DIREC + 1, 12, 21);
}


void    new_search_start(file_t *file)
{
    file->start_line = file->curline;
    file->start_col = ACTUAL_COL(file);
    file->search_wrapped = 0;
}


void    find_string(file_t *file, char *string, opt_t *options, buff_t *cut_buff)

{
    extern term_t   *Terminal;
    int             status;
    
    new_search_start(file);
    
    if (area_ended(cut_buff))
	set_search(string, cut_buff, SEARCH_STR_LEN);

    /* Get string to search for */
    status = panel_get_string(file, options, SEARCH_STR_LEN,
			    "Search for? ", "", TWC_VERBATIM, string);
    
    if (TW_EXIT_KEY(status) == TWC_INPUT_DONE)
    {
	move_search(file, string, options, cut_buff, file->curline,
		file->curchar - file->line[file->curline].buff,
		TW_LINES(file->text));
    }
}


/*
 * Search and prompt user to replace strings
 */

#define REPLACE_FIELDS  6
#define RESP_WIN_LINES  8


void    replace_string(file, string, options, cut_buff, resume_old_search)
file_t *file;
char   *string;
opt_t  *options;
buff_t *cut_buff;
int     resume_old_search;

{
    extern term_t   *Terminal;
    win_t  *win = NULL, *resp_win = NULL;
    tw_panel_t panel = TWC_PANEL_INIT;
    int     response, c, old_len, new_len, x, y, start_col, line, col,
	    save, status = 0, cap, choice, len, bytes;
    static char replace_str[4][SEARCH_STR_LEN + 1] = {"","","",""},
		cap_str[4], prompt[4][20];
    char    temp[SEARCH_STR_LEN + 1] = "", *yes_no[3] = YES_NO_ENUM,
	    *fl, *buttons[8] = {"[ 1 ]","[ 2 ]","[ 3 ]","[ 4 ]",
			    "[ Skip ]", "[ All ]", "[ Quit ]", NULL};
    static long start_line;
    static int  capitalize;
    
    if (!resume_old_search || (*string == '\0'))
    {
	if (area_ended(cut_buff))
	    set_search(string, cut_buff, SEARCH_STR_LEN);

	/* Set up input window */
	win = centered_panel_win(7+REPLACE_FIELDS, 65, options);
	tw_init_string(&panel, 2, 3, SEARCH_STR_LEN, 40, TWC_VERBATIM,
		    "Search for?     ","String to be replaced.", string);
	for (c=0; c<4; ++c)
	{
	    snprintf(prompt[c],19,"Replacement #%d? ",c+1);
	    tw_init_string(&panel, c+3, 3, SEARCH_STR_LEN, 40, TWC_VERBATIM,
		    prompt[c],"Replaces search string in selected instances.",
		    replace_str[c]);
	}
	tw_init_enum(&panel, 7, 3, 4, yes_no,
		    "Capitalize?     ",
		    " Capitalize replacements as needed?  Hit <space> to toggle. ",cap_str);
	strlcpy(cap_str,"Yes",4);
	
	/* Input search and replace strings */
	status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
	if (TW_EXIT_KEY(status) == TWC_INPUT_DONE)
	{
	    capitalize = (strcmp(cap_str,"Yes") == 0);
	    new_search_start(file);
	}
	tw_del_win(&win);
    }

    if (resume_old_search || TW_EXIT_KEY(status) == TWC_INPUT_DONE)
    {
	/* Initialize for search and replace */
	start_line = file->start_line;
	TW_RESTORE_WIN(file->window);
	response = ' ';         /* (y/n/q) */
	start_col = ACTUAL_COL(file);

	/* Search and replace */
	save = file->insert;
	file->insert = 1;
	while ((response != 'q') &&
	    (old_len = move_search(file, string, options,
	    cut_buff, start_line, start_col,
	    TW_LINES(file->text)-RESP_WIN_LINES)))
	{
	    TW_FIND_CURSOR(file->text, y, x);
	    if (response != 'a')
	    {
		/* Read string from text buffer */
		for (c = 0; c < old_len; ++c)
		    temp[c] = file->curchar[c];
		temp[c] = '\0';

		/* Reprint string in standout video */
		TW_FIND_CURSOR(file->text, line, col);
		set_highlight(file, options);
		rewrite(file, options, cut_buff,
		    file->curline, ACTUAL_COL(file),
		    file->curline, ACTUAL_COL(file) + strlen(temp));
		unset_highlight(file, options);
		show_cursor_pos(options, file);

		/* Have user select action */
		if ( resp_win == NULL )
		{
		    resp_win = panel_win(RESP_WIN_LINES, 66,
			TLINES(Terminal)-RESP_WIN_LINES-2, TWC_CENTER_WIN,
			options);
		    for (c=0; c<4; ++c)
			tw_print_at(resp_win,2,2+c*15,"%d) %-.10s",c+1,
				replace_str[c]);
		    tw_print_center(resp_win,4,"Replace with?");
		}
		TW_RESTORE_WIN(resp_win);
		response = tw_button_response(resp_win,6,buttons);
		if ( isupper(response) )
		    response = tolower(response);
	    }
	    tw_move_to(file->text, y, x);
	    if ( (strchr("1234",response) != NULL) || (response == 'a') )
	    {
		choice = isdigit(response) ? response - '1' : 0;
		new_len = strlen(replace_str[choice]);
		len = file->line[file->curline].length + new_len - old_len;
	    
		/* Expand buffer if necessary */
		expand_buff_if_needed(file,file->curline,len);
		
		/* Adjust everything right of the replaced text */
		bytes = file->line[file->curline].length -
			(ACTUAL_COL(file)+old_len);
		memmove(file->curchar+new_len,file->curchar+old_len,bytes);
		
		file->line[file->curline].length = len;
		/* Shouldn't be needed - strlen()+1 should do it,
		   but fails when replacing string and eoln */
		file->line[file->curline].buff[len] = '\0';
		
		/* If line has shrunk, start_col may no longer be valid */
		if ( file->curline == start_line )
		    start_col = MIN(start_col, len);

		/* Capitalize replace_string if necessary */
		for (fl=replace_str[choice];
		    (*fl != '\0') && !isalpha(*fl); ++fl)
		    ;
		cap = isupper(*file->curchar) && islower(file->curchar[1]) &&
		      islower(*fl) && capitalize;
		if ( cap )
		    *fl = toupper(*fl);
		
		/* Copy new string into place */
		memcpy(file->curchar,replace_str[choice],new_len);
		/* Restore replace string for next time */
		if ( cap )
		    *fl = tolower(*fl);
		
		/* Redraw line from starting point to eoln */
		update_line(file,options,cut_buff,file->curline,
			    ACTUAL_COL(file));
		
		/* Move to end of new insert, not past, or we'll
		   miss the next match if it comes right after. */
		file->curchar += new_len;
		file->curcol = ACTUAL_COL(file);
		move_left(file, options, cut_buff);
		SET_MODIFIED(file);
	    }
	    else
		/* Inefficient: should only redraw the string */
		update_line(file,options,cut_buff,file->curline,file->leftcol);
		/* Something like this would be better
		rewrite(file, options, cut_buff,
		    file->curline, ACTUAL_COL(file),
		    file->curline, ACTUAL_COL(file) + strlen(temp));
		*/
	    tw_move_to(file->text, y, x);
	    
	    if (file->search_wrapped)
		break;
	}
	file->insert = save;
	update_cursor(file,options,cut_buff);
	TW_RESTORE_WIN(file->window);
	stat_mesg("Done.");
    }
    if (resp_win != NULL)
	tw_del_win(&resp_win);
}


/*
 * Move to next occurrence of string in file.  Could search forward or
 * backward.  If location is not in first "visible_lines" lines of the
 * window, scroll as necessary.
 */
 
int     move_search(file, string, options, cut_buff, start_line, start_col, visible_lines)
file_t *file;
char   *string;
opt_t  *options;
buff_t *cut_buff;
int     start_line, start_col, visible_lines;

{
    int     len = 0;

    synhigh_update(file, file->curline, options, cut_buff);
    if (*string != '\0')
    {
	if (options->search_forward)
	{
	    len = search_forward(file, string, options, start_line,
		    start_col, visible_lines, cut_buff);
	}
	else
	{
	    len = search_backward(file, string, options, start_line,
		    start_col, visible_lines, cut_buff);
	}
	if (len == 0)
	    sprintw(2, TWC_ST_LEN, "Did not find \"%s\"", string);
    }
    return len;
}


int     compare(cur, search_str, options)
char   *cur, *search_str; 
opt_t  *options;

{
    if (options->case_sensitive)
	return wsmemcmp(cur, search_str);
    else
	return wsmemicmp(cur, search_str);
}


void    check_search_wrap(file_t *file, opt_t *options, size_t line, size_t col)

{
    char    *ok_button[2] = OK_BUTTON;
    
    if ( (line == file->start_line) && (col == file->start_col) )
    {
	popup_mesg("Search wrapped.", ok_button, options);
	TW_RESTORE_WIN(file->window);
	//stat_mesg("Search wrapped.");
	file->search_wrapped = 1;
    }
}


/*
 * Search forward from current position, quitting when reaching
 * start_line, column 0.
 */

int     search_forward(file, string, options, start_line, start_col, visible_lines, cut_buff)
file_t *file;
char   *string;
int     start_line, start_col, visible_lines;
opt_t  *options;
buff_t *cut_buff;

{
    int     line = file->curline, len=0;
    char   *cur = file->curchar;

    do
    {
	if (*cur++ == '\0')
	{
	    if (line++ == file->total_lines - 1)
		line = 0;
	    cur = file->line[line].buff;
	}
	check_search_wrap(file, options, line, (cur - file->line[line].buff));
	len = compare(cur, string, options);
    }   while ((len == 0) &&
	    (cur != file->line[start_line].buff + start_col));

    if ( len )
    {
	/* Move cursor to new position */
	file->curchar = cur;
	file->curline = line;
	file->curcol = (int) (file->curchar - file->line[line].buff);
	if ((file->curline >= file->topline + visible_lines - 6) ||
	    (file->curline < file->topline))
	{
	    file->topline = MAX(0, (int) file->curline - 5);
	    /* Update win only if check_column() didn't */
	    if (check_column(file, options, cut_buff) == 0)
		update_win(file, options, cut_buff);
	}
	else
	{
	    if (check_column(file, options, cut_buff) == 0)
		tw_move_to(file->text, file->curline - file->topline, file->curcol - file->leftcol);
	}
    }
    return (len);
}


/*
 * Search backward from current position, quitting when reaching
 * start_line, column 0.
 */

int     search_backward(file, string, options, start_line, start_col, visible_lines, cut_buff)
file_t *file;
char   *string;
int     start_line, start_col, visible_lines;
opt_t  *options;
buff_t *cut_buff;

{
    int     line = file->curline, len=0;
    char   *cur = file->curchar;

    do
    {
	if (cur-- == file->line[line].buff)
	{
	    do                  /* Skip blank lines */
	    {
		if (line-- == 0)
		    line = file->total_lines - 1;
		cur = file->line[line].buff;
	    } while (*cur == '\0');

	    cur += file->line[line].length;     /* Go to eoln */
	}
	check_search_wrap(file, options, line, cur - file->line[line].buff);
	len = compare(cur, string, options);
    } while ((len == 0) && (cur != file->line[start_line].buff + start_col));

    file->curchar = cur;
    file->curline = line;
    file->curcol = file->curchar - file->line[line].buff;
    if ((file->curline >= file->topline + visible_lines - 1) ||
	(file->curline < file->topline))
    {
	file->topline = 
	    MAX(0,(int) file->curline - (int) visible_lines + 3);
	if (check_column(file, options, cut_buff) == 0)
	    update_win(file, options, cut_buff);
    }
    else
    {
	if (check_column(file, options, cut_buff) == 0)
	    tw_move_to(file->text, file->curline - file->topline, file->curcol - file->leftcol);
    }
    return (len);
}


/***************************************************************************
 * Description:
 *  Return the X11 include directory for use in a find command.
 *
 * History: 
 *  Jul 2009    J Bacon
 ***************************************************************************/

char    *get_x11_include(void)

{
    /*
     *  If X11_INCLUDE is under /usr/x11, strip it to avoid a redundant
     *  find argument.
     */
    if (memcmp(X11_INCLUDE, "/usr/include", 12) == 0)
	return "";
    else
	return X11_INCLUDE;
}


void    grep_headers(file_t *file, opt_t *options)

{
    static char grep_str[TWC_SPEC_LEN + 1];
    char    cmd[CMD_LEN + 1], *x11_include;
    int     status;

    status = panel_get_string(file, options, TWC_SPEC_LEN,
	"Enter egrep pattern: ", "", TWC_VERBATIM, grep_str);
    if ( (status != TWC_INPUT_CANCEL) && (grep_str[0] != '\0') )
    {
	/* Don't search /usr/include twice */
	x11_include = get_x11_include();
	begin_full_screen();
	snprintf(cmd, CMD_LEN,
		 "find /usr/include %s %s %s -type d -exec search_headers '%s' '{}' \\; 2>&1 | more",
		 x11_include, LOCAL_INCLUDE, options->include_path, grep_str);
	spawnlp(P_WAIT, P_ECHO, NULL, NULL, NULL, "sh", "-c", cmd, NULL);
	end_full_screen(EFS_PAUSE);
    }
}


void    search_libs(file_t *file, opt_t *options)

{
    static char symbol[TWC_SPEC_LEN + 1] = "";
    char    cmd[CMD_LEN + 1];
    int     status;

    status = panel_get_string(file, options, SEARCH_STR_LEN,
			    "Enter egrep pattern: ", "", TWC_VERBATIM, symbol);
    if ( (status != TWC_INPUT_CANCEL) && (symbol[0] != '\0') )
    {
	if (strblank(options->lib_path))
	    snprintf(cmd, CMD_LEN, "search_libs %s",
		     symbol);
	else
	    snprintf(cmd, CMD_LEN, "search_libs %s %s/*.a",
		     symbol, options->lib_path);
	run_command(P_WAIT, P_NOECHO, cmd, "sh");
    }
}


void    set_popup_color(win, options)
win_t  *win;
opt_t  *options;

{
    if (TCOLOR_TERM(win->terminal) && !MONO_MODE(options))
    {
	tw_set_foreground(win, options->menu_fg);
	tw_set_background(win, WHITE);
    }
    else if (THAS_REVERSE_MODE(win->terminal))
	TW_SET_MODES(win, REVERSE_MODE);
    else
	TW_SET_MODES(win, HIGHLIGHT_MODE);
    iclear_image(&win->image);  /* Fill with new color or mode */
}


int     set_search(search_string, cut_buff, str_max)
char   *search_string;
buff_t *cut_buff;
size_t  str_max;

{
    if (cut_buff->marked_lines > 0)
	return -1;
    while (str_max-- && ((*search_string = getc(cut_buff->fp)) != EOF))
	++search_string;
    *search_string = '\0';
    rewind(cut_buff->fp);
    return 0;
}


/**********************************************************************
  wsmemcmp():   Whitespace-independent memory comparison
  Description:      
	See if location buff matches str, treating multiple
	whitespace characters as one.  All chars to the end
	of str must be matched, but any portion of buff is sufficient.
	I.e., str must match some or all of the beginning of buff.
  Arguments:
	buff:   
	str:    The nul-terminated string to look for
  Return values:
	
 **********************************************************************/

int     wsmemcmp(char *buff,char *str)

{
    char    *b = buff;
    
    while ( (*b != '\0') && (*str != '\0') && (*b == *str) )
    {
	if ( isspace(*b) )
	{
	    /* Eat additional whitespace */
	    while ( isspace(*str) )
		++str;
	    while ( isspace(*b) )
		++b;
	}
	else
	{
	    /* Advance to next char */
	    ++b;
	    ++str;
	}
    }
    
    /* If we reached the end of str, it's a match */
    /* Return the length of the matching string */
    if ( *str == '\0' )
	return b-buff;
    else
	return 0;
}


/**********************************************************************
  wsimemcmp():   Whitespace-independent memory comparison
  Description:      
	See if location buff matches str, treating multiple whitespace
	characters as one.  All chars to the end of str must be matched,
	but any portion of buff from the beginning is sufficient.
	I.e., str must match some or all of the beginning of buff.
  Arguments:
	buff:   The nul-terminated buffer in which str must be matched.
	str:    The nul-terminated string to look for.
  Return values:
	Number of characters matched in buff, including spaces
	
 **********************************************************************/

int     wsmemicmp(char *buff,char *str)

{
    char    *b = buff;
    
    while ( (*b != '\0') && (*str != '\0') && (tolower(*b) == tolower(*str)) )
    {
	/* Check either b or str, they are the same */
	if ( isspace(*b) )
	{
	    /* Eat additional whitespace */
	    while ( isspace(*str) )
		++str;
	    while ( isspace(*b) )
		++b;
	}
	else
	{
	    /* Advance to next char */
	    ++b;
	    ++str;
	}
    }
    
    /* If we reached the end of str, it's a match */
    /* Return the length of the matching string */
    if ( *str == '\0' )
	return b-buff;
    else
	return 0;
}

