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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"

/* Macro menu */

int     macro_menu(files,af_ptr,options,event,expand)
file_t  files[];
int     *af_ptr;
opt_t   *options;
event_t *event;
macro_expand_t expand;

{
    int     ch;
    char    selected_text[TWC_MENU_TEXT_LEN+1];
    
    /* Check for new global macros for this language */

    /* Read macro list (sort?) and update macro_text */
    ch = macro_get_key(files+*af_ptr,options,event,selected_text,2,38);
    if ( ch != MENU_NO_SELECTION )
	macro_invoke(files + *af_ptr, ch, options, expand);
    return ch;
}


int     macro_get_key(file_t *file,opt_t *options,event_t *event,
		    char selected_text[],int line,int col)

{
    extern term_t   *Terminal;
    extern win_t    *Swin;
    
    int     ch, start_row = 1, count;
    char    *menu_text[MACRO_MAX_MENU_ITEMS+2], *ok_button[2] = OK_BUTTON;
    win_t   *macro_menu;
    
    if ( (count = read_macros(file,menu_text)) == -1 )
    {
	popup_mesg("No macros defined for this language.",ok_button,options);
	return MENU_NO_SELECTION;
    }

    /* Sort macro text */
    if ( count > 0 )
	qsort(menu_text,count,sizeof(char *),(int (*)())menu_text_cmp);
    
    /* Display macro options */
    macro_menu = tw_menu(Terminal, 2, 38, menu_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);

    tw_set_win_attr(macro_menu,REVERSE_MODE,options->menu_fg,options->menu_bg,
		    BOLD_MODE,options->menu_hl_fg,options->menu_hl_bg);
    ch = tw_get_item(macro_menu, menu_text, event, &start_row,
		    options->reverse_menu, selected_text);
    
    if ( (selected_text != NULL) && (memcmp(selected_text, "SUBMENU", 7) == 0) )
    {
	sprintw(2, 50, "Submenu selected.");
	tgetc(Terminal);
    }

    tw_del_win(&macro_menu);

    /* Macro menu may grow big enough to obscure status window */
    TW_REDRAW_WIN(Swin);
    
    if ( macro_valid_key(ch, menu_text) )
	return ch;
    else
	return MENU_NO_SELECTION;
}


int     macro_valid_key(int ch, char *menu_text[])

{
    int     c;
    char    *p;
    
    for (c=0; menu_text[c] != NULL; ++c)
    {
	p = strchr(menu_text[c], '.');
	/*sprintw(2, 50, "%s %c %c", menu_text[c], ch, p[1]);
	tgetc(Terminal);*/
	if ( (p != NULL) && (ch == tolower(p[1])) )
	    return 1;
    }
    return 0;
}


int     macro_new_item(file_t files[],
			int *af_ptr,
			opt_t *options,
			buff_t *cut_buff,
			char *default_text)

