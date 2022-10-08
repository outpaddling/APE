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
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <twintk.h>
#include <xtend/string.h>
#include "edit.h"
#include "protos.h"


/****************************************************
 * Update the text window with the current file view
 ****************************************************/

void    update_win(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    win_t   *text = file->text;
    int     line, l;

    /* Redraw - check for marked area and highlight if necessary */
    for (l = 0, line=file->topline;
	(l < TW_LINES(text)) && (line < file->total_lines); ++l, ++line)
    {
	update_line(file,options,cut_buff,line,file->leftcol);
    }
    tw_clear_to_eow(text);
    tw_move_to(file->text,file->curline-file->topline,
	    ACTUAL_COL(file)-file->leftcol);
    /* Mutual recursion: update_cursor->check_column->update_win */
    /*update_cursor(file,options,cut_buff);*/
}


/******************************************************************
 * Redraw a portion of a line beginning at column start_col in the
 * file buffer.
 ******************************************************************/
 
void        update_line(file,options,cut_buff,line,start_col)
opt_t       *options;
buff_t      *cut_buff;
file_t      *file;
buff_len_t  start_col;
size_t      line;

{
    int     c,win_col,cols = TW_COLS(file->text) - 1, /* -1 for auto margin */
	    area=0;
    int     in_pattern = FALSE;
    char    *p, *endp;
    win_t   *text = file->text;
    char    *end_pattern = NULL;
    size_t  line1,line2;
    buff_len_t   col1,col2;

    /* Set pattern color for start if leftcol > 0 */
    if ( file->lang != NULL )
    {
	set_colors(file,options);   /* Start with normal colors. should this be necessary? */
	for (p=file->line[line].buff, endp=MIN(p+file->leftcol, p+file->line[line].length);
		p<endp; ++p)
	    if ( !in_pattern || (p == end_pattern) )
		synhigh_check_pattern_color(file,line,p,&end_pattern,options,&in_pattern);
    }
    
    /* Draw portion of line on screen */
    win_col = start_col - file->leftcol;
    if ( (win_col >= 0) && (win_col < cols) && (line >= file->topline) &&
	 (line < file->topline + TW_LINES(text)) )
    {
	tw_move_to(text, line-file->topline, win_col);
	if ( (line < file->total_lines) &&
	     (start_col <= file->line[line].length) )
	{
	    if ( (area = in_area(file,cut_buff,line,start_col,
				&line1,&col1,&line2,&col2)) )
		set_highlight(file,options);
		
	    /* Draw portion of line within screen */
	    /* For loops are somewhat redundant so that "if" is outside */
	    p = file->line[line].buff + start_col;
	    if ( file->lang != NULL )
	    {
		for (c = 0; (c < cols) && (*p != '\0'); ++c, ++p)
		{
		    /* Check if start or end of marked area is reached */
		    if ( area )
			check_highlight(file,options,line,start_col+c,line1,col1,line2,col2);
		    else    /* Highlight patterns if present */
		    {
			/* Save as many function calls as possible */
			if ( !in_pattern || (p == end_pattern) )
			    synhigh_check_pattern_color(file,line,p,&end_pattern,options,&in_pattern);
		    }
		    ape_putc(text, *p);
		}
	    }
	    else    /* No syntax highlighting */
	    {
		for (c = 0; (c < cols) && (*p != '\0'); ++c, ++p)
		{
		    /* Check if start or end of marked area is reached */
		    if ( area )
			check_highlight(file,options,line,start_col+c,line1,col1,line2,col2);
		    ape_putc(text, *p);
		}
	    }

	    if ( area )
		/* Turn off highlighting for marked area if needed */
		unset_highlight(file,options);
	    else
	    {
		in_pattern = FALSE;
		set_colors(file,options);
	    }
	}
    }
    file->line[line].needs_update = FALSE;
    //tw_set_foreground(text, 7);
    //tw_set_background(text, 0);
    tw_clear_to_eol(text);
}


/*
 * Turn highlighting on or off if at beginning or end of marked area,
 * respectively.
 */
 
void    check_highlight(file,options,line,col,line1,col1,line2,col2)
file_t  *file;
opt_t   *options;
size_t  line,line1,line2;
buff_len_t   col,col1,col2;

