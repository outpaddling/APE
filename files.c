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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"

extern term_t   *Terminal;

/*************************************************
 * Display file pop-down menu and get a selection
 *************************************************/

int     file_menu(files, af_ptr, options, cut_buff, event)
file_t  files[];
buff_t  *cut_buff;
int    *af_ptr;
opt_t  *options;
event_t *event;

{
    extern term_t   *Terminal;
    extern win_t    *File_list;
    char    path_name[PATH_MAX + 1] = "";
    int     ch, af, cancel, start_row = 1;
    win_t  *file_pop;
    static char *file_text[] = {".New",".Open file",//"Open .Encrypted file",
				".Close", ".Merge", ".Save (Ctrl+s)",
				"Save .As", //"Save Encr.Ypted",
				TWC_HLINE,
				".Toggle file (Ctrl+t)",
				TWC_HLINE, ".View header file",
				TWC_HLINE,
				".Quit", ""};
		/**reopen_list[MAX_FILENAMES+1] = {".Myfile",""};*/

    file_pop = tw_menu(Terminal, 1, 0, file_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr(file_pop,REVERSE_MODE,options->menu_fg,options->menu_bg,
		    BOLD_MODE,options->menu_hl_fg,options->menu_hl_bg);
    af = *af_ptr;
    ch = tw_get_item(file_pop, file_text, event, &start_row,
		    options->reverse_menu, NULL);
    switch (ch)
    {
	case 'n':
	    new_blank_file(files, af_ptr, options);
	    break;
	case 'o':
	    if ((af = load_new_file('l', files, options, OPEN_FLAG_NORMAL)) != CANT_LOAD)
		*af_ptr = af;
	    break;
#if 0
	case 'r':   /* Reopen previous file */
	    reopen_pop = tw_menu(Terminal, 2, 15, reopen_list, &options->border,
			     options->use_acs, options->use_color,
			     options->menu_fg, options->menu_bg,
			     options->menu_hl_fg, options->menu_hl_bg);
	
	    ch2 = tw_get_item(reopen_pop, reopen_list, seq, &start_row,
			    options->use_color, options->reverse_menu,
			    options->menu_fg, options->menu_bg
			    options->menu_hl_fg, options->menu_hl_bg);
	    tw_del_win(&reopen_pop);
	    break;
#endif
	case 'e':
	    /*if ((af = load_new_file('l', files, options, OPEN_FLAG_CRYPT)) != CANT_LOAD)
		*af_ptr = af;*/
	    break;
	case 'c':
	    /* Add filename to reopen list */
	    /* add_reopen(file+af,reopen_list); */
	    
	    /* If area started in file, cancel it */
	    if ( area_started(cut_buff) && (cut_buff->file == files+af) )
		cancel_area(files+af,cut_buff,options);
	    
	    /* Close the file */
	    close_file(files, af, options, PROMPT_BEFORE_CLOSE);
	    toggle_file(files, &af, options, cut_buff);
	    *af_ptr = af;
	    break;
	case 'm':
	    File_list = panel_win(TLINES(Terminal)-4, TCOLS(Terminal)-1,
				2,TWC_CENTER_WIN,options);
	    tw_set_win_attr(File_list,REVERSE_MODE,
		    TW_CUR_FOREGROUND(File_list),TW_CUR_BACKGROUND(File_list),
		    NORMAL_MODE,TW_CUR_BACKGROUND(File_list),
		    TW_CUR_FOREGROUND(File_list));
	    tw_get_pathname(File_list, path_name, options->file_spec);
	    tw_del_win(&File_list);
	    if (*path_name == '\0')
		break;
	    merge_file(files + af, path_name, options, cut_buff);
	    chdir(files[af].cwd);
	    break;
	case 'y':
	    //files[af].crypt = 1;
	case 's':
	    synhigh_update(files+af,files[af].curline,options,cut_buff);
	    if (strcmp(files[af].source, "untitled") == 0)
		save_as(files, af, options);
	    else
		save_file(files + af, options);
	    break;
	case 'a':
	    save_as(files, af, options);
	    break;
	case 't':
	    toggle_file(files, af_ptr, options, cut_buff);
	    break;
	case 'v':
	    view_header(files + af, options);
	    break;
	case 'q':
	case 'x':
	    if ((cancel = prompt_save_all(files, options)) == 'c')
		ch = cancel;
	    break;
	default:
	    break;
    }
    TW_RESTORE_WIN(files[*af_ptr].window);
    tw_del_win(&file_pop);
    return (ch);
}


/**************************************
 * Prompt for a filename and open file
 **************************************/

int     load_new_file(int ch, file_t files[], opt_t *options, unsigned int flags)

{
    extern win_t    *File_list;
    extern term_t   *Terminal;
    char    path_name[PATH_MAX + 1] = "", *pn, *button[2] = OK_BUTTON,
	    *message = "Sorry, can't open any more files.";
    int     af;

    if ((af = get_free_win(files, "", "", options)) == NO_FREE_WIN)
    {
	popup_mesg( message, button, options);
	return CANT_LOAD;
    }

    File_list = panel_win(TLINES(Terminal)-4,
			    TCOLS(Terminal)-1,2,TWC_CENTER_WIN,options);
    tw_set_win_attr(File_list,REVERSE_MODE,
		TW_CUR_FOREGROUND(File_list),TW_CUR_BACKGROUND(File_list),
		NORMAL_MODE,TW_CUR_BACKGROUND(File_list),
		TW_CUR_FOREGROUND(File_list));
    tw_get_pathname(File_list, path_name, options->file_spec);
    tw_del_win(&File_list);

    if (*path_name != '\0')
    {
	pn = strtok(path_name," \t");
	while ( pn != NULL )
	{
	    af = open_file(files, pn, options, flags);
	    pn = strtok(NULL," \t");
	}
	return af;
    }
    else
	return CANT_LOAD;
}


int     save_as(files, af, options)
file_t  files[];
int     af;
opt_t  *options;

{
    char    filename[PATH_MAX + 1] = "", temp[PATH_MAX+1] = "",
	    dir_name[PATH_MAX + 1] = "", base_name[PATH_MAX + 1] = "",
	    start_dir[PATH_MAX + 1], save_source[PATH_MAX + 1],
	    save_short[TWC_SHORT_NAME_LEN + 1], msg[128], path[PATH_MAX+1],
	    *buttons[3] = YES_NO_BUTTONS, *ok_button[2] = OK_BUTTON;
    struct stat st;
    int     remove;

    /* Get new filename */
    panel_get_string(files + af, options, PATH_MAX, "Save as? ", "",
	TWC_VERBATIM, temp);
    if (*temp == '\0')
    {
	stat_mesg("File not saved.");
	return CANT_SAVE;
    }
    strshellcpy(filename,temp,PATH_MAX);

    /* Split filename into base name and directory */
    if (get_dirname(filename, dir_name, base_name) == NO_DIR)
    {
	popup_mesg("Directory does not exist.  File not saved.",
		    ok_button, options);
	TW_RESTORE_WIN(files[af].window);
	return CANT_SAVE;
    }

    /* See if selected filename is already open */
    /* FIXME: open_in_aw() takes files[], not *file */
    if ( open_in_aw(files+af, base_name, dir_name, options) != FILE_NOT_OPEN )
    {
	sprintw(2, TWC_ST_LEN,
	  "File %s/%s is currently open.  Can't overwrite.", dir_name, filename);
	return CANT_SAVE;
    }

    /* See if file exists */
    if (stat(filename, &st) == 0)
    {
	snprintf(msg,127,"File \"%s\" exists.  Overwrite?",base_name);
	if( tolower(popup_mesg(msg,buttons,options)) != 'y' )
	    return CANT_SAVE;
    }

    /* Save old name in case new one can't be used */
    strlcpy(start_dir, files[af].cwd, PATH_MAX);
    strlcpy(save_source, files[af].source, PATH_MAX);
    strlcpy(save_short, files[af].short_src, TWC_SHORT_NAME_LEN);

    /* Switch to new name */
    if (chdir(dir_name) == -1)
    {
	sprintw(2, TWC_ST_LEN, "Can't open directory \"%s\".", dir_name);
	return CANT_SAVE;
    }
    strlcpy(files[af].cwd, dir_name, PATH_MAX);
    strlcpy(files[af].source, base_name, PATH_MAX);
    strlcpy(files[af].short_src, files[af].source, TWC_SHORT_NAME_LEN);
    if (save_file(files+af,options) == OK)
    {
	select_compiler(files+af, options);
	edit_border(files+af, options);
	
	/* Remove old file if it exists */
	if ( stat(save_source,&st) == 0 )
	{
	    snprintf(msg,127,"Remove %s?",save_source);
	    remove = popup_mesg(msg,buttons,options);
	    if ( tolower(remove) == 'y' )
	    {
		snprintf(path,PATH_MAX,"%s/%s",start_dir,save_source);
		unlink(path);
	    }
	    TW_RESTORE_WIN(files[af].window);
	}
	return OK;
    }
    else
    {
	sprintw(2,TWC_ST_LEN,"Unable to save %s: Keeping old name.",
	    files[af].source);
	chdir(start_dir);
	strlcpy(files[af].cwd, start_dir, PATH_MAX);
	strlcpy(files[af].source, save_source, PATH_MAX);
	strlcpy(files[af].short_src, save_short, TWC_SHORT_NAME_LEN);
	return CANT_SAVE;
    }
}


void    view_header(file_t *file, opt_t *options)

{
    static char path_name[PATH_MAX + 1] = "";
    char    cmd[CMD_LEN + 1] = "", *ext, *argv[MAX_ARGS],
	    *x11_include = X11_INCLUDE;

    panel_get_string(file, options, PATH_MAX, "Header? ", "",
	TWC_VERBATIM, path_name);
    
    sprintw(2, 50, "path_name = %s", path_name);
    if ( *path_name == '\0' )
	return;
    ext = strrchr(path_name, '.');
    
    /* Check for no extension, ./base, or ../base */
    /*
    if ((ext == NULL) || ((ext <= path_name + 1) && (*path_name == '.')))
	strlcat(path_name, ".h", PATH_MAX);
    */
    if ((path_name[0] == '/') || (path_name[0] == '.'))
	snprintf(cmd, CMD_LEN, "more %s", path_name);
    else
    {
	if ( memcmp(x11_include,"/usr/include",12) == 0 )
	    x11_include = "";
	snprintf(cmd, CMD_LEN,
		"find /usr/include %s %s %s -name %s -exec more {} ;",
		x11_include, LOCAL_INCLUDE, options->include_path, path_name);
    }
    parse_cmd(argv, cmd);
    begin_full_screen();
    spawnvp(P_WAIT, P_NOECHO, argv, NULL, NULL, NULL);
    end_full_screen(EFS_PAUSE);
}


/*****************************************************************
 * Prompt the user to save files.  Reject any invalid responses.
 *****************************************************************/

int     prompt_save_all(file_t files[], opt_t *options)

{
    int     c, sv = ' ';

    for (c = 0; (c < options->max_files) && (sv != 'c'); ++c)
    {
	if (files[c].window != NULL)
	    sv = prompt_save(files, c, options);
    }
    return (sv);
}


/********************************************
 * Prompt the user to save a particular file
 ********************************************/

int     prompt_save(files,af,options)
file_t  files[];
int     af;
opt_t   *options;

{
    int     sv = 'n', status;
    char    *buttons[4] = YES_NO_CANCEL_BUTTONS,
	    msg[128];

    if (files[af].modified)
    {
	snprintf(msg,127,"File \"%s\" has changed.  Save?",files[af].source);
	sv = popup_mesg(msg,buttons,options);
	if (sv == 'y')
	{
	    if ( strcmp(files[af].source,"untitled") == 0 )
		status = save_as(files, af, options);
	    else
		status = save_file(files+af,options);
	    if ( status == CANT_SAVE )
		sv = 'c';   /* Cancel */
	}
    }
    return (sv);
}


/***************************************************************************
 *  Description:
 *      Warn user if a file contains TABs, since APE will convert them to
 *      spaces when saving.
 *
 *  History: 
 *  Date        Name        Modification
 *  2021-05-29  Jason Bacon Begin
 ***************************************************************************/

int     prompt_tabs(file_t *file, opt_t *options)

{
    int     sv = 'n';
    char    *buttons[3] = YES_NO_BUTTONS,
	    msg[200];

    snprintf(msg, 199, "The file \"%s\" contains TAB characters.  APE is a soft-tabs\n"
		       "editor and will replace tabs with spaces when saving.\n"
		       "Open read-only to avoid corrupting the file?",
	     file->source);
    sv = popup_mesg(msg,buttons,options);
    if (sv == 'y')
	file->read_only = 1;
    return (sv);
}


/****************************************************
 * Open a new file and create an edit window for it.
 ****************************************************/

int     open_file(file_t files[], char *path_name, opt_t *options, unsigned int flags)

{
    FILE   *fp;
    int     af,
	    ftype,
	    status;
    char    dir_name[PATH_MAX + 1], base_name[PATH_MAX + 1],
	    temp[PATH_MAX + 1], home[PATH_MAX + 1],
	    *button[2] = OK_BUTTON,
	    cmd[CMD_LEN+1],
	    key[MCRYPT_KEY_LEN+1];
    
    /* Expand ~ to home dir if necessary */
    if (*path_name == '~')
	snprintf(temp, PATH_MAX, "%s/%s", get_home_dir(home, PATH_MAX), path_name + 1);
    else
	strlcpy(temp, path_name, PATH_MAX);

    /* Check if file is a directory */
    ftype = file_type(temp);
    if ( (ftype != S_IFREG) && (ftype != 0) )
    {
	popup_mesg("Not a regular file.", button, options);
	return CANT_OPEN;
    }
    
    /* Get directory and base name for file */
    if (get_dirname(temp, dir_name, base_name) == NO_DIR)
	return CANT_OPEN;

    /* If already open, switch to window */
    if ( (af = open_in_aw(files, base_name, dir_name, options)) != FILE_NOT_OPEN )
	return af;
    
    /* Find a free window */
    if ((af = get_free_win(files, dir_name, base_name, options)) == NO_FREE_WIN)
	return CANT_OPEN;

    /* If reusing "untitled" window, free old stuff */
    if (files[af].window != NULL)
    {
	close_file(files, af, options, PROMPT_BEFORE_CLOSE);
	if (files[af].window != NULL)    /* Check if user cancelled close */
	    return af;
    }

    if (init_file(files + af, options) == NOMEM)         /* Allocate buffer, etc. */
	return CANT_OPEN;

    /* Set filenames and dir name */
    strlcpy(files[af].source, base_name, PATH_MAX);
    strlcpy(files[af].short_src, base_name, TWC_SHORT_NAME_LEN);
    strlcpy(files[af].cwd, dir_name, PATH_MAX);
    strlcpy(files[af].run_directory, dir_name, PATH_MAX);

    /* Hack to allow testing real tabs while editing 
       the rest with expanded tabs. */
    //files[af].expand_tabs = (strcmp(files[af].source,"tabtest") != 0);
    files[af].expand_tabs = TRUE;
    //sprintw(2,50,"expand_tabs = %d",files[af].expand_tabs);
    
    /* Attempt to open file */
    if (chdir(dir_name) == -1)
    {
	sprintw(2, TWC_ST_LEN, "Cannot open directory \"%s\".", dir_name);
	return CANT_OPEN;
    }
    
    /* FIXME: Support other encryption programs */
    if ( flags & OPEN_FLAG_CRYPT )
    {
	// tunset_tty(Terminal,C_LFLAG,ECHO);
	*key = '\0';
	status = panel_get_string(files+af, options, MCRYPT_KEY_LEN,
			    "Key? ", "", TWC_SECURE, key);
	snprintf(cmd, CMD_LEN, "mcrypt --flush -q -F -d -k %s < %s",
	    key, files[af].source);
	fp = popen(cmd, "r");
	files[af].crypt = 1;
    }
    else
    {
	fp = fopen(base_name, "r");
	files[af].crypt = 0;
    }
	
    if (fp != NULL)
    {
	load_file(files + af, fp,options);
	if ( files[af].crypt )
	{
	    pclose(fp);
	
	    /* Erase key from memory for security */
	    memset(key, 0, MCRYPT_KEY_LEN);
	    memset(cmd, 0, CMD_LEN);
	}
	else
	    fclose(fp);
    }
    else if (new_file(files + af) == NOMEM) /* Must come after init_file() */
	return CANT_OPEN;
    
    /* Pick compiler based on name */
    select_compiler(files + af, options);

    /* Must come before update_win! */
    synhigh_clear_tags(files + af);
    synhigh_tag_lines(files + af, 0, files[af].total_lines-1);
    
    edit_border(files + af, options);    /* Put name in window */
    update_win(files + af,options,NULL);
    display_mode(files+af);
    init_compiler_lines(files,options);

    /* Invoke auto-load macro if one exists for this language */
    if (EMPTY_FILE (files + af) &&
	(strcmp (files[af].source, "untitled") != 0))
	macro_invoke (files + af, -1, options, MACRO_EXPAND);

    return af;
}


int     new_file(file)
file_t *file;

{
    ALLOC_LINE(file,0,1);
    if (file->line[0].buff == NULL)
    {
	stat_mesg( "open_file(): Cannot allocate line[0].");
	return NOMEM;
    }
    file->line[0].length = 0;
    file->curchar = file->line[0].buff;
    *file->curchar = '\0';
    file->line[1].buff = NULL;
    return 0;
}


int     init_file(file, options)
file_t *file;
opt_t  *options;

{
    file->insert = 1;
    file->modified = 0;
    file->total_lines = 1;
    file->curline = file->topline = file->old_curline = 0;
    file->curcol = file->leftcol = file->old_col = file->old_leftcol = 0;
    file->max_line_len = MAX_LINE_LEN;
    file->max_lines = MAX_LINES;
    file->lang_rebuild = 0;
    file->crypt = 0;
    file->start_line = 0;
    file->high_line = file->high_col = -1;
    file->line_style = 0;
    file->read_only = 0;
    file->tabs_flagged = 0;

    create_edit_win(file,options);
	
    /* Create an edit buffer - make sure file->max_lines is set! */
    file->line = MALLOC(file->max_lines,line_t);
    
    set_colors(file, options);
    
    undo_stack_init(&file->undo_record);
    return 0;
}


void    create_edit_win(file,options)
file_t  *file;
opt_t   *options;

{
    extern term_t   *Terminal;
    
    /* Create edit window */
    file->window = tw_new_win(Terminal, TLINES(Terminal) - 2, TCOLS(Terminal),
			   1, 0, NO_AUTO_SCROLL);
    if ( options->scroll_bars )
    {
	file->text = tw_sub_win(file->window, TW_LINES(file->window) - 2,
			 TW_COLS(file->window)-1, 1, 0, NO_AUTO_SCROLL);
	file->vscroll_bar = tw_sub_win(file->window, TW_LINES(file->window) - 2,
			     1, 1, TW_COLS(file->window)-1, NO_AUTO_SCROLL);
	file->hscroll_bar = tw_sub_win(file->window, 1, TW_COLS(file->window)-1,
			     TW_LINES(file->window)-1, 0, NO_AUTO_SCROLL);
    }
    else
	file->text = tw_sub_win(file->window, TW_LINES(file->window) - 1,
			 TW_COLS(file->window), 1, 0, NO_AUTO_SCROLL);
}


void    set_colors(file, options)
file_t *file;
opt_t  *options;

{
    extern term_t   *Terminal;
    
    if (TCOLOR_TERM(Terminal) && !MONO_MODE(options))
    {
	tw_set_foreground(file->window, options->title_fg);
	tw_set_background(file->window, options->title_bg);
	tw_set_foreground(file->text, options->text_fg);
	tw_set_background(file->text, options->text_bg);
    }
    TW_SET_MODES(file->window,NORMAL_MODE);
    TW_SET_MODES(file->text,NORMAL_MODE);
}


int     get_dirname(full_name, dir_name, base_name)
char    full_name[], dir_name[], base_name[];

{
    char   *end_dir, temp_path[PATH_MAX + 1], start_dir[PATH_MAX + 1], *new_dir;

    /* Look for last '/' in name to see if path info present */
    strlcpy(temp_path, full_name, PATH_MAX);
    end_dir = strrchr(temp_path, '/');
    if (end_dir != NULL)        /* Path info present in name */
    {
	if (end_dir == temp_path)       /* Directory is / */
	    new_dir = "/";
	else
	    new_dir = temp_path;
	*end_dir = '\0';        /* Cap off directory name */
	getcwd(start_dir, PATH_MAX);
	if (chdir(new_dir) == -1)
	{
	    sprintw(2, TWC_ST_LEN, "Cannot chdir to \"%s\".", temp_path);
	    *dir_name = '\0';   /* Blank out return values */
	    *base_name = '\0';
	    return NO_DIR;
	}
	else
	{
	    getcwd(dir_name, PATH_MAX);         /* Get new directory name */
	    strlcpy(base_name, end_dir + 1, PATH_MAX);
	    chdir(start_dir);   /* Return to original directory */
	}
    }
    else                        /* Only base name given */
    {
	getcwd(dir_name, PATH_MAX);
	strlcpy(base_name, full_name, PATH_MAX);
    }
    return OK;
}


void    close_file(file_t files[],int af,opt_t *options,int prompt)

{
    int     c;

    /* Save file first if necessary */
    if ( prompt != NO_PROMPT_BEFORE_CLOSE )
	if (prompt_save(files,af,options) == 'c')
	    return;

    /* Free window and file buffer */
    tw_del_win(&files[af].text);
    tw_del_win(&files[af].window);
    files[af].window = NULL;

    /* Free file buffers */
    for (c = 0; c < files[af].total_lines; ++c)
	free(files[af].line[c].buff);
    free(files[af].line);
}


/*********************************
 * Load a file into a file buffer
 *********************************/

int     load_file(file_t *file, FILE *fp, opt_t *options)

{
    extern term_t   *Terminal;
    int     len, line = 0, nbytes = 0;
    struct stat st;
    char    temp[MAX_LINE_LEN + 1];

    file->tabs_flagged = 0;
    do
    {
	if ( (len = read_line(temp, fp, file, options)) != -1)
	{
	    ALLOC_LINE(file,line,MAX(len,0));
	    if (file->line[line].buff == NULL)
	    {
		stat_mesg( "Could not allocate line: File is read only.");
		file->read_only = 1;
		tgetc(Terminal);
		break;
	    }
	    else
	    {
		if (len != -1)
		{
		    strlcpy(file->line[line].buff, temp, LINE_BUFF_SIZE(len));
		    file->line[line++].length = len;
		    if (line == file->max_lines-2)
		    {
			if (more_lines(file) == NOMEM)
			{
			    stat_mesg( "Could not allocate more lines: File is read only.");
			    file->read_only = 1;
			    tgetc(Terminal);
			    break;
			}
		    }
		    nbytes += len + 1;
		}
	    }
	}
    }   while (len != -1);
    
    /* If file exists but is empty, allocate one blank line. */
    if ( line == 0 )
	ALLOC_LINE(file, line, 0);
    file->total_lines = line;
    file->curchar = file->line[0].buff;

    /* Get last modification time as save time If file doesn't exist, use now
     * as creation time in case it's created before first save. */
    if (stat(file->source, &st) == 0)
	file->save_time = st.st_mtime;
    else
	time(&file->save_time);

    sprintw(2, TWC_ST_LEN, "Loaded %s: %d lines, %d characters",
	    file->short_src, line, nbytes);
    if (line == 0)
    {
	file->line[0].length = 0;
	*file->curchar = '\0';
	file->total_lines = 1;
    }
    file->line[file->total_lines].buff = NULL;
    return (1);
}


/***********************************************************
 * Read a line from a file stream, expanding tabs to spaces
 * if expand_tabs is set.
 ***********************************************************/

int     read_line(char string[], FILE *fp, file_t *file, opt_t *options)

{
    char   *ptr = string, *end = string + file->max_line_len;
    int     ch = '\0';
    
    /* Check ptr first */
    while ( (ptr < end) && ((ch = getc(fp)) != '\n') && 
	    (ch != '\r') && (ch != EOF) )
    {
	switch (ch)
	{
	    case '\t':
		if ( ! (file->read_only || file->tabs_flagged) )
		{
		    prompt_tabs(file, options);
		    file->tabs_flagged = 1;
		}
		if ( file->expand_tabs )
		{
		    /* Expand tabs to spaces */
		    do
		    {
			*ptr++ = ' ';
		    } while (((ptr - string) % DEFAULT_TAB_STOPS) && (ptr < end));
		}
		else
		{
		    /* Convert tabs to fillers */
		    do
		    {
			*ptr++ = TAB_FILLER_CHAR;
		    }   while (((ptr - string + 1) % DEFAULT_TAB_STOPS) && (ptr < end));
		    *ptr++ = '\t';
		}
		break;
	    
	    case '\f':
		/* FF messes up the tty screen. Just remove it. */
		break;
	    
	    default:
		*ptr++ = ch;
		break;
	}
    }
    *ptr = '\0';                /* Mark end of line */

    /* If \r\n sequence, discard \n */
    if ( ch == '\r' )
    {
	SET_LINE_STYLE(file,LINE_STYLE_CR);
	
	/* If not newline, put it back for next line read */
	if ( (ch = getc(fp)) == '\n' )
	    SET_LINE_STYLE(file,LINE_STYLE_NL);
	else
	    ungetc(ch,fp);
    }
    else if ( ch == '\n' )
	file->line_style |= LINE_STYLE_NL;

    /* Return length of string or EOF flag */
    if ( feof(fp) && ( ptr == string ) )
	return EOF;
    else
	return ptr - string;
}


/*****************************
 * Save file from file buffer
 *****************************/

int     save_file(file_t *file, opt_t   *options)

{
    win_t   *win = NULL;
    tw_panel_t panel = TWC_PANEL_INIT;
    int     l = 0, nbytes = 0, lines, status, match;
    FILE   *fp;
    char    pipe[PATH_MAX + 1],
	    *ok_button[2] = OK_BUTTON,
	    key[MCRYPT_KEY_LEN+1] = "",
	    key2[MCRYPT_KEY_LEN+1] = "",
	    algo[MCRYPT_ALGO_LEN+1],
	    *encryption_algorithms[] = { \
		"cast-128", "gost", "rijndael-128", "twofish", "arcfour",
		"cast-256", "loki97", "rijndael-192", "saferplus",
		"wake", "blowfish-compat", "des", "rijndael-256",
		"serpent", "xtea", "blowfish", "enigma", 
		"rc2", "tripledes", NULL },
	    cmd[CMD_LEN+1],
	    full_path[PATH_MAX+1],
	    backup_path[PATH_MAX+1];

    if (file->read_only)
    {
	popup_mesg( "File is read only.", ok_button, options);
	TW_RESTORE_WIN(file->window);
	return CANT_SAVE;
    }
    
    /* Assume Unix line style for new files */
    if ( LINE_STYLE_UNSET(file) )
	SET_LINE_STYLE(file,LINE_STYLE_NL);

    /* Check if file has been modified by another process since last save */
    /* If it doesn't exist, okily-dokily */
    /* Too many problems over NFS links with clock skews
    if (stat(file->source, &st) == 0)
	if (st.st_mtime > file->save_time)
	{
	    ch = tolower(popup_mesg(msg,buttons,options));
	    TW_RESTORE_WIN(file->window);
	    if ( ch != 'y' )
		return OK;
	}
    */

    snprintf(full_path, PATH_MAX, "%s/%s", file->cwd, file->source);
    snprintf(backup_path, PATH_MAX, "%s/.%s.bak", file->cwd, file->source);
    
    /* Back up file before first save */
    if (!file->saved_once)
    {
	fast_cp(full_path, backup_path);
	file->saved_once = 1;
    }

    if (file->crypt)
    {
	for (match = 0; !match; )
	{
	    *key = *key2 = '\0';
	    win = centered_panel_win(10, 65, options);
	    tw_init_enum(&panel, 2, 3, MCRYPT_ALGO_LEN, encryption_algorithms,
			"Algorithm?  ",
			" mcrypt -a algorithm?  Hit <space> to toggle. ",algo);
	    tw_init_string(&panel, 3, 3, MCRYPT_KEY_LEN, 40, TWC_SECURE,
			"Key?        "," Encryption Key ", key);
	    tw_init_string(&panel, 4, 3, MCRYPT_KEY_LEN, 40, TWC_SECURE,
			"Verify Key: "," Encryption Key ", key2);
	    status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
	    match = (strcmp(key, key2) == 0);
	    if ( ! match )
		popup_mesg("Keys do not match.  Please try again.",
		    ok_button,options);
	    tw_del_win(&win);
	    TW_RESTORE_WIN(file->window);
	}
	
	if (TW_EXIT_KEY(status) != TWC_INPUT_DONE)
	    return OK;
	snprintf(cmd, CMD_LEN, "mcrypt --flush -q -F -a %s -k %s > %s 2> mcrypt.stderr",
	    algo, key, full_path);
	fp = popen(cmd, "w");
    }
    else
	fp = fopen(full_path, "w");
	
    if (fp == NULL)
    {
	popup_mesg("Cannot save file",ok_button,options);
	TW_RESTORE_WIN(file->window);
	return CANT_SAVE;
    }

    for (l = 0, lines = 0; l < file->total_lines; ++l)
    {
	write_line(fp,file,l,options);
	++lines;
	nbytes += file->line[l].length + 1;
    }
    fflush(fp);
    if (file->crypt)
    {
	pclose(fp);
	
	/* Erase key from memory for security */
	memset(key, 0, MCRYPT_KEY_LEN);
	memset(cmd, 0, CMD_LEN);
    }
    else
    {
	fsync(fileno(fp));
	fclose(fp);
    }
    time(&file->save_time);

    /* Script or other interpreted language */
    if ((file->lang != NULL) && (*file->lang->compiler_cmd == '\0'))
	make_exe(full_path);

    sprintw(2, TWC_ST_LEN, "Saved %s: %d lines, %d characters.",
	    file->short_src, lines, nbytes);
    file->modified = 0;
    if (file->crypt)
	unlink(pipe);
    return OK;
}


int     write_line(FILE *fp,file_t *file,size_t l,opt_t *options)

{
    int     sp=0,c;
    char    *line_ptr = file->line[l].buff;
    
    /* Replace leading spaces with tabs for most files.  This is
       important for Makefiles.
       (Exceptions: TeX, MacPorts Portfiles, etc.
       Replacing spaces beyond the first non-space is risky, since
       it may be part of a string literal or other text that should
       not be altered.
       
       This will be deprecated when expand_tabs is fully implemented.
    */
    if ( !file->notabs )
    {
	for (sp=0; isspace(*line_ptr); ++line_ptr)
	    ++sp;
	for (c=0; c < sp / DEFAULT_TAB_STOPS; ++c)
	    putc('\t',fp);
    }
    
    /* Write rest of line as is. */
    line_ptr -= sp % DEFAULT_TAB_STOPS;
    while ( *line_ptr != '\0' )
    {
	if (*line_ptr == TAB_FILLER_CHAR)
	{
	    while (*line_ptr == TAB_FILLER_CHAR)
		++line_ptr;
	    putc('\t',fp);
	}
	else
	    putc(*line_ptr,fp);
	++line_ptr;
    }
    if ( FILE_USES_CR(file) )
	putc('\r',fp);
    if ( FILE_USES_NL(file) )
	putc('\n',fp);
    return 0;
}



void    make_exe(file)
char   *file;

{
    struct stat st;

    /* Set execute bit for each field with read bit set */
    stat(file, &st);
    st.st_mode |= (unsigned long)(st.st_mode & 0444) >> 2;
    chmod(file, st.st_mode);
}


int     file_type(char *filename)

{
    struct stat st;
    
    if ( stat(filename,&st) == 0 )
	return st.st_mode & S_IFMT; /* File type (reg, fifo, dir, ...) */
    else
	return 0;
}


void    new_blank_file(file_t files[], int *af_ptr, opt_t *options)

{
    int     af;
    
    if ((af = open_file(files, "untitled", options, OPEN_FLAG_NORMAL)) != CANT_OPEN)
	*af_ptr = af;
}


int     file_undo(file_t *file, opt_t *options, buff_t *cut_buff)

{
    undo_item_t *undo_item = undo_stack_pop(&file->undo_record);
    
    if ( undo_item != UNDO_STACK_EMPTY )
    {
	switch(UNDO_ITEM_ACTION(undo_item))
	{
	    case    UNDO_ITEM_INSERT_CHAR:
		move_to(file, options, cut_buff, UNDO_ITEM_LINE(undo_item),
			UNDO_ITEM_COL(undo_item));
		del_under(file, options, cut_buff);
		break;
	}
    }
    return OK;
}


int     file_save_for_undo(file_t *file, undo_action_t undo_action,
			    char *deleted_text)

{
    undo_stack_push(&file->undo_record, file->curline, ACTUAL_COL(file),
	      undo_action, deleted_text);
    return OK;
}