{
    char    menu_text[TWC_MENU_TEXT_LEN+1],
	    macro_dir[PATH_MAX+1]="", path_name[PATH_MAX+1]="",
	    macro_list[PATH_MAX+1]="",
	    *ok_button[2] = OK_BUTTON,
	    *yes_no[] = YES_NO_ENUM, auto_indent[4] = "Yes",
	    auto_load[4] = "No", *buttons[3] = {"[ Yes ]","[ Cancel ]",NULL};
    struct stat st;
    FILE    *fp, *macrofp;
    int     ch, done, flags, status;
    size_t  save_line, save_col, save_topline;
    char    *save_curchar;
    win_t   *win;
    tw_panel_t panel = TWC_PANEL_INIT;
    
    strlcpy(menu_text,default_text,TWC_MENU_TEXT_LEN+1);

    if ( files[*af_ptr].lang == NULL )
	if ( language_options(files+*af_ptr,options) == FALSE )
	    return 0;
    
    if ( !area_ended(cut_buff) )
    {
	if ( tolower(popup_mesg("No area marked.  Use whole file?",
		     buttons,options)) == 'y' )
	{
	    save_topline = files[*af_ptr].topline;
	    save_line = files[*af_ptr].curline;
	    save_col = files[*af_ptr].curcol;
	    save_curchar = files[*af_ptr].curchar;

	    home_file_no_redraw(files+*af_ptr,options,cut_buff);
	    begin_area(files+*af_ptr,cut_buff);
	    end_file_no_redraw(files+*af_ptr);
	    end_line_no_redraw(files+*af_ptr);
	    end_area(files+*af_ptr,options,cut_buff);

	    files[*af_ptr].topline = save_topline;
	    files[*af_ptr].curline = save_line;
	    files[*af_ptr].curcol = save_col;
	    files[*af_ptr].curchar = save_curchar;
	}
	else
	    return 0;
    }
    
    /* do like custom menu, looping until valid action key or quit */
    /* Get macro name with action key */
    tw_init_string(&panel,2,6,TWC_MENU_TEXT_LEN,TWC_MENU_TEXT_LEN,TWC_VERBATIM,"Menu text:   ",
	"Place a '.' before the character that invokes the macro",
	menu_text);
    tw_init_enum(&panel,3,6,4,yes_no,"Auto indent? ",
	" Indent to match line above? ",auto_indent);
    tw_init_enum(&panel,4,6,4,yes_no,"Auto load?   ",
	" Automatically invoke this macro for new files? ",auto_load);
    win = centered_panel_win(10, 65, options);
    
    do
    {
	status = tw_input_panel(win,&panel,TW_LINES(win)-3);
	if ( (TW_EXIT_KEY(status) == TWC_INPUT_CANCEL) ||
	    strblank(menu_text) )
	{
	    cancel_area(files+*af_ptr,cut_buff,options);
	    done = 1;
	}
	else
	{
	    /* Make sure action key was specified */
	    done = strchr(menu_text,'.') != NULL;
	    if ( !done )
	    {
		popup_mesg("Menu text must contain a '.' before the action key.",
			ok_button,options);
		TW_RESTORE_WIN(files[*af_ptr].window);
		TW_RESTORE_WIN(win);
	    }
	    else
	    {
		/* Get filename from text */
		if ( macro_get_config_dir(files+*af_ptr,macro_dir,PATH_MAX) == NO_LANGUAGE_OPTS )
		    return NO_MACROS;
    
		/* See if macro action key is taken */
		macro_get_filename(files+*af_ptr,path_name,PATH_MAX);
		if ( macro_key_taken(files+*af_ptr,macro_key(menu_text)) )
		{
		    sprintw(2,TWC_ST_LEN,"Action key '%c' is already in use.",
			    macro_key(menu_text));
		    done = 0;
		}
	    }
	}
    }   while ( !done );
    
    tw_del_win(&win);
	
    if ( (TW_EXIT_KEY(status) != TWC_INPUT_CANCEL) && ! strblank(menu_text) )
    {
	/* Get macro body */
	if ( stat(macro_dir,&st) == -1 )
	    rmkdir(macro_dir,0755);
	if ( (macrofp = fopen(path_name,"a")) == NULL )
	{
	    sprintw(2,TWC_ST_LEN,"Can't write %s.",path_name);
	    return 1;
	}
	flags = 0;
	if ( strcmp(auto_indent,"Yes") == 0 )
	    flags = 0x01;
	if (strcmp(auto_load,"Yes") == 0)
	    flags |= 0x02;
	macro_write_header(macrofp,macro_key(menu_text),flags,cut_buff);
	rewind(cut_buff->fp);
	while ( (ch=getc(cut_buff->fp)) != EOF )
	    putc(ch,macrofp);
	putc(MACRO_END,macrofp);
	fclose(macrofp);
	rewind(cut_buff->fp);
    
	snprintf(macro_list,PATH_MAX,"%s/menu.txt",macro_dir);
	if ( (fp=fopen(macro_list,"a")) != NULL )
	{
	    fprintf(fp,"%s\n",menu_text);
	    fclose(fp);
	}
	cancel_area(files+*af_ptr,cut_buff,options);
    }
    return 0;
}