{
    if ( (line == line2) && (col == col2) )
	unset_highlight(file,options);  /* Turn off highlighting */
    else if ( (line == line1) && (col == col1) )
	set_highlight(file,options);    /* Turn on highlighting */
}

void    set_color(file,no_color,fg,bg)
file_t  *file;
int     no_color,fg,bg;

{
    extern term_t   *Terminal;
    
    if ( TCOLOR_TERM(Terminal) && !no_color )
    {
	tw_set_foreground(file->window,fg);
	tw_set_background(file->window,bg);
    }
    else
    {
	tw_set_foreground(file->window,WHITE);
	tw_set_background(file->window,BLACK);
    }
}


/**********************************
 * Draw border on file edit window
 **********************************/

void    edit_border(file,options)
file_t *file;
opt_t *options;

{
    extern term_t   *Terminal;
    int     c, edit_border, top_right, name_len, left_tee, right_tee;
    char    full_path[FULL_PATH_MAX+10+1];
    
    if ( options->no_acs )
    {
	edit_border = options->edit_border;
	top_right = options->edit_border;
	left_tee = ']';
	right_tee = '[';
    }
    else
    {
	edit_border = ACS_HLINE(Terminal);
	top_right = ACS_URCORNER(Terminal);
	left_tee = ACS_LTEE(Terminal);
	right_tee = ACS_RTEE(Terminal);
    }
    
    if ( strcmp(file->cwd,"/") == 0 )
	snprintf(full_path, FULL_PATH_MAX+10+1, " /%s ",file->source);
    else
	snprintf(full_path, FULL_PATH_MAX+10+1, " %s/%s ",file->cwd,file->source);
    if ( strlen(full_path) > (size_t)(TW_COLS(file->window)-4) )
	snprintf(full_path, FULL_PATH_MAX+10+1, " %s ",file->source);
    name_len = strlen(full_path);
    
    /* Draw border left of name */
    tw_move_to(file->window, 0, 0);
    set_color(file,MONO_MODE(options),options->text_fg,options->text_bg);
    if ( !options->no_acs )
	TW_SET_MODES(file->window,ACS_MODE);
    for (c = 0; c < TW_CENTER_STRING_COL(file->window,full_path); ++c)
	tw_putc(file->window, edit_border);
    tw_putc(file->window,right_tee);
    
    /* Draw name */
    if ( !options->no_acs )
	TW_SET_MODES(file->window,NORMAL_MODE);
    if ( TCOLOR_TERM(Terminal) && !MONO_MODE(options) )
    {
	tw_set_foreground(file->window,options->title_fg);
	tw_set_background(file->window,options->title_bg);
    }
    else
    {
	if ( THAS_REVERSE_MODE(Terminal) )
	    TW_SET_MODES(file->window,REVERSE_MODE|BOLD_MODE);
	else
	    TW_SET_MODES(file->window,STANDOUT_MODE|BOLD_MODE);
    }
    tw_puts(file->window,full_path);
    
    /* Draw border right of name */
    set_color(file,MONO_MODE(options),options->text_fg,options->text_bg);
    if ( !options->no_acs )
	TW_SET_MODES(file->window,ACS_MODE);
    else
	TW_SET_MODES(file->window,NORMAL_MODE);
    tw_putc(file->window,left_tee);
    for (c+=name_len+2; c < TW_COLS(file->window)-1; ++c)
	tw_putc(file->window, edit_border);
    
    /* Put corner in if scroll bars active */
    if ( options->scroll_bars_at_startup )
	tw_putc(file->window,top_right);
    else
	tw_putc(file->window,edit_border);
    if ( !options->no_acs )
	TW_SET_MODES(file->window,NORMAL_MODE);

    /* Restore normal colors */
    if ( !TCOLOR_TERM(Terminal) || MONO_MODE(options) )
	TW_SET_MODES(file->window,NORMAL_MODE);

    if ( options->scroll_bars_at_startup )
    {
	tw_draw_scroll_bar(file->vscroll_bar);
	tw_draw_scroll_bar(file->hscroll_bar);
	tw_move_scroll_bar(file->vscroll_bar,file->topline,file->total_lines);
	
	/* Draw lower right corner to connect scroll bars */
	tw_move_to(file->window,TW_LINES(file->window)-1,TW_COLS(file->window)-1);
	tw_set_foreground(file->window,WHITE);
	tw_set_background(file->window,BLACK);
	if ( options->no_acs )
	    tw_putc(file->window,options->border.lower_right);
	else
	{
	    TW_SET_MODES(file->window,ACS_MODE);
	    tw_putc(file->window,ACS_LRCORNER(Terminal));
	    TW_SET_MODES(file->window,NORMAL_MODE);
	}
	set_colors(file,options);
    }

    TW_RESTORE_WIN(file->window); /* FreeBSD alt charset bug? */
}


