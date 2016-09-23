/*
 * Copyright (c) 1993-Present, Jason W. Bacon, Acadix Software Systems All
 * rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions
 * and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sysexits.h>
#include <fnmatch.h>
#include <twintk.h>
#include <bacon.h>
#include "edit.h"
#include "protos.h"


int
	check_first_ext(old_lang, new_lang)
lang_t *old_lang, *new_lang;

{
    char    old_ext[EXT_LEN + 1], new_ext[EXT_LEN + 1], config_dir[PATH_LEN + 1],
	    from[PATH_LEN + 1], to[PATH_LEN + 1];

    strlcpy(old_ext, old_lang->name_spec, SPEC_LEN);
    strlcpy(new_ext, new_lang->name_spec, SPEC_LEN);
    strtok(old_ext, " ");
    strtok(new_ext, " ");
    if (strcmp(old_ext, new_ext) != 0)
    {
	if (get_config_dir(config_dir, PATH_LEN) != NULL)
	{
	    /* Skip '.' by adding one to pointer */
	    snprintf(from, PATH_LEN, "%s/macros/%s", config_dir, old_ext + 1);
	    snprintf(to, PATH_LEN, "%s/macros/%s", config_dir, new_ext + 1);
	    rename(from, to);
	}
    }
    return 0;
}


int     language_options(file, options)
file_t *file;
opt_t  *options;

{
    int     options_ok,
	    status;
    tw_panel_t panel = TWC_PANEL_INIT;
    win_t  *win;
    lang_t  old_opt;
    char   *exe_options[5] = {"", "Command Line Flag", "Standard Output", "Fixed", NULL},
	   *yes_no[3] = YES_NO_ENUM, auto_wrap[4], *p;

    /* Check for no options */
    if (create_lang_options_if_missing(file, options))
	return FALSE;

    /* Save old options for comparison to see if modified */
    memcpy(&old_opt, file->lang, sizeof(lang_t));

    /* Create options window */
    win = centered_panel_win(20, 75, options);

    /* Input options */
    tw_init_string(&panel, 2, 2, LANG_ID_LEN, 15, TWC_VERBATIM,
		   "Language name:   ",
		   " Common name of the language. (Case sensitive) ",
		   file->lang->lang_name);
    tw_init_string(&panel, 2, 35, SPEC_LEN, 23,
		    TWC_VERBATIM, "File spec(s): ",
	    " Filespec matching source file name, e.g. '*.c', 'Makefile'. ",
		   file->lang->name_spec);
    tw_init_string(&panel, 3, 2, LANG_ID_LEN, TW_COLS(win) - 21, TWC_VERBATIM,
		   "Language ID:     ",
		   " Comment on first line of script identifying language. ",
		   file->lang->id_comment);
    tw_init_string(&panel, 4, 2, TWC_FILENAME_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Compiler:        ",
		   " Compile command name. ", file->lang->compiler_cmd);
    tw_init_string(&panel, 5, 2, OPTION_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Compile options: ",
    " Compiler options preceding source file name in all compile commands. ",
		   file->lang->compile_flags);
    tw_init_string(&panel, 6, 2, COMPILE_ONLY_LEN, TW_COLS(win) - 21, TWC_VERBATIM,
		   "Compile only:    ",
		   " Options used to compile to object (no link). ",
		   file->lang->compile_only_flag);
    tw_init_string(&panel, 7, 2, OPTION_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Link options:    ",
		   " Compiler options that go after the source file name. ",
		   file->lang->link_flags);
    tw_init_string(&panel, 8, 2, SYNTAX_CHECK_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Syntax check:    ",
		   " Compiler options to run syntax check only. ",
		   file->lang->syntax_check_flag);
    tw_init_string(&panel, 9, 2, TWC_FILENAME_LEN, TW_COLS(win) - 21,
		   TWC_VERBATIM, "Run prefix:      ",
		   " Command preceding executable name in execute command. ",
		   file->lang->run_prefix);
    tw_init_string(&panel, 10, 2, TWC_FILENAME_LEN, TW_COLS(win) - 21,
		   TWC_VERBATIM, "Upload prefix:   ",
		   " Command preceding executable name in upload command. ",
		   file->lang->upload_prefix);
    tw_init_string(&panel, 11, 2, ERR_FORMAT_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Error format:    ",
	 " \\fn=filename, \\ln=line #, \\te=message, \\ig=text to ignore. ",
		   file->lang->error_msg_format);
    tw_init_string(&panel, 12, 2, TWC_FILENAME_LEN, TW_COLS(win) - 21, TWC_VERBATIM, "Executable name: ",
	 " \\fn = source file name, \\st = source name without extension. ",
		   file->lang->executable_name);
    tw_init_enum(&panel, 13, 2, EXE_SRC_LEN, exe_options, "Compiler output: ",
		 " Where does compiler output go?  Hit <space> to toggle. ",
		 file->lang->executable_spec);
    tw_init_string(&panel, 13, 40, OUTPUT_FLAG_LEN, TW_COLS(win) - 63,
		   TWC_VERBATIM, "Compile output flag: ",
		   " E.g. '-o ', '-O='.  Include trailing space! ",
		   file->lang->compile_output_flag);
    tw_init_enum(&panel, 14, 2, 4, yes_no, "Auto wrap?       ",
		 " Start new line at right margin?  Hit <space> to toggle. ",
		 auto_wrap);

    if (file->lang->auto_wrap)
	strlcpy(auto_wrap, "Yes", 4);
    else
	strlcpy(auto_wrap, "No", 4);

    do
    {
	status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
	if ( (TW_EXIT_KEY(status) == TWC_INPUT_CANCEL) ||
	      strblank(file->lang->error_msg_format) ||
	      ( strstr(file->lang->error_msg_format,"\\ln") &&
		strstr(file->lang->error_msg_format,"\\fn") &&
		strstr(file->lang->error_msg_format,"\\te") ) )
	{
	    options_ok = 1;
	}
	else
	{
	    char    *buttons[] = OK_BUTTON;
	    
	    popup_mesg("Error message format must be blank\n"
		"or contain \\ln, \\fn, and \\te.", buttons, options);
	    options_ok = 0;
	}
	TW_RESTORE_WIN(win);
    }   while ( ! options_ok );
    
    file->lang->auto_wrap = (strcmp(auto_wrap, "Yes") == 0);

    /* Check for changes */
    if (strcmp(old_opt.compiler_cmd, file->lang->compiler_cmd) ||
	strcmp(old_opt.compile_flags, file->lang->compile_flags) ||
	strcmp(old_opt.link_flags, file->lang->link_flags) ||
	strcmp(old_opt.executable_name, file->lang->executable_name) ||
	strcmp(old_opt.executable_spec, file->lang->executable_spec))
    {
	file->lang_rebuild = TRUE;
    }
    /* Remove leading whitespace from name_spec */
    for (p = file->lang->name_spec; isspace(*p); ++p)
	;
    strlcpy(file->lang->name_spec, p, SPEC_LEN);

    /*
     * Macros directory is named after first extension.  See if it changed.
     */
    check_first_ext(&old_opt, file->lang);

    /* Update error message format if user forgot to */
    check_compiler_options(file->lang, &old_opt);

    /* Update executable and run command */
    set_exe(file);
    snprintf(file->run_cmd, CMD_LEN,
	     "%s ./%s", file->lang->run_prefix, file->executable);

    tw_del_win(&win);

    /* Cause options to be saved if necessary */
    return TW_EXIT_KEY(status) == TWC_INPUT_DONE;
}


