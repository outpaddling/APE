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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"

extern term_t   *Terminal;

/*
 * Simple view of error message file
 */
 
void    view_errors(char *file)

{
    char    *argv[3];
    struct stat st;
    int     s, status;
    
    s = stat(file,&st);
    if ( (s == -1) || (st.st_size == 0) )
	stat_mesg("No errors.");
    else
    {
	argv[0] = "more";
	argv[1] = file;
	argv[2] = NULL;
	begin_full_screen();
	status = spawnvp(P_WAIT,P_NOECHO,argv,NULL,NULL,NULL);
	check_stat(status,argv[0]);
	end_full_screen(EFS_PAUSE);
    }
}


/*
 * Read through error messages and display the next message
 * that matches the format specified for the language of the
 * current file.
 */
 
void    next_error(file_t *files,int *aw,err_t *errfile,
		opt_t * options,buff_t *cut_buff)

{
    char    mesg[ERROR_LEN+1],
	    raw_file[PATH_MAX+1],
	    *last_slash,
	    source_file[PATH_MAX+1],
	    source_dir[PATH_MAX+1],
	    *text = "";
    int     line, c, found = 0, resp = 'n';
    struct stat st;

    /* Open/reopen error file */
    if ( errfile->fp == NULL )
    {
	errfile->fp = fopen(errfile->filename,"r");
	if ( errfile->fp == NULL )
	{
	    stat_mesg("No errors.");
	    return;
	}
    }
    
    do
    {
	/* Read next error message and move if possible */
	line = read_error_msg(errfile,mesg,raw_file,&text,files+*aw,options);
	if (line != 0)
	{
	    /* Full or relative pathname? */
	    if ( *raw_file == '/' )
	    {
		last_slash = strrchr(raw_file,'/');
		strlcpy(source_file,last_slash+1,PATH_MAX);
		strlcpy(source_dir,raw_file,PATH_MAX);
		*strrchr(source_dir,'/') = '\0';
	    }
	    else
		strlcpy(source_file,raw_file,PATH_MAX);
    
	    /* Check open files for filename (and directory if possible) */
	    for (c=0; c < options->max_files; ++c)
	    {
		if ((files[c].window != NULL) &&
			(strcmp(files[c].source,source_file) == 0) &&
		    ((*raw_file != '/') ||
			(strcmp(files[c].cwd,source_dir) == 0)) )
		    break;
	    }
	    
	    /* Found the file already open? */
	    if ( c < options->max_files )
	    {
		*aw = c;    /* Display file at line with error */
		goto_error(files+c,line,options,cut_buff);
		resp = display_error(files+c, line, text, options);
		update_line(files+c, options, cut_buff, line, 0);
	    }
	    else    /* Try to open it */
	    {
		/* Full pathname in error message? */
		if ( *raw_file == '/' )
		    found = (stat(raw_file,&st) == 0);
		else
		{
		    /* File not curently open and only partial pathname given. */
		    /* Last resort: check directories of all open files */
		    for (c=0; !found && (c<options->max_files); ++c)
		    {
			if ( files[c].window != NULL )
			{
			    char    path[PATH_MAX+1];
			    
			    snprintf(path, PATH_MAX, "%s/%s", files[c].cwd, raw_file);
			    if ( stat(raw_file,&st) == 0 )
				found = 1;
			}
		    }
		}
    
		/*
		 *  Open file and go to error message.  Make sure file
		 *  exists first, since we can't completely trust user-defined
		 *  error message formats.
		 */
		if ( found )
		{
		    c = open_file(files,raw_file,options, OPEN_FLAG_NORMAL);
		    if ( c >= 0 )
		    {
			*aw = c;
			goto_error(files+c,line,options,cut_buff);
		    }
		}
		resp = display_error(files + *aw, line, mesg, options);
	    }
	}
	else
	{
	    resp = display_error(files+*aw, line, mesg, options);
	}
	update_win(files+*aw, options, cut_buff);
    }   while ( resp == 'n' );
}


void    remove_linefeed(char *line)

{
    int     end;
    
    end = strlen(line)-1;
    line[MAX(end,0)] = '\0';
}


/*
 * Read next error message and determine line number
 */
 
int     read_error_msg(errfile,mesg,source_file,text,file, options)
err_t   *errfile;
char    mesg[],source_file[],**text;
file_t  *file;
opt_t   *options;