/***************************************
 * Switch active file to next open file
 ***************************************/

void    toggle_file(file, af_ptr,options,cut_buff)
file_t  file[];
int    *af_ptr;
opt_t *options;
buff_t  *cut_buff;

{
    int     af;

    /* Select next file window with an open file */
    af = (*af_ptr + 1) % options->max_files;
    while ((af != *af_ptr) && (file[af].window == NULL))
	af = (af + 1) % options->max_files;

    /* If no open files, open untitled - can this happen? */
    if (file[af].window == NULL)
    {
	af = open_file(file,"untitled",options, OPEN_FLAG_NORMAL);
    }

    /* Update screen */
    /*if ( file[af].highlighted && !area_started() )*/
    /* Only needed to erase highlighting */
    update_win(file+af,options,cut_buff);
    edit_border(file+af,options);
    TW_RESTORE_WIN(file[af].window);
    
    /* Change directory */
    if ( chdir(file[af].cwd) == -1 )
	sprintw(2,TWC_ST_LEN,"Cannot open directory \"%s\".",file[af].cwd);
    
    /* Update status bar */
    display_mode(file+af);
    
    *af_ptr = af;
}


/* FIXME: Move this to a library */

const char    *find_ext(const char *filename)

{
    const char    *ext;
    
    /* Get extension, or end of string */
    if ( ((ext = strrchr(filename, '.')) == NULL) ||
	 (ext == filename) )
	ext = filename + strlen(filename);
    return ext;
}


/***********************************************
 * Set up compiler and options for an open file
 ***********************************************/

void    select_compiler(file, options)
file_t *file;
opt_t *options;

{
    const char   *ext;
    
    ext = find_ext(file->source);
    
    /* Check for Fortran 77 */
    if (strcmp(ext,".f") == 0)
	file->max_line_len = 72;
    else
	file->max_line_len = MAX_LINE_LEN;
    
    /* FIXME: Don't hard-code this, do it with language options */
    file->notabs =  (strcmp(ext,".tex") == 0) ||
		    (strcmp(ext,".dbk") == 0) ||
		    (strcmp(ext,".py") == 0) ||
		    (strcmp(file->source,"Portfile") == 0);
		    // FIXME: crashes if we replace the ext check with this
		    // (strcmp(file->lang->lang_name,"Python") == 0);

    /* Find options with matching name spec */
    file->lang = get_bop(file,options->lang_head);
    
    /* Set run command */
    if ( file->lang != NULL )
    {
	/* Set executable */
	set_exe(file);
	snprintf(file->run_cmd, APE_CMD_MAX,
		"%s ./%s",file->lang->run_prefix,file->executable);
    }
    else
	*file->executable = *file->run_cmd = '\0';
}


/*************************
 * Set name of executable
 *************************/

void    set_exe(file)
file_t  *file;

{
    char    *src, *dest, *s2, *ext;

    src=file->lang->executable_name;
    dest=file->executable;
    while ( *src != '\0' )
    {
	if ( *src == '\\' )
	{
	    /* Using source base name? */
	    if ( memcmp(src,"\\st",3) == 0 )
	    {
		if ( (ext = strrchr(file->source, '.')) == NULL)
		    ext = file->source + strlen(file->source);
		s2 = file->source;
		while ( s2 != ext )
		    *dest++ = *s2++;
		src += 3;
	    }
	    /* Using whole source name? */
	    else if ( memcmp(src,"\\fn",3) == 0 )
	    {
		/* strcpy source name to exe */
		s2 = file->source;
		while ( *s2 != '\0' )
		    *dest++ = *s2++;
		src += 3;
	    }
	    else
		*dest++ = *src++;
	}
	else
	    *dest++ = *src++;
    }
    *dest = '\0';
}