int     macro_remove(file,options,event,selected_text)
file_t  *file;
opt_t   *options;
event_t *event;
char    selected_text[];

{
    int     ch;
    char    macro_dir[PATH_MAX+1],
	    path_name[PATH_MAX+1],
	    line[TWC_MENU_TEXT_LEN+1],
	    tempfile[PATH_MAX+1],
	    *p,
	    *yes_no_buttons[] = YES_NO_BUTTONS;
    FILE    *infile,*outfile;
    
    /* Select a macro to remove */
    ch = macro_get_key(file,options,event,selected_text,2,38);
    
    if ( ch == MENU_NO_SELECTION )
	return MENU_NO_SELECTION;
    
    /* Make dang sure */
    if ( tolower(popup_mesg("Are you sure you want to remove this macro??",
		yes_no_buttons, options, __func__)) != 'y' )
	return MENU_NO_SELECTION;
    
    /* Remove macro body from macros file */
    if ( macro_get_config_dir(file,macro_dir,PATH_MAX) == NO_LANGUAGE_OPTS )
	return NO_MACROS;
    macro_get_filename(file,path_name,PATH_MAX);
    macro_remove_body(path_name,ch,options);
    
    /* Remove text from menu */
    snprintf(path_name,PATH_MAX,"%s/menu.txt",macro_dir);
    
    /* Must be on the same filesystem as path_name for rename() */
    snprintf(tempfile,PATH_MAX,"%s.tmp",path_name);
    
    if ( (infile=fopen(path_name,"r")) == NULL )
    {
	sprintw(2,TWC_ST_LEN,"Cannot read %s.",path_name);
	return 1;
    }
    if ( (outfile=fdopen(mkstemp(tempfile),"w")) == NULL )
    {
	sprintw(2,TWC_ST_LEN,"Cannot write %s.",tempfile);
	return 1;
    }
    while ( fgets(line,TWC_MENU_TEXT_LEN,infile) != NULL )
    {
	if ( ((p=strchr(line,'.')) != NULL) && (tolower(p[1])!=ch) )
	    fputs(line,outfile);
    }
    fclose(infile);
    fclose(outfile);
    unlink(path_name);
    rename(tempfile,path_name);
    unlink(tempfile);
    return 0;
}


int     macro_remove_body(path_name,remove_ch,options)
char    *path_name;
int     remove_ch;
opt_t   *options;

{
    FILE    *infp, *outfp, *tempfp;
    char    macro_key, *ok_button[2] = OK_BUTTON;
    buff_t  macro_buff;
    int     flags, ch;
    
    if ( (infp=fopen(path_name,"r")) == NULL )
    {
	popup_mesg("%s(): Unable to open macros file.",
		    ok_button,options, __func__);
	return 1;
    }
    if ( (tempfp = tmpfile()) == NULL )
    {
	popup_mesg("%s(): Unable to create temp file.",
		    ok_button,options, __func__);
	return 1;
    }
    
    /* Copy all but deleted macro to temp file */
    while ( macro_read_header(infp,&macro_key,&flags,&macro_buff) != EOF )
    {
	if ( macro_key != remove_ch )
	{
	    /* Copy macro body to temp file */
	    macro_write_header(tempfp,macro_key,flags,&macro_buff);
	    do
	    {
		ch = getc(infp);
		putc(ch,tempfp);
	    }   while ( ch != '\004' );
	}
	else
	{
	    /* Skip macro body to be deleted */
	    while ( getc(infp) != '\004' )
		;
	}
    }
    
    /* Copy temp file back to original. Low-level I/O or rename() would
       normally be more efficient, but we already have the FILE pointer
       open, plus writing back to the same file saves on fragmentation. */
    rewind(tempfp);
    fclose(infp);
    if ( (outfp = fopen(path_name,"w")) == NULL )
    {
	sprintw(2,TWC_ST_LEN,"Unable to write %s.",path_name);
	return 1;
    }
    while ( (ch=getc(tempfp)) != EOF )
	putc(ch,outfp);
    fclose(tempfp);
    fclose(outfp);
    return 0;
}