void    check_compiler_options(lang_t * lang, lang_t * old)
{
    if ((strcmp(lang->compiler_cmd, old->compiler_cmd) != 0) &&
	(strcmp(lang->error_msg_format, old->error_msg_format) == 0))
    {
	if ((strcmp(lang->compiler_cmd, "gcc") == 0) ||
	    (strcmp(lang->compiler_cmd, "g++") == 0))
	    strlcpy(lang->error_msg_format, GCC_ERROR_FORMAT, ERR_FORMAT_LEN);
	else if ((strcmp(lang->compiler_cmd, "cc") == 0) ||
		 (strcmp(lang->compiler_cmd, "c++") == 0))
	    strlcpy(lang->error_msg_format, ERROR_FORMAT, ERR_FORMAT_LEN);
    }
}


lang_t *
	check_build_opts(file)
file_t *file;

{
    if (file->lang == NULL)
    {
	sprintw(2, TWC_ST_LEN, "No language options exist for %s.",
		file->short_src);
    }
    return file->lang;
}


int     read_lang(char *lang_file, lang_t *lang)

{
    char    line[LANG_OPTS_LINE_LEN + 1], *p, *name, *end;
    int     end_language = 0;
    FILE    *fp;

    fp = fopen(lang_file, "r");
    if ( fp == NULL )
    {
	sprintw(2, 50, "Error loading %s", lang_file);
	return 1;
    }
    
    /* Initialize in case anything is missing from the options file */
    init_lang(lang);

    /* Load from options file */
    while (!end_language && (fgetline(fp, line, LANG_OPTS_LINE_LEN) != EOF))
    {
	p = line;
	//puts(line);
	name = strsep(&p, " \t");

	/*
	 * If this option is blank, replace NULL with "" for strlcpy()
	 */
	if (p == NULL)
	{
	    p = "";
	}
	while (isspace(*p))
	    ++p;
	if (strcmp(name, "name") == 0)
	    strlcpy(lang->lang_name, p, LANG_NAME_LEN);
	else if (strcmp(name, "name_spec") == 0)
	    strlcpy(lang->name_spec, p, SPEC_LEN);
	else if (strcmp(name, "id_comment") == 0)
	    strlcpy(lang->id_comment, p, LANG_ID_LEN);
	else if (strcmp(name, "compiler") == 0)
	    strlcpy(lang->compiler_cmd, p, TWC_FILENAME_LEN);
	else if (strcmp(name, "compile_flags") == 0)
	    strlcpy(lang->compile_flags, p, OPTION_LEN);
	else if (strcmp(name, "compile_output_flag") == 0)
	    strlcpy(lang->compile_output_flag, p, OUTPUT_FLAG_LEN);
	else if (strcmp(name, "syntax_check_flag") == 0)
	    strlcpy(lang->syntax_check_flag, p, SYNTAX_CHECK_LEN);
	else if (strcmp(name, "compile_only_flag") == 0)
	    strlcpy(lang->compile_only_flag, p, COMPILE_ONLY_LEN);
	else if (strcmp(name, "compile_to_asm_flag") == 0)
	    strlcpy(lang->compile_to_asm_flag, p, OPTION_LEN);
	else if (strcmp(name, "preprocess_only_flag") == 0)
	    strlcpy(lang->preprocess_only_flag, p, OPTION_LEN);
	else if (strcmp(name, "link_flags") == 0)
	    strlcpy(lang->link_flags, p, OPTION_LEN);
	else if (strcmp(name, "debugger") == 0)
	    strlcpy(lang->debugger_cmd, p, TWC_FILENAME_LEN);
	else if (strcmp(name, "debugger_flags") == 0)
	    strlcpy(lang->debugger_flags, p, OPTION_LEN);
	else if (strcmp(name, "debugger_backtrace_cmd") == 0)
	    strlcpy(lang->debugger_backtrace_cmd, p, BACKTRACE_LEN);
	else if (strcmp(name, "run_prefix") == 0)
	    strlcpy(lang->run_prefix, p, TWC_FILENAME_LEN);
	else if (strcmp(name, "upload_prefix") == 0)
	    strlcpy(lang->upload_prefix, p, TWC_FILENAME_LEN);
	else if (strcmp(name, "executable_name") == 0)
	    strlcpy(lang->executable_name, p, TWC_FILENAME_LEN);
	else if (strcmp(name, "executable_source") == 0)
	    strlcpy(lang->executable_spec, p, EXE_SRC_LEN);
	else if (strcmp(name, "error_msg_format") == 0)
	    strlcpy(lang->error_msg_format, p, ERR_FORMAT_LEN);
	else if (strcmp(name, "auto_wrap") == 0)
	{
	    lang->auto_wrap = strtod(p, &end);
	    if (end == p)
	    {
		fputs("Error reading auto_wrap in language options.\n", stderr);
		return EOF;
	    }
	}
	else if (strcmp(name, "end_language") == 0)
	{
	    end_language = 1;
	}
	else
	{
	    fprintf(stderr, "Warning: Invalid language option: %s\n", name);
	}
    }
    fclose(fp);
    if (end_language)
	return 0;
    else
	return EOF;
}


