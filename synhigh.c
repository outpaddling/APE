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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include "pare.h"
#include <xtend/string.h>
#include <xtend/time.h>
#include <xtend/file.h>
#include "edit.h"
#include "protos.h"

/*******************************************************************
 * Description:
 *  If first_line is tagged, back up to the beginning of
 *  the multi-line token to ensure proper tagging.  Tagging will
 *  proceed to last_line or to the end of the multiline token,
 *  whichever is further.
 *******************************************************************/

void 
synhigh_tag_which_lines (file_t *file, size_t first_guess, size_t last_guess, size_t *first_line, size_t *last_line)

{
    //extern term_t   *Terminal;
    //sprintw(2, 50, "total_lines = %d fg = %d lg = %d",
    //    file->total_lines, first_guess, last_guess);
    //tgetc(Terminal);
    
    /* Make sure starting guesses are within available lines */
    first_guess = MAX((long)first_guess, 0);
    last_guess = MIN(last_guess, file->total_lines-1);
    
    /* Back up to first tagged line */
    while ( (first_guess > 0 ) && HAS_START_PATTERN(file,first_guess) )
	--first_guess;

    /* Adjust last line to include tagged lines adjacent to first */
    while ( (last_guess < file->total_lines-2) && HAS_START_PATTERN(file,last_guess) )
	++last_guess;
    
    //sprintw(2, 50, "fg = %d  lg = %d", first_guess, last_guess);
    //tgetc(Terminal);
    *first_line = first_guess;
    *last_line = last_guess;
}


/****************************************************************************
 *
 * Purpose: Tag lines for syntax highlighting
 * Created: May, 1998
 * Modification history:
 *
 * Arguments:
 *
 * Return values:
 *
 * Description:
 *  Tag lines first_line through last line, inclusive.
 *  Each line receives a start_pattern and is marked for update
 *
 ****************************************************************************/

int 
synhigh_tag_lines (file_t *file, size_t first_line, size_t last_line)

{
    size_t  line = first_line, c, next_line;
    char   *p;
    int     pattern_num, in_pattern = FALSE;
    reloc_t end;
    struct timeval t1, t2;

    /* Make sure there's something to look for first */
    if ((file->lang == NULL) || (file->lang->patterns[0] == NULL))
	return 0;

    gettimeofday (&t1, NULL);

    /* Search file from beginning for patterns in lang->patterns */
    /* Tag each line to record positions of multiline patterns */
    while ((line <= last_line) && (line < file->total_lines))
    {
	p = file->line[line].buff;
	while (*p != '\0')
	{
	    pattern_num = synhigh_check_patterns (file, line, p, &end);
	    if ((in_pattern = (pattern_num != NO_PATTERN)))
	    {
		p = end.end_char + 1; /* Skip to end of token to save time */

		/* Tag subsequent lines with start pattern multi-line token */
		if (end.lines > 1)
		{
		    for (c = line + 1, next_line = line + end.lines; c < next_line; ++c)
		    {
			if (file->line[c].start_pattern != pattern_num)
			    file->line[c].needs_update = TRUE;
			file->line[c].start_pattern = pattern_num;
			file->line[c].needs_update = TRUE;
		    }
		    line = next_line;
		}
	    }
	    else
		++p;
	}

	/* Lines tagged out here are not part of a multiline token */
	/* Force update of any lines that are no longer part of a token */
	if (file->line[line].start_pattern != NO_PATTERN)
	    file->line[line].needs_update = TRUE;

	/* Untag lines that aren't in token */
	file->line[line].start_pattern = NO_PATTERN;    /* Clear all tags */
	++line;
    }

    gettimeofday (&t2, NULL);
    return 0;
}


void 
synhigh_check_pattern_color (file_t *file, size_t line, char *ptr, char **end_pattern, opt_t *options, int *in_pattern)