int     read_macros(file,menu_text)
file_t  *file;
char    *menu_text[];

{
    char    macro_dir[PATH_MAX+1], macro_list[PATH_MAX+1];
    FILE    *fp;
    static char macro[MACRO_MAX_MENU_ITEMS+2][TWC_MENU_TEXT_LEN+1];
    int     c = 0;
    struct stat st;

    if ( macro_get_config_dir(file,macro_dir,PATH_MAX) == NO_LANGUAGE_OPTS )
	return -1;

    snprintf(macro_list,PATH_MAX,"%s/menu.txt",macro_dir);
    if ( (fp=fopen(macro_list,"r")) != NULL )
    {
	fstat(fileno(fp),&st);
	if ( st.st_size == 0 )
	    return -1;
	
	for (c=0; (c<MACRO_MAX_MENU_ITEMS) &&
		  (fgetline(fp,macro[c],TWC_MENU_TEXT_LEN)!=EOF); ++c)
	{
	    /* Add text to menu */
	    menu_text[c] = macro[c];
	}
	
	fclose(fp);
	menu_text[c] = "";
	menu_text[c+1] = NULL;  // Is this redundant?
	return c-1;
    }
    else
    {
	menu_text[0] = "";
	return -1;
    }
}


int     macro_read_markup(FILE *fp, char *string, size_t max)

{
    int     ch,
	    c,
	    width;
    
    c = 0;
    while ( ((ch = getc(fp)) != EOF) && (isalpha(ch) || (ch == '_')) )
	string[c++] = ch;
    string[c] = '\0';
    
    /* (width) should follow name */
    if ( (strcmp(string, "date") == 0) ||
	 (strcmp(string, "gecos") == 0) ||
	 (strcmp(string, "username") == 0) ||
	 (strcmp(string, "filename_stem") == 0) ||
	 (strcmp(string, "filename") == 0) )
    {
	if ( ch != '(' )
	    return MACRO_BAD_MARKUP;
	
	/* Field width */
	if ( fscanf(fp, "%d", &width) != 1 )
	    return MACRO_BAD_MARKUP;
	
	/* Closing ')' */
	if ( (ch = getc(fp)) != ')' )
	    return MACRO_BAD_MARKUP;
	
	return width;
    }
    return MACRO_NOT_MARKUP;
}


int     macro_expand_markup(file_t *file, FILE *infp, FILE *outfp)