int     read_language_opts(lang_t **head)

{
    lang_t  temp, *lang;
    long    status;
    char    language_parent_dir[PATH_LEN+1],
	    lang_dir[PATH_LEN+1],
	    filename[PATH_LEN+1];
    DIR     *dp;
    struct dirent *dir_entry;
    struct stat     st;
    extern term_t *Terminal;
    
    /* For each directory under Languages, read language opts and syntax opts */
    get_language_parent_dir(language_parent_dir,PATH_LEN); 
    dp = opendir(language_parent_dir);
    if ( dp != NULL ) 
    {
	while ( (dir_entry = readdir(dp)) != NULL )
	{ 
	    if ( dir_entry->d_name[0] != '.' )
	    {
		snprintf(lang_dir, PATH_LEN, "%s/%s",
		    language_parent_dir, dir_entry->d_name);
		if ( stat(lang_dir, &st) != 0 )
		{
		    sprintw(2, 50, "Could not stat %s", dir_entry->d_name);
		    tgetc(Terminal);
		    return CANT_LOAD;
		}
		if ( S_ISDIR(st.st_mode) )
		{
		    snprintf(filename, PATH_LEN, "%s/%s/language_opts",
			language_parent_dir, dir_entry->d_name);
		    
		    if ( (status = read_lang(filename, &temp)) == 0 )
		    {
			lang = MALLOC(1, lang_t);
			*lang = temp;
			snprintf(filename, PATH_LEN, "%s/%s/syntax_highlighting",
			    language_parent_dir, dir_entry->d_name);
			synhigh_load_opts(filename, lang);
			lang->next = *head;
			*head = lang;
		    }
		}
	    }
	}
	closedir(dp);
	/* Check for trailing garbage in config file */
	if (status != EOF)
	    return CANT_LOAD;
	else
	    return OK;
    }
    else
	sprintw(2, 50, "Error opening dir %s.", language_parent_dir);
    return OK;
}