{
    int     line, have_line, have_name, have_text, mismatch;
    char    *mesg_ptr, *fn, *format_ptr, *end_ptr;

    /* Check build options before attempting to parse error message */
    if ( check_build_opts(file) == NULL )
	return 0;
    
    do  /* Find an error message with a line number and text in it */
    {
	have_line = have_name = have_text = mismatch = line = 0;
	mesg_ptr = fgets(mesg,ERROR_LEN,errfile->fp);
	if ( mesg_ptr != NULL )
	{
	    remove_linefeed(mesg);
	    format_ptr = file->lang->error_msg_format;
	    
	    /* Parse message for filename, line number, and text */
	    while ((*format_ptr != '\0') && !mismatch
		&& !(have_line && have_name && have_text))
	    {
		/* Any amount of whitespace in the same spot is a match */
		while ( isspace(*format_ptr) )
		    ++format_ptr;
		while ( isspace(*mesg_ptr) )
		    ++mesg_ptr;
		
		/* Look for line number in message */
		if ( memcmp(format_ptr,"\\ln",3) == 0 )
		{
		    format_ptr += 3;
		    line = strtol(mesg_ptr,&end_ptr,10);
		    
		    /* Does next literal char in message match format? */
		    if ( (end_ptr != mesg_ptr) && (*end_ptr == *format_ptr) )
			have_line = 1;
		    mesg_ptr = end_ptr;
		}
		
		/* Look for filename in message */
		else if ( memcmp(format_ptr,"\\fn",3) == 0 )
		{
		    format_ptr += 3;              /* Next char after \fn */
		    fn = source_file;
		    while ( (*mesg_ptr != *format_ptr) &&
			    (*mesg_ptr != '\0') &&
			    (fn-source_file < PATH_MAX) )
			*fn++ = *mesg_ptr++;
		    *fn = '\0';

		    /* Does next literal char in message match format? */
		    if ( *mesg_ptr == *format_ptr )
			have_name = 1;
		}
		
		/* Text to be displayed in error message window */
		else if ( memcmp(format_ptr,"\\te",3) == 0 )
		{
		    format_ptr += 3;
		    *text = mesg_ptr;
		    if ( !strblank(*text) )
			have_text = 1;
		    /*sprintw(2, 50, "text = '%s'  have_text = %d",
			*text, have_text);
		    tgetc(Terminal);*/
		}
		
		/* Some compiler errors contain useless fluff to discard */
		else if ( memcmp(format_ptr,"\\ig",3) == 0 )
		{
		    format_ptr += 3;
		    while ( !isspace(*mesg_ptr) &&
			    (*mesg_ptr != *format_ptr) &&
			    (*mesg_ptr != '\0') )
			++mesg_ptr;
		}
		
		/* Match everything else literally, but case-insensitive  */
		else
		{
		    if ( tolower(*mesg_ptr++) != tolower(*format_ptr++) )
		    {
			line = 0; mismatch = 1;
		    }
		}
	    }
	}
    }   while ( (mesg_ptr != NULL) && !(have_line && have_name && have_text) );
    if ( mesg_ptr == NULL )
    {
	rewind(errfile->fp);
	strlcpy(mesg,"*** No more error messages ***",ERROR_LEN);
    }
    return line;
}


/*
 * Display error message
 */
 
void    goto_error(file_t *file,int compiler_line,
		opt_t *options,buff_t *cut_buff)

{
    long    line;
    
    /* Move to error in file */
    edit_border(file,options);
    chdir(file->cwd);
    
    if ( (line = get_current_line(file,compiler_line)) != -1 )
    {
	file->curline = MIN(line,file->total_lines-1);
	file->topline =
	    MAX(line - (int)(TW_LINES(file->text) - ERROR_WIN_ROWS) / 2, 0);
	file->curcol = 0;
	file->curchar = file->line[file->curline].buff;
    }
    
    check_column(file,options,cut_buff);
    update_win(file,options,cut_buff);
}


int     display_error(file_t *file, size_t line, char *msg, opt_t *options)

{
    //stat_mesg(msg);
    
    /* New format using error window */
    static win_t   *win = NULL, *text_win;
    static char *buttons[] = {"[ Next ]", "[ Quit ]", NULL};
    int     c;
    
    if ( win == NULL )
    {
	win = panel_win(ERROR_WIN_ROWS, TCOLS(Terminal)-2,
			TLINES(Terminal)-8, 1, options);
	text_win = tw_sub_win(win, 4, TW_COLS(win)-2, 1, 1, NO_AUTO_SCROLL);
	set_popup_color(text_win, options);
    }

    //sprintw(2, 50, "line = %d", line);
    if ( (line > 0) && (line < file->total_lines) )
    {
	tw_set_foreground(file->text, BLACK);
	tw_set_background(file->text, WHITE);
	tw_move_to(file->text, line - file->topline - 1, 0);
	for (c=0; c < MIN(TW_COLS(file->text), file->line[line-1].length); ++c)
	    tw_putc(file->text, file->line[line-1].buff[c]);
	while ( c++ < TW_COLS(file->text) )
	    tw_putc(file->text, ' ');
    }
    
    tw_clear_win(text_win);
    tw_puts(text_win,msg);
    TW_RESTORE_WIN(win);
    return tw_button_response(text_win,3,buttons);
}


/*
 * Close error file and re-initialize structure
 */
 
void    err_close(errfile)
err_t   *errfile;

{
    if ( errfile->fp != NULL )
    {
	fclose(errfile->fp);
	errfile->fp = NULL;
    }

    /* fp will still be NULL if next_error() hasn't been called */
    unlink(errfile->filename);
}


/****************************************************************************
 * Created: 5-18-98
 * Modifications:
 * Arguments:
 * Return values:
 *      The actual line matching the line in the compiler message, or
 *      -1 if the line no longer exists.
 * Description:
 *      Return the actual line number matching the line number at the
 *      last compile.  In most cases, line will be very close to compiler
 *      line, since few insertions and deletions generally occur between
 *      compiles.
 ****************************************************************************/
 
long    get_current_line(file,compiler_line)
file_t  *file;
size_t  compiler_line;

{
    long    line = compiler_line;
    
    /* Search from compiler line forward */
    while ( (file->line[line].compiler_line != compiler_line) &&
	    (line < file->total_lines) )
	++line;
    if ( line != file->total_lines )
	return line;
    
    /* Search backwards from compiler line */
    line = compiler_line-1;
    while ( (file->line[line].compiler_line != compiler_line) && (line >= 0) )
	--line;
    return line;
}