{
    long    start_position = ftell(infp);
    char    macro_cmd[MACRO_MARKUP_NAME_MAX+1];
    struct passwd   *pw_ent;
    int             width;
    
    /* If \ followed by a known command, expand */
    width = macro_read_markup(infp, macro_cmd, MACRO_MARKUP_NAME_MAX);
    
    if ( width == MACRO_BAD_MARKUP )
    {
	fprintf(outfp, "Bad markup: ");
	fseek(infp, start_position, SEEK_SET);
    }
    else if ( width == MACRO_NOT_MARKUP )
    {
	fseek(infp, start_position, SEEK_SET);
	putc('\\', outfp);
    }
    else if ( strcmp(macro_cmd, "date") == 0 )
    {
	char    date_str[30];
	time_t  t;
	time(&t);
	strftime(date_str, 29, "%Y-%m-%d", localtime(&t));
	fprintf(outfp, "%-*s", width, date_str);
    }
    else if ( strcmp(macro_cmd, "username") == 0 )
    {
	pw_ent = getpwuid(getuid());
	fprintf(outfp, "%-*s", width, pw_ent->pw_name);
    }
    else if ( strcmp(macro_cmd, "gecos") == 0 )
    {
	pw_ent = getpwuid(getuid());
	fprintf(outfp, "%-*s", width, pw_ent->pw_gecos);
    }
    else if ( strcmp(macro_cmd, "filename_stem") == 0 )
    {
	char    filename_stem[PATH_MAX+1],
		*last_dot;
	strlcpy(filename_stem, file->source, PATH_MAX);
	if ( (last_dot = strrchr(filename_stem, '.')) != NULL )
	    *last_dot = '\0';
	fprintf(outfp, "%-*s", width, filename_stem);
    }
    else if ( strcmp(macro_cmd, "filename") == 0 )
    {
	fprintf(outfp, "%-*s", width, file->source);
    }
    else
    {
	putc('\\', outfp); /* Not a system() call, print literal \ */
	fseek(infp, start_position, SEEK_SET);
    }
    return width;
}


int     macro_invoke(file_t *file,int ch,opt_t *options, macro_expand_t expand)

{
    FILE    *macrofp;
    char    path_name[PATH_MAX+1],
	    macro_dir[PATH_MAX+1],
	    macro_key,
	    *ok_button[2] = OK_BUTTON;
    int     key, flags, found = 0;
    buff_t  macro_buff;
    
    if ( macro_get_config_dir(file,macro_dir,PATH_MAX) == NO_LANGUAGE_OPTS )
	return NO_MACROS;
    macro_get_filename(file,path_name,PATH_MAX);
    
    /* Test ownership and permissions for security */
    
    /* Read macro into a cut buffer */
    if ( (macrofp=fopen(path_name,"r")) != NULL )
    {
	while ( !feof(macrofp) && !found )
	{
	    macro_read_header(macrofp,&macro_key,&flags,&macro_buff);
	    found = (macro_key == ch) || ((ch == -1) && (flags&MACRO_AUTO_LOAD));
	    if ( !found )
		while ( ((key=getc(macrofp)) != EOF) &&
			(key != MACRO_END) )
		    ;
	}
	if ( found )
	{
	    macro_buff.fp = tmpfile();
	    if ( macro_buff.fp == NULL )
	    {
		popup_mesg("tmpfile() returned NULL! Check your tmp dir.",
		    ok_button,options);
	    }
	    else
	    {
		macro_buff.file = NULL;
		/* Copy macro to cut buffer, expanding markup */
		while ( ((key=getc(macrofp)) != MACRO_END) &&
			(key != EOF) )
		{
		    /*
		     *  Macro command? (\date, \username, ...)
		     */
		    if ( (expand == MACRO_EXPAND) && (key == '\\') )
			macro_expand_markup(file, macrofp, macro_buff.fp);
		    else
			/* Plain text */
			putc(key,macro_buff.fp);
		}
		rewind(macro_buff.fp);
		paste_area(file,options,&macro_buff,flags&MACRO_AUTO_INDENT);
		fclose(macro_buff.fp);
	    }
	}
	fclose(macrofp);
	stat_mesg(BAR_HELP);
	return 0;
    }
    else
    {
	sprintw(2,TWC_ST_LEN,"Could not open macro: ~/%s",path_name);
	return 1;
    }
}


int     macro_get_config_dir(file_t *file,char macro_dir[],size_t maxlen)

{
    char    language_dir[PATH_MAX+1];
    
    if ( get_language_dir(file->lang, language_dir,PATH_MAX) != NO_LANGUAGE_OPTS )
    {
	/* Use the language name as the macro dir */
	snprintf(macro_dir,maxlen,"%s/Macros", language_dir);
	return 0;
    }
    else
	return NO_LANGUAGE_OPTS;
}


int     macro_get_filename(file_t *file,char macro_filename[],size_t maxlen)