int     save_language(lang_t *lang)

{
    char    lang_dir[PATH_LEN+1],
	    lang_file[PATH_LEN+1];
    FILE    *fp;
    
    if ( lang == NULL )
	return NO_LANGUAGE_OPTS;

    if ( get_language_dir(lang,lang_dir,PATH_LEN) == 0 )
    {
	snprintf(lang_file, PATH_LEN, "%s/language_opts", lang_dir);
	/* Create language dir if it doesn't already exist. */
	mkdir(lang_dir,0777);
	fp = fopen(lang_file, "w");
	if ( fp != NULL )
	{
	    fprintf(fp, "name                    %s\n", lang->lang_name);
	    fprintf(fp, "name_spec               %s\n", lang->name_spec);
	    fprintf(fp, "id_comment              %s\n", lang->id_comment);
	    fprintf(fp, "compiler                %s\n", lang->compiler_cmd);
	    fprintf(fp, "compile_flags           %s\n", lang->compile_flags);
	    fprintf(fp, "compile_output_flag     %s\n", lang->compile_output_flag);
	    fprintf(fp, "syntax_check_flag       %s\n", lang->syntax_check_flag);
	    fprintf(fp, "compile_only_flag       %s\n", lang->compile_only_flag);
	    fprintf(fp, "compile_to_asm_flag     %s\n", lang->compile_to_asm_flag);
	    fprintf(fp, "preprocess_only_flag    %s\n", lang->preprocess_only_flag);
	    fprintf(fp, "link_flags              %s\n", lang->link_flags);
	    fprintf(fp, "debugger                %s\n", lang->debugger_cmd);
	    fprintf(fp, "debugger_flags          %s\n", lang->debugger_flags);
	    fprintf(fp, "debugger_backtrace_cmd  %s\n", lang->debugger_backtrace_cmd);
	    fprintf(fp, "run_prefix              %s\n", lang->run_prefix);
	    fprintf(fp, "upload_prefix           %s\n", lang->upload_prefix);
	    fprintf(fp, "executable_name         %s\n", lang->executable_name);
	    fprintf(fp, "executable_source       %s\n", lang->executable_spec);
	    fprintf(fp, "error_msg_format        %s\n", lang->error_msg_format);
	    fprintf(fp, "auto_wrap               %d\n", lang->auto_wrap);
	    fputs("end_language\n", fp);
	    fclose(fp);
	}
	else
	    sprintw(2,50,"Error saving language options for %s.",lang->lang_name);
	
	synhigh_save_opts(lang_dir, lang);
    }
    return 0;
}


void    save_language_opts(lang_t *head)

{
    lang_t *lang = head;

    while (lang != NULL)
    {
	save_language(lang);
	lang = lang->next;
    }
}


int     multispec_match(char *name_spec, char *str, int flags)
{
    char   *spec, *p, *token;

    spec = strdup(name_spec);
    if (spec == NULL)
    {
	fprintf(stderr, "multispec_match(): Cannot allocate memory.\n");
	return 1;
    }
    p = spec;
    while ((token = strsep(&p, " \t")) != NULL)
    {
	if (fnmatch(token, str, flags) == 0)
	    return 0;
    }
    free(spec);
    return FNM_NOMATCH;
}


lang_t *get_bop(file_t * file, lang_t * head)
{
    lang_t *lang = head;
    char    first_line[64], *p;

    /* First check language ID if it exists. */
    if (memcmp(file->line[0].buff, "#!", 2) == 0)
    {
	/* Remove whitespace following #! if present */
	strlcpy(first_line, file->line[0].buff, 63);
	for (p = first_line + 2; isspace(*p); ++p);
	strlcpy(first_line + 2, p, 63);

	/* Find matching language ID */
	while ((lang != NULL) && ((*lang->id_comment == '\0') ||
			     (strstr(first_line, lang->id_comment) == NULL)))
	    lang = lang->next;
    }
    else
    {                           /* Rely on filename */
	while ((lang != NULL) &&
	       (multispec_match(lang->name_spec, file->source, 0) != 0))
	    lang = lang->next;
    }
    return lang;
}