{
    int     pattern_num;
    reloc_t end;

    /* If end of token, turn off highlighting */
    if (ptr == *end_pattern)
    {
	set_colors (file, options);
	*in_pattern = FALSE;
    }

    /* Look for start of a new token */
    if (!*in_pattern)
    {
	/* See if line begins within a token */
	if ( (ptr == file->line[line].buff) &&
	     ((pattern_num = file->line[line].start_pattern) >= 0) )
	{
	    //extern term_t *Terminal;
	    *in_pattern = TRUE;
	    //sprintw(2, 50, "pattern_num = %d line = %d %p %d", pattern_num, line,
	    //    file->line[line].buff, file->line[line].length);
	    //tgetc(Terminal);
	    synhigh_set_pattern_color (file, options,
		file->lang->patterns[pattern_num]);
	}
	else    /* Check current position for all possible patterns */
	{
	    pattern_num = synhigh_check_patterns (file, line, ptr, &end);
	    if ((*in_pattern = (pattern_num != NO_PATTERN)))
	    {
		synhigh_set_pattern_color (file, options, file->lang->patterns[pattern_num]);
		*end_pattern = end.end_char + 1;
	    }
	}
    }
}


/****************************************************************************
 *
 * Author:
 * Purpose: See if any token patterns match at the current position
 * Created: May, 1998
 * Modification history:
 *
 * Arguments:
 *
 * Return values:
 *
 * Description:
 *
 ****************************************************************************/

int     synhigh_check_patterns (file, line, ptr, end)
file_t *file;
size_t  line;
char   *ptr;
reloc_t *end;

{
    pattern_t **pattern;
    cre_t  *cre;

    /* Try to match a configured regular expression to the current pos */
    if (file->lang != NULL)
	for (pattern = file->lang->patterns; *pattern != NULL; ++pattern)
	{
	    cre = &(*pattern)->compiled_re;
	    if ( IS_RE_STARTER(cre,*ptr) )
		if (pare_match (cre, ptr, &file->line[line].buff, sizeof (line_t), end) > 0)
		    return pattern - file->lang->patterns;
	}
    return NO_PATTERN;
}


/****************************************************************************
 *
 * Author: Jason Bacon
 * Purpose: Set text window colors for the given pattern
 * Similar functions and their authors:
 * Created: May, 1998
 * Modification history:
 *
 * Arguments:
 *
 * Description:
 *
 ****************************************************************************/

void 
synhigh_set_pattern_color (file_t *file, opt_t *options, pattern_t *pattern)

{
    extern term_t *Terminal;
    static int  alternate[8] = {4,5,3,2,6,1,4,3};

    if (TCOLOR_TERM (Terminal) && !MONO_MODE(options))
    {
	if ( !TNO_COLOR_VIDEO(Terminal,NCV_BOLD) )
	    TW_SET_MODES (file->text, BOLD_MODE);

	/* Use text foreground? */
	if (pattern->foreground == TEXT_FG)
	    tw_set_foreground (file->text, options->text_fg);
	else if ( pattern->foreground == options->text_bg )
	    tw_set_foreground (file->text, alternate[pattern->foreground%8]);
	else
	    tw_set_foreground (file->text, pattern->foreground);

	/* Use text background? */
	if (pattern->background == TEXT_BG)
	    tw_set_background (file->text, options->text_bg);
	else
	    tw_set_background (file->text, pattern->background);
    }
    else
	TW_SET_MODES (file->text, pattern->modes);
    return;
}


void 
synhigh_clear_tags (file_t *file)

{
    size_t  line;

    for (line = 0; line < file->total_lines; ++line)
	file->line[line].start_pattern = NO_PATTERN;    /* Clear all tags */
}


void 
synhigh_update (file_t *file, size_t line, opt_t *options, buff_t *cut_buff)

{
    size_t  start_line, end_line;

    check_language(file,options,cut_buff);
    
    /* Update it if it's been modified */
    if (file->line[line].needs_update)
    {
	/* See if line is within a multi-line token */
	/* Must check surrounding lines for start patterns */
	synhigh_tag_which_lines(file, line-1, line+1, &start_line, &end_line);
	//sprintw(2, 50, "start_line = %d  end_line = %d", start_line, end_line);
	//sleep(1);
	
	synhigh_tag_lines(file, start_line, end_line);
	
	/* FIXME: Is this updating lines that don't need it? */
	for (line=start_line;
		(line <= end_line) && //file->line[line].needs_update &&
		(line < file->topline + TW_LINES (file->text)); ++line)
	{
	    //sprintw(2, 50, "Updating line %d.", line);
	    update_line (file, options, cut_buff, line, file->leftcol);
	}
	update_cursor (file, options, cut_buff);
    }
}


void 
check_language (file_t *file, opt_t *options, buff_t *cut_buff)

