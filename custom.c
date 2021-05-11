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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"


int     custom_menu(file_t files[],
		    int *af_ptr,
		    opt_t *options,
		    event_t *event)

{
    extern term_t   *Terminal;
    extern win_t    *Swin;
    static int ch, item_count;
    int     c, ch2, start_row = 1, skip;
    win_t  *custom_pop;
    char    *pos, *ok_button[2] = OK_BUTTON;
    static char *custom_text[MAX_CUSTOM_ITEMS + FIXED_CUSTOM_ITEMS + 1] = {
					".Unix shell prompt (F10)",
					"Unix .Command",
					TWC_HLINE,
					".Add menu item",
					".Delete menu item",
					".Edit item",
					TWC_HLINE,
					""};
    static cust_t menu_items[MAX_CUSTOM_ITEMS];
    item_count = read_custom(custom_text, menu_items);
    custom_pop = tw_menu(Terminal, 1, 36, custom_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr(custom_pop,REVERSE_MODE,options->menu_fg,options->menu_bg,
		    BOLD_MODE,options->menu_hl_fg,options->menu_hl_bg);
    ch = tw_get_item(custom_pop, custom_text, event, &start_row,
		    options->reverse_menu,NULL);

    switch (ch)
    {
	case 'a':
	    if (item_count < MAX_CUSTOM_ITEMS)
	    {
		add_item(menu_items, item_count, options);
		item_count = read_custom(custom_text, menu_items);
		tw_del_win(&custom_pop);
		custom_pop = tw_menu(Terminal, 1, 36, custom_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
	    }
	    else
		popup_mesg( "No more menu items.", ok_button, options);
	    break;
	case 'd':
	    if (item_count != 0)
	    {
		stat_mesg( "Please select the item to delete.");
		skip = tw_get_item(custom_pop, custom_text, event, &start_row,
				options->reverse_menu,NULL);
		write_items(menu_items, skip, item_count);
		item_count = read_custom(custom_text, menu_items);
		tw_del_win(&custom_pop);
		custom_pop = tw_menu(Terminal, 1, 36, custom_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
	    }
	    break;
	case 'e':
	    if (item_count != 0)
	    {
		stat_mesg( "Please select the item to edit.");
		do
		    ch2 = tw_get_item(custom_pop, custom_text, event,
				    &start_row, options->reverse_menu,NULL);
		while ( strchr("aderu",ch2) != NULL );
		for (c = 0; c < item_count; ++c)
		{
		    pos = strchr(menu_items[c].text, '.');
		    if (pos != NULL)
			if (tolower(pos[1]) == ch2)
			{
			    edit_item(menu_items, item_count, menu_items + c,
				      options, OLD_ITEM);
			    break;
			}
		}
		if (c < item_count)
		{
		    write_items(menu_items, 0, item_count);
		    item_count = read_custom(custom_text, menu_items);
		    tw_del_win(&custom_pop);
		    custom_pop = tw_menu(Terminal, 1, 36, custom_text,
			&options->border,
			 options->no_acs, MONO_MODE(options),
			 options->menu_fg, options->menu_bg,
			 options->menu_hl_fg, options->menu_hl_bg);
		}
	    }
	    break;
	case 'c':
	    /*
	    sprintw(2, 50, "active_file = %d  file->window = %p",
		*af_ptr, files[*af_ptr].window);
	    tgetc(Terminal);
	    */
	    
	    run_unix(files, *af_ptr, options);
	    break;
	case 'u':
	    suspend(options);
	    /* Assume tgetevent() missed release event because
	       begin_full_screen() masks them */
	    Terminal->button_status = BUTTON_UNKNOWN;
	    break;
	case KEY_LEFT:
	case KEY_RIGHT:
	    break;
	default:
	    run_custom_item(files,af_ptr,options,menu_items,item_count,ch);
	    break;
    }
    tw_del_win(&custom_pop);
    /* *af_ptr may have been changed by run_custom_item */
    TW_RESTORE_WIN(files[*af_ptr].window);
    
    /* Custom menu may grow big enough to obscure status win */
    TW_RESTORE_WIN(Swin);
    return (ch);
}


int     run_unix(file_t files[], int aw, opt_t *options)

{
    extern win_t    *Swin;
    static char    cmd[CMD_LEN+1] = "";
    int     status;
    
    status = panel_get_string(files + aw, options, CMD_LEN, "Command? ", "",
	TWC_VERBATIM, cmd);

    if ( (cmd[0] != '\0') && (status != TWC_INPUT_CANCEL) )
    {
	if ( prompt_save_all(files, options) != 'c' )
	    return run_command(P_WAIT, P_NOECHO, cmd, options->ishell);
    }
    return 0;
}


/*
 * Run a program selected from the custom menu
 */
 
int     run_custom_item(file_t files[],
			int *af_ptr,
			opt_t *options,
			cust_t menu_items[],
			int item_count,
			int ch)

{
    char    temp[PATH_MAX + 1];
    struct stat *old_stat, new_stat;
    int     c;
    
    old_stat = (struct stat *) malloc(options->max_files * sizeof(struct stat));
    
    /* Get modification times on all files */
    for (c = 0; c < options->max_files; ++c)
	if (files[c].window != NULL)
	    stat(files[c].source, &old_stat[c]);

    /* Run command */
    run_item(files, *af_ptr, menu_items, item_count, ch, options);

    /* See if custom command modified an open file */
    for (c = 0; c < options->max_files; ++c)
    {
	if (files[c].window != NULL)
	{
	    if (stat(files[c].source, &new_stat) != -1)
		if (new_stat.st_mtime > old_stat[c].st_mtime)
		{
		    /* Reload file if modified and desired */
		    if (1)
		    {
			strlcpy(temp, files[c].source, PATH_MAX);
			close_file(files, c, options, PROMPT_BEFORE_CLOSE);
			*af_ptr = open_file(files, temp, options, OPEN_FLAG_NORMAL);
			sprintw(2,TWC_ST_LEN,"Reloaded %s.",files[*af_ptr].source);
		    }
		}
	}
    }
    free(old_stat);
    return 0;
}


/******************************
 * Add new item to custom menu
 ******************************/

void    add_item(cust_t menu_items[],
		int item_count,
		opt_t *options)

{
    cust_t  item = CUST_INIT;
    int     status;

    status = edit_item(menu_items, item_count, &item, options, NEW_ITEM);
    if ( isgraph(item.text[0]) && (status != TWC_INPUT_CANCEL) )
	save_item(&item);
}


/*
 * Allow user to alter a custom menu item
 */
 
int     edit_item(cust_t menu_items[],
	int item_count,
	cust_t *item,
	opt_t *options,
	int new_item)

{
    extern term_t   *Terminal;
    win_t  *win;
    tw_panel_t panel = TWC_PANEL_INIT;
    static char *execution_modes[] = {"Foreground", "Background", NULL}, *echo_options[] = {"Yes", "No", NULL}, echo[YES_NO_LEN + 1];
    int     no_hot_key=0, key_taken = 0, status;
    char   *key, *ok_button[2] = OK_BUTTON;

    win = centered_panel_win(12, 74, options);

    if (item->echo_command == P_WAIT)
	strlcpy(echo, "Yes", YES_NO_LEN);
    else
	strlcpy(echo, "No", YES_NO_LEN);

    tw_init_string(&panel, 2, 2, TWC_MENU_TEXT_LEN, TW_COLS(win)-24, TWC_VERBATIM, "Menu text?         ",
		"Place a '.' before the character which selects the option.",
		item->text);
    tw_init_string(&panel, 3, 2, PATH_MAX, TW_COLS(win)-24, TWC_VERBATIM, "Working directory? ",
	 "A \"cd\" to this directory will occur before running the command.",
		item->directory);
    tw_init_string(&panel, 4, 2, CMD_LEN, TW_COLS(win)-24, TWC_VERBATIM, "Command or script? ",
		"\\fn = current file, \\st = \\fn without extension, \\in = key input.", item->command);
    tw_init_enum(&panel, 5, 2, MIN(OPTION_LEN, TW_COLS(win)-24), execution_modes, "Execution mode?    ",
	      "Hit <space> to toggle value.", item->run_mode);
    tw_init_enum(&panel, 6, 2, MIN(OPTION_LEN, TW_COLS(win)-24), echo_options, "Echo command?      ",
	      "Hit <space> to toggle value.", echo);
    do
    {
	status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
	key = strchr(item->text, '.');
	no_hot_key = ((key == NULL) && isgraph(item->text[0]));
	if (no_hot_key)
	    popup_mesg( "Menu text must specify a selection key.",
		    ok_button, options);
	else
	{
	    if (key != NULL)
	    {                   /* Add option was aborted */
		item->hot_key = tolower(key[1]);
		if (new_item && (key_taken = taken(menu_items, item_count, key[1])))
		    sprintw(2, TWC_ST_LEN, "Selection key '%c' is already used by another menu item.",
			    key[1]);
		if (strcmp(echo, "Yes") == 0)
		    item->echo_command = P_ECHO;
		else
		    item->echo_command = P_NOECHO;
	    }
	}
    } while ((TW_EXIT_KEY(status) == TWC_INPUT_DONE) && (no_hot_key || key_taken));
    tw_del_win(&win);
    return TW_EXIT_KEY(status);
}


/*
 * Return true if a custom menu item key is already taken by
 * another item.
 */
 
int     taken(cust_t menu_items[],
	    int item_count,
	    int key)

{
    int     c;

    key = tolower(key);
    if (strchr("ruade", key) != NULL)
	return 1;
    for (c = 0; c < item_count; ++c)
    {
	if (menu_items[c].hot_key == key)
	    return 1;
    }
    return 0;
}


void    write_items(cust_t items[],
		    int skip,
		    int item_count)

{
    int     c, fd;
    char   *pos,
	    config_dir[PATH_MAX+1],
	    pathname[PATH_MAX+1];

    if ( get_config_dir(config_dir, PATH_MAX) != NULL )
    {
	snprintf(pathname, PATH_MAX, "%s/custom_menu", config_dir);

	/* Write back all but the item selected for deletion */
	if ((fd = open(pathname, O_WRONLY | O_TRUNC)) == -1)
	{
	    sprintw(2, TWC_ST_LEN, "write_items(): Cannot update %s", pathname);
	    return;
	}
	for (c = 0; c < item_count; ++c)
	{
	    if ((pos = strchr(items[c].text, '.')) != NULL)
		if (tolower(pos[1]) != skip)
		    if (write(fd, items + c, sizeof(*items)) != sizeof(*items))
		    {
			sprintw(2, TWC_ST_LEN, "write_items(): Error writing %s",
				pathname);
			break;
		    }
	}
	close(fd);
    }
}


int     read_custom(char *custom_text[],
		    cust_t menu_items[])

{
    int     c, fd;
    char    config_dir[PATH_MAX+1],
	    pathname[PATH_MAX+1];
    struct stat statinfo;

    if (get_config_dir(config_dir, PATH_MAX) == NULL)
	return (0);
    snprintf(pathname, PATH_MAX, "%s/custom_menu", config_dir);
    if ((fd = open(pathname, O_RDONLY)) == -1)
	return (0);
    fstat(fd, &statinfo);
    if (statinfo.st_size % sizeof(cust_t) != 0)
    {
	sprintw(2, TWC_ST_LEN, "Custom menu corrupt.  Removing %s.", pathname);
	unlink(pathname);
	return (0);
    }
    for (c = 0; (read(fd, menu_items + c, sizeof(*menu_items)) != 0)
	 && (c < MAX_CUSTOM_ITEMS); ++c)
	custom_text[c + FIXED_CUSTOM_ITEMS] = menu_items[c].text;
    close(fd);
    custom_text[c + FIXED_CUSTOM_ITEMS] = "";
    
    /* Sort variable menu items */
    if ( c > 0 )
	qsort(custom_text+FIXED_CUSTOM_ITEMS,c,sizeof(char *),
		(int (*)())menu_text_cmp);
    return (c);
}


void    save_item(cust_t *item)

{
    int     fd;
    char    pathname[PATH_MAX+1],
	    config_dir[PATH_MAX+1];

    if (get_config_dir(config_dir, PATH_MAX) == NULL)
	return;
    snprintf(pathname, PATH_MAX, "%s/custom_menu", config_dir);
    fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0700);
    if (fd == -1)
    {
	sprintw(2, TWC_ST_LEN, "Cannot append %s.", pathname);
	getchar();
	return;
    }
    write(fd, item, sizeof(*item));
    close(fd);
}


void    run_item(file_t files[],
		int aw,
		cust_t menu_items[],
		int item_count,
		int ch,
		opt_t *options)

{
    int     c;
    char    cmd[CMD_LEN + 1], cwd[PATH_MAX + 2];

    /* Find menu item matching key */
    for (c = 0; c < item_count; ++c)
    {
	if (menu_items[c].hot_key == ch)
	{
	    if ( prompt_save_all(files, options) == 'c' )
		return;
	    getcwd(cwd, PATH_MAX);
	    if (chdir(menu_items[c].directory) == -1)
	    {
		sprintw(2, TWC_ST_LEN, "Cannot chdir to %s", menu_items[c].directory);
		return;
	    }
	    expand_command(files[aw].source, "", menu_items[c].command,
			   cmd, CMD_LEN);
	    if (strcmp(menu_items[c].run_mode, "Foreground") == 0)
		run_command(P_WAIT, menu_items[c].echo_command, cmd, options->shell);
	    else
		run_command(P_NOWAIT, menu_items[c].echo_command, cmd, options->shell);
	    chdir(cwd);
	}
    }
}


void    expand_command(char *source_file, char *executable,
		       char *command, char *expanded, size_t maxlen)

{
    extern win_t    *Swin;
    char    base[PATH_MAX + 1], input[CMD_LEN + 1], *p, *temp = expanded;

    while ( (*command != '\0') && (temp - expanded < maxlen) )
    {
	if (*command == '\\')
	{
	    if (memcmp(command, "\\fn", 3) == 0)
	    {
		command += 3;
		for (p = source_file; *p != '\0';)
		    *temp++ = *p++;
	    }
	    else if (memcmp(command, "\\ex", 3) == 0)
	    {
		command += 3;
		for (p = executable; *p != '\0';)
		    *temp++ = *p++;
	    }
	    else if (memcmp(command, "\\st", 3) == 0)
	    {
		command += 3;
		strlcpy(base, source_file, PATH_MAX);
		if ((p = strrchr(base, '.')) != NULL)
		    *p = '\0';
		for (p = base; *p != '\0';)
		    *temp++ = *p++;
	    }
	    else if (memcmp(command, "\\in", 3) == 0)
	    {
		command += 3;
		*temp = *input = '\0';
		stat_mesg(expanded);
		tw_get_string(Swin, input, PATH_MAX, TWC_ST_LEN-TW_CUR_COL(Swin)-2, TWC_VERBATIM, NULL);
		for (p = input; *p != '\0';)
		    *temp++ = *p++;
	    }
	    else
		*temp++ = *command++;
	}
	else
	    *temp++ = *command++;
    }
    *temp = '\0';
    // FIXME: Deal with truncated commands
}


void    suspend(opt_t *options)

{
    begin_full_screen();
    puts("Type \"exit\" to return to APE.");
    fflush(stdout);
    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,options->ishell,NULL);

/* Works only if parent is a shell
#else
    puts("Type \"fg\" to return to APE.");
    fflush(stdout);
    kill(getpid(),SIGSTOP);
#endif
*/
    end_full_screen(EFS_NO_PAUSE);
}