lang_t *new_bop(head, filename)
lang_t **head;
char   *filename;

{
    lang_t *lang;
    char   *ext;

    if ((ext = strrchr(filename, '.')) == NULL)
	ext = "";
    if ((lang = (lang_t *) calloc(1, sizeof(lang_t))) != NULL)
    {
	lang->next = *head;
	snprintf(lang->name_spec, SPEC_LEN, "*%s", ext);
	*head = lang;
    }
    return lang;
}


lang_t *add_language(
		lang_t * head,
		char  *name,
		char  *name_spec,
		char  *id_comment,
		char  *compiler,
		char  *compile_only_flag,
		char  *compile_flags,
		char  *link_flags,
		char  *syntax_check_flag,
		char  *debugger,
		char  *debugger_flags,
		char  *run_prefix,
		char  *error_msg_format,
		char  *executable_name,
		char  *compile_output_flag,
		char  *executable_spec,
		int     auto_wrap,
		char  *compile_to_asm_flag,
		char  *preprocess_only_flag,
		char  *upload_prefix,
		char  *debugger_backtrace_cmd)
{
    lang_t *temp;

    temp = MALLOC(1, lang_t);
    if (temp != NULL)
    {
	strlcpy(temp->lang_name, name, LANG_NAME_LEN);
	strlcpy(temp->name_spec, name_spec, SPEC_LEN);
	strlcpy(temp->id_comment, id_comment, LANG_ID_LEN);
	strlcpy(temp->compiler_cmd, compiler, TWC_FILENAME_LEN);
	strlcpy(temp->compile_flags, compile_flags, OPTION_LEN);
	strlcpy(temp->compile_only_flag, compile_only_flag, COMPILE_ONLY_LEN);
	strlcpy(temp->compile_output_flag, compile_output_flag, OUTPUT_FLAG_LEN);
	strlcpy(temp->syntax_check_flag, syntax_check_flag, SYNTAX_CHECK_LEN);
	strlcpy(temp->compile_to_asm_flag, compile_to_asm_flag, OPTION_LEN);
	strlcpy(temp->preprocess_only_flag, preprocess_only_flag, OPTION_LEN);
	strlcpy(temp->link_flags, link_flags, OPTION_LEN);
	strlcpy(temp->debugger_cmd, debugger, TWC_FILENAME_LEN);
	strlcpy(temp->debugger_flags, debugger_flags, OPTION_LEN);
	strlcpy(temp->debugger_backtrace_cmd, debugger_backtrace_cmd,
	    BACKTRACE_LEN);
	strlcpy(temp->run_prefix, run_prefix, TWC_FILENAME_LEN);
	strlcpy(temp->upload_prefix, upload_prefix, TWC_FILENAME_LEN);
	strlcpy(temp->executable_name, executable_name, TWC_FILENAME_LEN);
	strlcpy(temp->executable_spec, executable_spec, EXE_SRC_LEN);
	strlcpy(temp->error_msg_format, error_msg_format, ERR_FORMAT_LEN);
	temp->auto_wrap = auto_wrap;

	temp->patterns[0] = NULL;
	temp->next = head;
	return temp;
    }
    else
	return NULL;
}


void
	destroy_bop_list(head)
lang_t **head;

{
    lang_t *lang = *head, *prev;

    while (lang != NULL)
    {
	prev = lang;
	synhigh_free_patterns(lang->patterns);
	lang = lang->next;
	free(prev);
    }
    *head = NULL;
}


void
	init_lang(lang_t * lang)
{
    *lang->lang_name = '\0';
    *lang->name_spec = '\0';
    *lang->id_comment = '\0';
    *lang->compiler_cmd = '\0';
    *lang->compile_flags = '\0';
    *lang->compile_output_flag = '\0';
    *lang->syntax_check_flag = '\0';
    *lang->compile_only_flag = '\0';
    *lang->compile_to_asm_flag = '\0';
    *lang->preprocess_only_flag = '\0';
    *lang->link_flags = '\0';
    *lang->debugger_cmd = '\0';
    *lang->debugger_flags = '\0';
    *lang->debugger_backtrace_cmd = '\0';
    *lang->run_prefix = '\0';
    *lang->upload_prefix = '\0';
    *lang->executable_name = '\0';
    *lang->executable_spec = '\0';
    *lang->error_msg_format = '\0';
    lang->auto_wrap = 0;
}