{
    /* Check for new info on language type */
    if ( (file->line[0].needs_update) &&
	 (memcmp(file->line[0].buff,"#!",2) == 0) )
    {
	select_compiler(file,options);
	synhigh_tag_lines(file,0,file->total_lines - 1);
	update_win(file,options,cut_buff);
    }
}


/****************************************************************************
 *
 * Title:
 * Author:
 * Purpose:
 * Similar functions and their authors:
 * Created:
 * Modification history:
 *
 * Arguments:
 *
 * Return values:
 *
 * Description:
 *
 * Conventions:
 *      Yellow: comments
 *      Blue:   strings/constants
 *      Green:  keywords
 *      Red:    types and type modifiers
 *      Magenta:    procedures and functions
 *
 ****************************************************************************/

int     synhigh_load_opts (char *filename, lang_t *lang)

{
    int     fg, bg, modes, c = MAX_PATTERNS, count;
    char    str[MAX_RE + 1];
    FILE    *fp;

    if (lang == NULL)
	return 0;

    fp = fopen(filename, "r");
    if ( fp == NULL )
    {
	sprintw(2, 50, "Error loading %s.", filename);
	return 1;
    }
    
    /* Read syntax highlighting config from config_dir/options.rc */
    /* compile expressions into pattern->cre as they are read */
    lang->patterns[0] = NULL;   /* Mark end of list for add_pattern */
    do
    {
	count = fscanf (fp, "%d %d %d\n", &fg, &bg, &modes);
	xt_fgetline(fp,str,MAX_RE);
	/*
	printf("Read %d %d %d %s\n",fg,bg,modes,str);
	   fflush(stdout);
	   getchar();
	*/
	synhigh_add_pattern (lang->patterns, str, fg, bg, modes);
    }   while ((count == 3) && (fg != END_PATTERNS) && (c-- > 0));
    fclose(fp);
    return 0;
}


int 
synhigh_add_pattern (pattern_t *patterns[], char *re, int fg, int bg, int modes)

{
    int     p;
    static int count = 0;

    /* Don't add end marker as a pattern */
    if (fg == END_PATTERNS)
	return 0;

    ++count;
    for (p = 0; (patterns[p] != NULL) && (p <= MAX_PATTERNS); ++p)
	;

    if ( p <= MAX_PATTERNS )
    {
	patterns[p] = MALLOC (1, pattern_t);
	patterns[p]->re = strdup(re);
	synhigh_compile_pattern (patterns[p]);
	patterns[p]->foreground = fg;
	patterns[p]->background = bg;
	patterns[p]->modes = modes;
	patterns[p + 1] = NULL;
	return 0;
    }
    else
	return -1;
}


/****************************************************************************
 *
 * Title:
 * Author:
 * Purpose:
 * Created: May 31, 1998
 * Modification history:
 *
 * Arguments:
 *
 * Return values:
 *
 * Description:
 *
 ****************************************************************************/

int     synhigh_save_opts (char *lang_dir, lang_t *lang)

{
    pattern_t   **p;
    FILE        *fp;
    char        synhigh_file[APE_PATH_MAX+1];
    
    if ( lang == NULL )
	return NO_LANGUAGE_OPTS;
    
    snprintf(synhigh_file, APE_PATH_MAX, "%s/syntax_highlighting", lang_dir);
    fp = fopen(synhigh_file, "w");
    if ( fp != NULL ) 
    {
	for (p = lang->patterns; *p != NULL; ++p)
	{
	    fprintf (fp, "%d %d %d\n%s\n", (*p)->foreground, (*p)->background,
		     (*p)->modes, (*p)->re);
	}
	fprintf (fp, "%d 0 0\nend\n",END_PATTERNS);
	fclose(fp);
    }
    else
	sprintw(2, 50, "Error writing %s.", synhigh_file);
    return 0;
}


int 
synhigh_compile_pattern (pattern_t *pattern)

{
    pare_compile (&pattern->compiled_re, pattern->re);
    return 0;
}


void 
draw_modes_bar (win_t *win, int line, int col)

{
    int     mode, save_modes = TW_CUR_MODES (win), save_fg = TW_CUR_FOREGROUND (win),
	    save_bg = TW_CUR_BACKGROUND (win);

    tw_move_to (win, line, col);
    tw_puts (win, " Screen modes: ");
    tw_set_foreground (win, WHITE);
    tw_set_background (win, BLACK);
    for (mode = NORMAL_MODE; mode <= HIGHLIGHT_MODE; mode <<= 1)
    {
	TW_SET_MODES (win, NORMAL_MODE | mode);
	tw_printf (win, " %d ", mode);
    }
    TW_SET_MODES (win, save_modes);
    tw_set_foreground (win, save_fg);
    tw_set_background (win, save_bg);
}