{
    char    macro_dir[PATH_MAX+1];
    
    if ( file->lang == NULL )
	return NO_LANGUAGE_OPTS;

    macro_get_config_dir(file,macro_dir,PATH_MAX);
    snprintf(macro_filename,maxlen,"%s/macros",macro_dir);
    
    return 0;
}


/*************************************************************************
 * Author:
 * Created:
 * Modification history:
 * Arguments:
 * Return value:
 * Description:
 *************************************************************************/
 
int     macro_key(char *menu_text)

{
    char    *p;
    
    if ( (p=strchr(menu_text,'.')) != NULL )
    {
	++p;
	if ( isupper(*p) )
	    return tolower(*p);
	else
	    return *p;
    }
    else
	return MACRO_MISSING_ACTION_KEY;
}


/*************************************************************************
 * Author:
 * Created:
 * Modification history:
 * Arguments:
 * Return value:
 * Description:
 *************************************************************************/
 
int     macro_key_taken(file,key)
file_t  *file;
int     key;

{
    char    *menu_text[MACRO_MAX_MENU_ITEMS+2], **item;
    
    if ( read_macros(file,menu_text) == -1 )
	return 0;

    for (item=menu_text; *item != NULL; ++item)
    {
	if (macro_key(*item) == key)
	    return 1;
    }
    return 0;
}


int     macro_read_header(infile,macro_key,flags,macro_buff)
FILE    *infile;
char    *macro_key;
int     *flags;
buff_t  *macro_buff;

{
    unsigned long   start_line, end_line, start_top, end_top,
		    marked_lines, start_line_len;
    int     status;
    
    status = fscanf(infile,"%c %d %lu %lu %hu %hu %lu %lu %d %lu %lu:",
	    macro_key,flags,
	    &start_line, &end_line,
	    &macro_buff->start_col,&macro_buff->end_col,
	    &start_top, &end_top, &macro_buff->deleted,
	    &marked_lines, &start_line_len);
    
    macro_buff->start_line = start_line;
    macro_buff->end_line = end_line;
    macro_buff->start_top = start_top;
    macro_buff->end_top = end_top;
    macro_buff->marked_lines = marked_lines;
    macro_buff->start_line_len = start_line_len;
    return status;
}

    
int     macro_write_header(outfile,macro_key,flags,macro_buff)
FILE    *outfile;
int     macro_key,flags;
buff_t  *macro_buff;

{
    return fprintf(outfile,"%c %d %lu %lu %hu %hu %lu %lu %d %lu %lu:",
	    macro_key, flags,
	    (unsigned long)macro_buff->start_line,
	    (unsigned long)macro_buff->end_line,
	    macro_buff->start_col, macro_buff->end_col,
	    (unsigned long)macro_buff->start_top,
	    (unsigned long)macro_buff->end_top,
	    macro_buff->deleted,
	    (unsigned long)macro_buff->marked_lines,
	    (unsigned long)macro_buff->start_line_len);
}


/*
 * Compare two menu items by their hot-key.  Used to sort custom menus.
 */
 
int     menu_text_cmp(char **pp1,char **pp2)

{
    char    *p1 = *pp1, *p2 = *pp2;

    while ( *p1++ != '.' )
	;
    while ( *p2++ != '.' )
	;
    return *p1-*p2;
}


int     macro_replace(file_t files[], int *af_ptr, opt_t *options,
		    buff_t *cut_buff, event_t *event)

{
    char    selected_text[TWC_MENU_TEXT_LEN+1];
    
    if ( macro_remove(files+*af_ptr, options, event, selected_text) == MENU_NO_SELECTION )
	return MENU_NO_SELECTION;
    else
	return macro_new_item(files, af_ptr, options, cut_buff, selected_text);
}


int     macro_edit(
    file_t files[],
    int *af_ptr,
    opt_t *options,
    buff_t *cut_buff,
    event_t *event )