/*
 * Return true of options are "blank", meaning they don't contain
 * essential information.
 */

int     blank_opts(opt)
lang_t *opt;

{
    if ( (*opt->name_spec == '\0') && (*opt->syntax_check_flag == '\0')
	&& (*opt->compiler_cmd == '\0') && (*opt->compile_only_flag == '\0')
	&& (*opt->compile_flags == '\0') && (*opt->link_flags == '\0')
	&& (*opt->debugger_cmd =='\0')
	&& (*opt->run_prefix == '\0') && (*opt->error_msg_format == '\0') )
	return 1;
    else
	return 0;
}


/*****************************************
 * Select the first available free window
 *****************************************/

int     get_free_win(file, dir_name, file_name, options)
file_t  file[];
char   *file_name,*dir_name;
opt_t *options;

{
    int     af;

    for (af = 0; (af < options->max_files) && (file[af].window != NULL) && 
		(strcmp(file[af].source, "untitled") != 0); ++af)
	;
    if (af == options->max_files)
	return (NO_FREE_WIN);
    else
	return (af);
}


/*
 * Determine if a file given by path dirname/filename is already loaded
 */
 
int     open_in_aw(files,filename,dirname,options)
file_t  files[];
char    *filename,*dirname;
opt_t   *options;

{
    int     c;
    
    for (c=0; c<options->max_files; ++c)
    {
	if ( (files[c].window != NULL) && 
	    (strcmp(files[c].source,filename) == 0) &&
	    (strcmp(files[c].cwd,dirname) == 0) )
	    return c;
    }
    return FILE_NOT_OPEN;
}


/**********************************************
 * Merge a file into the active file window
 * FIXME: Causes corruption.  Disabled for now
 **********************************************/

void    merge_file(file, path_name, options, cut_buff)
file_t *file;
opt_t   *options;
buff_t  *cut_buff;
char   *path_name;

{
    extern win_t    *Swin;
    int     len, l, curline;
    FILE   *fp;
    char    temp[MAX_LINE_LEN + 1] = "";

    fp = fopen(path_name, "r");
    if (fp == NULL)
    {
	tw_move_to(Swin, 0, 2);
	sprintw(2, TWC_ST_LEN, "Can't open %s", path_name);
	return;
    }
    curline = file->curline;
    file->curcol = ACTUAL_COL(file);

    /* Add one new line from merge file */
    while ((len = read_line(temp, fp, file, options)) != -1)
    {
	/* Shift lines to make room for new text */
	for (l = file->total_lines - 1; l >= curline; --l)
	    file->line[l + 1] = file->line[l];

	/* Allocate buffer for new line */
	ALLOC_LINE(file,curline,len);
	if (file->line[curline].buff == NULL)
	{
	    stat_mesg( "Merge: out of memory.");
	    break;
	}

	/* Put new line in buffer */
	strlcpy(file->line[curline].buff, temp, LINE_BUFF_SIZE(len));
	++file->total_lines;
	++curline;
    }
    fclose(fp);
    
    /* Reset curchar to new curline buff */
    file->curchar = file->line[file->curline].buff + file->curcol;
    
    /* Retag everything to ensure highlighting is correct */
    synhigh_tag_lines(file,0,file->total_lines-1);
    update_win(file,options,cut_buff);
    SET_MODIFIED(file);
}


int     more_lines(file)
file_t *file;

{
    line_t  *temp;

    file->max_lines <<= 1;
    temp = (line_t *) realloc(file->line, file->max_lines * sizeof(line_t));
    if (temp == NULL)
    {
	stat_mesg( "Out of memory.");
	return (NOMEM);
    }
    file->line = temp;
    return (0);
}


int     ape_putc(win_t *text,int ch)

{
    switch(ch)
    {
	case    TAB_FILLER_CHAR:
	    tw_putc(text,'_');
	    break;
	case    '\t':
	    tw_putc(text,'+');
	    break;
	default:
	    if ( isprint(ch) )
		tw_putc(text,ch);
    }
    return 0;
}