int 
synhigh_options (file_t *file, opt_t *options, buff_t *cut_buff)

{
    extern term_t *Terminal;
    tw_panel_t panel = TWC_PANEL_INIT;
    win_t  *win;
    int     status, c, line, save, max_colors = TMAX_COLORS (Terminal),
	    fg[MAX_PATTERNS], bg[MAX_PATTERNS], modes[MAX_PATTERNS];
    char    str[MAX_PATTERNS][MAX_RE + 1];

    /* Check for options and create if desired */
    if (create_lang_options_if_missing (file, options))
	return FALSE;

    win = centered_panel_win(18, 75, options);

    /* Set up integer array for input panel */
    for (c = 0; file->lang->patterns[c] != NULL; ++c)
    {
	fg[c] = file->lang->patterns[c]->foreground;
	bg[c] = file->lang->patterns[c]->background;
	modes[c] = file->lang->patterns[c]->modes;
	strlcpy (str[c], file->lang->patterns[c]->re, MAX_RE);
    }

    while (c < MAX_PATTERNS)
    {
	fg[c] = bg[c] = modes[c] = 0;
	*str[c] = '\0';
	++c;
    }

    /* Set up input panel */
    for (c = 0; c < MAX_PATTERNS; ++c)
    {
	// f = c * 4;
	line = c + 2;
	tw_init_int (&panel, line, 2, -1, max_colors, "Fg: ",
		  " Foreground color. -1 matches text foreground. ", &fg[c]);
	tw_init_int (&panel, line, 9, -1, max_colors, "Bg: ",
		  " Background color. -1 matches text background. ", &bg[c]);
	tw_init_int (&panel, line, 16, 0, 255, "Modes: ",
		  " Modes may be combined by adding mode values. ", &modes[c]);
	tw_init_string (&panel, line, 27, MAX_RE, TW_COLS (win) - 38,
		     TWC_VERBATIM, "Pattern: ",
		     " Regular expression identifying token. ", str[c]);
    }
    
    /* Add visual aids for color and modes */
    draw_color_bar (win, 11, options);
    draw_modes_bar (win, 12, 16);
    
    /* Input changes */
    status = tw_input_panel (win, &panel, TW_LINES (win) - 3);
    tw_del_win (&win);
    
    if ((save = TW_EXIT_KEY (status)) == TWC_INPUT_DONE)
    {
	for (c = 0; (*str[c] != '\0') && (c < MAX_PATTERNS); ++c)
	{
	    if ( file->lang->patterns[c] == NULL )
	    {
		file->lang->patterns[c] = MALLOC(1,pattern_t);
		file->lang->patterns[c]->re = NULL;
	    }
	    file->lang->patterns[c]->foreground = fg[c];
	    file->lang->patterns[c]->background = bg[c];
	    file->lang->patterns[c]->modes = modes[c];
	    if (file->lang->patterns[c]->re != NULL)
		free(file->lang->patterns[c]->re);
	    file->lang->patterns[c]->re = strdup(str[c]);
	    synhigh_compile_pattern (file->lang->patterns[c]);
	}
	file->lang->patterns[c] = NULL; /* Mark end of patterns */
	synhigh_tag_lines (file, 0, file->total_lines - 1);
	update_win (file, options, cut_buff);
    }
    return save;
}


void 
synhigh_free_patterns (pattern_t *patterns[])

{
    int     c;
    
    for (c=0; patterns[c] != NULL; ++c)
    {
	pare_free(&patterns[c]->compiled_re);
	free(patterns[c]->re);
	free(patterns[c]);
    }
}


void 
update_lines (file_t *file, opt_t *options, buff_t *cut_buff, size_t first, size_t last)

{
    size_t  line;
    
    last = MIN(last,file->topline+TW_LINES(file->text));
    for (line=first; (line<last) || 
	    (file->line[line].needs_update &&
	    (line<file->topline+TW_LINES(file->text))); ++line)
	update_line(file,options,cut_buff,line,file->leftcol);
    update_cursor(file,options,cut_buff);
}