{
    int     ch;
    char    *yes_no_buttons[] = YES_NO_BUTTONS;
    
    ch = popup_mesg("The raw macro text will be inserted at\nthe current cursor position.  Continue?",
	    yes_no_buttons, options, __func__);
    if ( tolower(ch) == 'y' )
    {
	/* Invoke macro without expanding */
	ch = macro_menu(files, af_ptr, options, event, MACRO_NO_EXPAND);
    }
    return ch;
}


int     macro_new_submenu(file_t files[],
			int *af_ptr,
			opt_t *options,
			buff_t *cut_buff,
			char *default_text)

{
    char    menu_text[TWC_MENU_TEXT_LEN+1],
	    macro_dir[PATH_MAX+1]="", path_name[PATH_MAX+1]="",
	    macro_list[PATH_MAX+1]="",
	    *ok_button[2] = OK_BUTTON,
	    cmd[CMD_LEN+1];
    struct stat st;
    FILE    *fp;
    int     done, status;
    win_t   *win;
    tw_panel_t panel = TWC_PANEL_INIT;
    extern term_t   *Terminal;
    
    strlcpy(menu_text,default_text,TWC_MENU_TEXT_LEN+1);

    if ( files[*af_ptr].lang == NULL )
	if ( language_options(files+*af_ptr,options) == FALSE )
	    return 0;
    /* do like custom menu, looping until valid action key or quit */
    /* Get macro name with action key */
    tw_init_string(&panel,2,6,TWC_MENU_TEXT_LEN,TWC_MENU_TEXT_LEN,TWC_VERBATIM,"Menu text:   ",
	"Place a '.' before the character that invokes the macro",
	menu_text);
    win = centered_panel_win(8, 65, options);
    
    do
    {
	status = tw_input_panel(win,&panel,TW_LINES(win)-3);
	if ( (TW_EXIT_KEY(status) == TWC_INPUT_CANCEL) ||
	    strblank(menu_text) )
	{
	    cancel_area(files+*af_ptr,cut_buff,options);
	    done = 1;
	}
	else
	{
	    /* Make sure action key was specified */
	    done = strchr(menu_text,'.') != NULL;
	    if ( !done )
	    {
		popup_mesg("Menu text must contain a '.' before the action key.",
			ok_button,options);
		TW_RESTORE_WIN(files[*af_ptr].window);
		TW_RESTORE_WIN(win);
	    }
	    else
	    {
		/* Get filename from text */
		if ( macro_get_config_dir(files+*af_ptr,macro_dir,PATH_MAX) == NO_LANGUAGE_OPTS )
		    return NO_MACROS;
    
		/* See if macro action key is taken */
		macro_get_filename(files+*af_ptr,path_name,PATH_MAX);
		if ( macro_key_taken(files+*af_ptr,macro_key(menu_text)) )
		{
		    sprintw(2,TWC_ST_LEN,"Action key '%c' is already in use.",
			    macro_key(menu_text));
		    done = 0;
		}
	    }
	}
    }   while ( !done );
    
    tw_del_win(&win);
	
    if ( (TW_EXIT_KEY(status) != TWC_INPUT_CANCEL) && ! strblank(menu_text) )
    {
	/* Get macro body */
	if ( stat(macro_dir,&st) == -1 )
	    rmkdir(macro_dir,0755);
    
	snprintf(macro_list,PATH_MAX,"%s/menu.txt",macro_dir);
	if ( (fp=fopen(macro_list,"a")) != NULL )
	{
	    fprintf(fp,"SUBMENU %s\n",menu_text);
	    fclose(fp);
	}
	
	/* Create menu and macro files for submenu */
	snprintf(cmd, CMD_LEN, "touch %s/menu-%c.txt %s/macros-%c",
	    macro_dir, macro_key(menu_text), macro_dir, macro_key(menu_text));
	system(cmd);
    }
    return 0;
}

