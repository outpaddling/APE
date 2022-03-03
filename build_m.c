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
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xtend/proc.h>
#include <xtend/file.h>
#include <xtend/string.h>
#include <twintk.h>
#include "edit.h"
#include "protos.h"


/*******************************************************
 * Display the build pop-down menu and select an option
 *******************************************************/

int     build_menu(files, af_ptr, project, options, errfile, cut_buff, event)
file_t  files[];
int     *af_ptr;
proj_t  *project;
opt_t *options;
err_t   *errfile;
buff_t  *cut_buff;
event_t *event;

{
    extern term_t   *Terminal;
    int     ch, start_row = 1, c, status = 0;
    win_t *build_pop;
    struct stat mkst;
    static char *build_text[20];

    create_build_menu(build_text, project, files[*af_ptr].lang);
    build_pop = tw_menu(Terminal, 1, 20, build_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr(build_pop, REVERSE_MODE, options->menu_fg, options->menu_bg,
		    BOLD_MODE, options->menu_hl_fg, options->menu_hl_bg);
    ch = tw_get_item(build_pop, build_text, event, &start_row,
		    options->reverse_menu, NULL);
    switch (ch)
    {
	case 's':
	    status = syntax_check(files, *af_ptr, options, errfile);
	    if ( errors(status, errfile) )
		next_error(files, af_ptr, errfile, options, cut_buff);
	    break;
	case 'p':
	    status = preprocess(files, *af_ptr, options);
	    if ( errors(status, errfile) )
		next_error(files, af_ptr, errfile, options, cut_buff);
	    break;
	case 'a':
	    if ( check_build_opts(files+*af_ptr) != NULL )
	    {
		char    out_filename[APE_PATH_MAX+1], *p;
	
		status = compile_prog(files, *af_ptr, errfile, options, ASSEMBLY);
		edit_border(files+*af_ptr, options);
		if ( errors(status, errfile) )
		    next_error(files, af_ptr, errfile, options, cut_buff);
		else
		{
		    /* Open assembly file */
		    strlcpy(out_filename, files[*af_ptr].source, APE_PATH_MAX);
		    if ( (p=strrchr(out_filename, '.')) != NULL )
			strlbasecpy(p, out_filename, ".s", APE_PATH_MAX);
		    if ( (c = open_file(files+*af_ptr, out_filename, options, OPEN_FLAG_NORMAL)) >= 0 )
			*af_ptr = c;
		}
	    }
	    break;
	case 'o':
	    /* Make sure build options exist first */
	    if ( check_build_opts(files+*af_ptr) != NULL )
	    {
		status = compile_prog(files, *af_ptr, errfile, options, OBJECT);
		edit_border(files+*af_ptr, options);
		if ( errors(status, errfile) )
		    next_error(files, af_ptr, errfile, options, cut_buff);
	    }
	    break;
	case 'c':
	    status = clean(files, *af_ptr, project, errfile, options);
	    break;
	case 'i':
	    status = install(files, *af_ptr, project, errfile, options);
	    break;
	case 'b':               /* Need to check all windows for save */
	    status = build(files, *af_ptr, project, errfile, options);
	    if ( errors(status, errfile) )
		next_error(files, af_ptr, errfile, options, cut_buff);
	    break;
	case 'r':
	    status = run_prog(files, *af_ptr, project, errfile, options, RUN_PROG);
	    break;
	case 'u':
	    status = run_prog(files, *af_ptr, project, errfile, options, UPLOAD_PROG);
	    break;
	case 'd':
	    debug(files+*af_ptr, project, NULL, options);
	    break;
	case 't':
	    debug(files+*af_ptr, project,
		files[*af_ptr].lang->debugger_backtrace_cmd, options);
	    break;
	case 'e':
	    //stat_mesg( "Command? ");
	    if ( project->makefile[0] != '\0' )
		set_makefile(project, "makefile", files+*af_ptr, options);
	    else
		panel_get_string(files+*af_ptr, options, APE_CMD_MAX,
		    "Command? ", "", TWC_VERBATIM, files[*af_ptr].run_cmd);
	    break;
	case 'm':
	    if ( stat("makefile", &mkst) == 0 )
		set_makefile(project, "makefile", files+*af_ptr, options);
	    else
		set_makefile(project, "Makefile", files+*af_ptr, options);
	    break;
	case 'v':
	    view_errors(errfile->filename);
	    break;
	case 'g':
	    next_error(files, af_ptr, errfile, options, cut_buff);
	    break;
	case 'x':
	    profile(project, files, *af_ptr, options);
	    break;
	default:
	    break;
    }

    TW_RESTORE_WIN(files[*af_ptr].window);
    tw_del_win(&build_pop);
    return(ch);
}


void    create_build_menu(char *build_text[], proj_t *project, lang_t *lang)

{
    int     c = 0,
	    line_c;
    
    if ( ACTIVE_PROJ(project) || compiled_language(lang) )
    {
	if ( (lang != NULL) && !strblank(lang->upload_prefix) )
	    build_text[c++] = "Build, Upload and .Run (Esc-r or F5)";
	else
	    build_text[c++] = "Build and .Run (Esc-r or F5)";
    }
    else
	build_text[c++] = ".Run (Esc-r or F5)";
    
    if ( (lang != NULL) && !strblank(lang->upload_prefix) ) 
    {
	if ( compiled_language(lang) )
	    build_text[c++] = "Build and .Upload (Esc-w)";
	else
	    build_text[c++] = ".Upload";
    }
    
    if ( (lang != NULL) && !strblank(lang->syntax_check_flag) )
	build_text[c++] = ".Syntax check (Esc-y or F8)";
    
    if ( (lang != NULL) && !strblank(lang->compile_only_flag) )
	build_text[c++] = "Compile to .Object (Esc-i or F6)";

    if ( ACTIVE_PROJ(project) || ((lang != NULL) && compiled_language(lang)) )
    {
	build_text[c++] = ".Build executable (Esc-x or F7)";
	build_text[c++] = ".Clean (Esc-c)";
	build_text[c++] = ".Install";
    }
    
    if ( (lang != NULL) && !strblank(lang->compile_to_asm_flag) )
	build_text[c++] = "Translate to .Assembly language";
    
    if ( (lang != NULL) && !strblank(lang->preprocess_only_flag) )
	build_text[c++] = "View .Preprocessor output";
    
    build_text[c++] = TWC_HLINE;
    line_c = c;
    
    /* Feature: Make this work while viewing a non-source file */
    if ( (lang != NULL) && !strblank(lang->error_msg_format) )
	build_text[c++] = ".Goto next error (Esc-n)";
    
    if ( ACTIVE_PROJ(project) || ((lang != NULL) && compiled_language(lang)) )
	build_text[c++] = ".View compiler errors";
    
    if ( (lang != NULL) && !strblank(lang->debugger_cmd) )
	build_text[c++] = "Run .Debugger";
    
    if ( (lang != NULL) && !strblank(lang->debugger_backtrace_cmd) )
	build_text[c++] = "View function call .trace";
    
    if ( (lang != NULL) && !strblank(lang->debugger_cmd) )
	build_text[c++] = "View e.Xecution profile";
    
    if ( line_c != c )
	build_text[c++] = TWC_HLINE;
    
    build_text[c++] = "Select .Makefile";
    /*"Set execute d.Irectory",*/
    build_text[c++] = "Set .Execute command";
    build_text[c] = "";
}


void    debug(file, project, trace_cmd, options)
file_t  *file;
proj_t  *project;
char    *trace_cmd;
opt_t   *options;

{
    char    cmd[APE_CMD_MAX+1], *expanded_cmd, *argv[MAX_ARGS], *exe,
	    command_file[APE_PATH_MAX+1], debugger_cmd[APE_CMD_MAX+1],
	    *ok_button[2] = OK_BUTTON,
	    *stdout_file = NULL;
    int     status, fd;
    FILE    *fp;
    
    if ( (file->lang == NULL) || (*file->lang->debugger_cmd == '\0') )
    {
	popup_mesg("No debugger configured for this language.",
		    ok_button, options);
	return;
    }
    strlcpy(debugger_cmd, file->lang->debugger_cmd, APE_CMD_MAX);

    /* Make sure build options exist */
    if ( check_build_opts(file) == NULL )
	return;
    
    /* Set up GDB start command if needed */
    if ( trace_cmd != NULL )
    {
	strlcpy(command_file, ".ape_cmd_file.XXXXX", APE_PATH_MAX);
	stdout_file = command_file;
	if ( (fd = mkstemp(command_file)) != -1 )
	{
	    if ( (fp = fdopen(fd, "w")) != NULL )
	    {
#ifdef SCO_SV  
		/* db[xX]tra assumes stdin is a tty and will fail */
		/* Fall back to dbx, which is redirectable */
		if ( stricmp(debugger_cmd, "dbxtra") == 0 )
		    strlcpy(debugger_cmd, "dbx", APE_PATH_MAX);
#endif
		fprintf(fp, "%s\n", trace_cmd);
		fclose(fp);
	    }
	    close(fd);
	}
    }
    
    /* Fixme: abstract out all these references to project->makefile */
    if ( project->makefile[0] != '\0')
	exe = project->executable;
    else
	exe = file->executable;
    expand_command(file->source, exe, debugger_cmd, cmd, APE_CMD_MAX);
    
    expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
    begin_full_screen();
    status = spawnvp(P_WAIT, P_ECHO, argv, stdout_file, NULL, NULL);
    check_stat(status, argv[0]);
    if ( *command_file != '\0' )
    {
	putchar('\n');
    }
    unlink(command_file);
    free(expanded_cmd);
    end_full_screen(EFS_PAUSE);
}


int     compile_prog(files, af, errfile, options, output)
file_t  files[];
int     af;
err_t   *errfile;
opt_t *options;
out_t   output;

{
    char    *argv[MAX_ARGS],
	    cmd[APE_CMD_MAX+4],
	    *expanded_cmd,
	    *ok_button[2] = OK_BUTTON;
    int     stat;

    if ( check_build_opts(files+af) == NULL )
	return 0;
    
    /* Give user a chance to save files or cancel */
    if (prompt_save_all(files, options) == 'c')
	return 0;
    files[af].lang_rebuild = 0;
    switch(output)
    {
	case    OBJECT:
	    snprintf(cmd, APE_CMD_MAX + 4, "%s %s %s %s", files[af].lang->compiler_cmd,
		files[af].lang->compile_only_flag,
		files[af].lang->compile_flags, files[af].source);
	    break;
	case    ASSEMBLY:
	    snprintf(cmd, APE_CMD_MAX + 4, "%s %s %s %s", files[af].lang->compiler_cmd,
		files[af].lang->compile_to_asm_flag,
		files[af].lang->compile_flags,
		files[af].source);
	    break;
	default:
	    popup_mesg("compile_prog(): Unknown output format.",
			ok_button, options);
	    break;
    }
    expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
    begin_full_screen();
    err_close(errfile);
    
    mkfifo(APE_FIFO, 0700);
    /*
    stat = spawnvp(P_WAIT, P_ECHO, argv, NULL, errfile->filename,
		errfile->filename);
    */
    spawnlp(P_NOWAIT, P_NOECHO, APE_FIFO, NULL, NULL,
	    "tee", errfile->filename, NULL);
    stat = spawnvp(P_WAIT, P_ECHO, argv, NULL, APE_FIFO, APE_FIFO);
    unlink(APE_FIFO);
    
    check_stat(stat, cmd);
    //more(errfile->filename);
    init_compiler_lines(files, options);
    end_full_screen(EFS_PAUSE);
    free(expanded_cmd);
    return P_EXIT_CODE(stat);
}


/***************************************************
 * Compile and run the program in the active window
 ***************************************************/

int     run_prog(files, af, project, errfile, options, flags)
file_t  files[];
int     af;
proj_t  *project;
err_t   *errfile;
opt_t *options;
int     flags;

{
    extern term_t   *Terminal;
    char    run_cmd[APE_CMD_MAX + 6] = "",
	    upload_cmd[APE_CMD_MAX + 6] = "";
    int     run_status,
	    build_status = 0,
	    upload_status = 0,
	    status = 0,
	    rebuild;

    rebuild = rebuild_needed(files, af, project);
    if ( rebuild || !compiled_language(files[af].lang) )
    {
	if (prompt_save_all(files, options) == 'c')
	    return 0;
    }
    
    /* Create run command */
    if ( project->makefile[0] != '\0' )
    {
	strlcpy(run_cmd, project->run_cmd, APE_CMD_MAX);
	if ( (check_build_opts(files+af) != NULL) &&
	    !strblank(files[af].lang->upload_prefix) )
	{
	    snprintf(upload_cmd, APE_CMD_MAX, "%s %s",
		files[af].lang->upload_prefix, project->executable);
	}
	if ( chdir(project->make_directory) == -1 )
	{
	    sprintw(2, TWC_ST_LEN, "Cannot chdir to %s",
		    project->make_directory);
	    tgetc(Terminal);
	}
    }
    else
    {
	/* Make sure build options exist */
	if ( check_build_opts(files+af) == NULL )
	    return 0;
	strlcpy(run_cmd, files[af].run_cmd, APE_CMD_MAX);
	if ( !strblank(files[af].lang->upload_prefix) )
	{
	    snprintf(upload_cmd, APE_CMD_MAX, "%s %s",
		files[af].lang->upload_prefix, files[af].executable);
	}
	if ( chdir(files[af].run_directory) == -1 )
	{
	    sprintw(2, TWC_ST_LEN, "Cannot chdir to %s",
		    files[af].run_directory);
	    tgetc(Terminal);
	}
    }

    begin_full_screen();

    /* Rebuild program if necessary */
    if ( rebuild )
    {
	build_status = build_it(project, files, af, errfile, options);
	status = build_status;
    }

    /* Upload the program if appropriate */
    if ( (build_status == 0) && 
	(strcmp(upload_cmd, "exec ") != 0) && !strblank(upload_cmd) )
    {
	upload_status = spawnlp(P_WAIT, P_ECHO, NULL, NULL, NULL,
			    options->shell, "-c", upload_cmd, NULL);
	status = upload_status;
    }
    
    /* Run the program */
    if ( (build_status == 0) && (upload_status == 0) && (flags != UPLOAD_PROG)
	&& (strcmp(run_cmd, "exec ") != 0) && !strblank(run_cmd) )
    {
	run_status = spawnlp(P_WAIT, P_ECHO, NULL, NULL, NULL,
			    options->shell, "-c", run_cmd, NULL);
	status = run_status;
    }

    fprintf(stderr, "Exit status = %d\n", status);
    end_full_screen(EFS_PAUSE);
    sprintw(2, TWC_ST_LEN, "Return code: 0x%x", status);
    
    /* Switch back to source directory */
    chdir(files[af].cwd);
    
    /* Return build status, not run status, to trigger error parsing */
    return P_EXIT_CODE(build_status);
}


int     rebuild_needed(files, af, project)
file_t  files[];
int     af;
proj_t  *project;

{
    if ( project->makefile[0] != '\0' )
    {
	return 1;
    }
    
    if (check_build_opts(files+af) == NULL)
    {
	return 0;
    }
    
    if ( compiled_language(files[af].lang) )
    {
	if ( files[af].modified || files[af].lang_rebuild ||
	    (xt_file_mod_cmp(files[af].source, files[af].executable) > 0) )
	    return 1;
	else
	    return 0;
    }
    else
	return 0;
}


int     build_it(project, files, af, errfile, options)
proj_t  *project;
file_t  files[];
int     af;
err_t   *errfile;
opt_t   *options;

{
    char    build_cmd[APE_CMD_MAX + 1] = "",
	    *expanded_cmd,
	    *argv[MAX_ARGS],
	    *outfile,
	    *executable,
	    *ok_button[2] = OK_BUTTON;
    struct stat statinfo;
    int     status;
    
    get_build_cmd(files+af, project, build_cmd, APE_CMD_MAX, &outfile);
    if ( *project->makefile == '\0' )
	executable = files[af].executable;
    else
	executable = project->executable;
    if ( *build_cmd != '\0' )
    {
	expanded_cmd = parse_cmd(argv, MAX_ARGS, build_cmd);
	err_close(errfile);
	files[af].lang_rebuild = 0;
	status = spawn_build_cmd(argv, outfile, errfile, project, executable);
	check_stat(status, build_cmd);
	stat(errfile->filename, &statinfo);
	if ( statinfo.st_size > 0 )
	{
	    more(errfile->filename);
	    init_compiler_lines(files, options);
	    if ( status == 0 )
	    {
		printf("** End of error list - press <return> to continue...");
		fflush(stdout);
		getchar();
	    }
	}
	free(expanded_cmd);
    }
    else
    {
	status = 1;
	popup_mesg(CANT_COMPILE_MSG, ok_button,  options);
    }
    return status;
}


/****************************************************
 * Compile and link the program in the active window 
 ****************************************************/

int     build(files, af, project, errfile, options)
file_t files[];
int     af;
proj_t  *project;
err_t   *errfile;
opt_t *options;

{
    char    cmd[APE_CMD_MAX + 1] = "",
	    *expanded_cmd,
	    *argv[MAX_ARGS],
	    *outfile,
	    *executable,
	    *ok_button[2] = OK_BUTTON;
    int     stat = 0;
    
    /* For single file programs, check mod times to avoid unnecessary build */
    if (*project->makefile == '\0')
    {
	if (xt_file_mod_cmp(files[af].source, files[af].executable) < 0)
	{
	    popup_mesg("\"%s\" is up to date.  Save file to force rebuild.",
		ok_button, options, files[af].executable);
	    /*sprintw(2, TWC_ST_LEN,
		"\"%s\" is up to date.  Save file to force rebuild.",
		files[af].executable);*/
	    return 0;
	}
	if ( check_build_opts(files+af) == NULL )
	    return CANT_BUILD;
	executable = files[af].executable;
    }
    else
	executable = project->executable;
    
    /* Allow user to save files or cancel build */
    if (prompt_save_all(files, options) == 'c')
	return 0;
    
    /* Mark build options unmodified since last build */
    files[af].lang_rebuild = 0;
    
    get_build_cmd(files+af, project, cmd, APE_CMD_MAX, &outfile);
    if ( *cmd != '\0' )
    {
	begin_full_screen();
	expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
	err_close(errfile);
	/*
	fprintf(stderr, "In build()...\n");
	fprintf(stderr, "%s\n", expanded_cmd);
	for (int c = 0; argv[c] != NULL; ++c)
	    fprintf(stderr, "%s\n", argv[c]);
	fprintf(stderr, "outfile = %p\n", outfile);
	*/
	stat = spawn_build_cmd(argv, outfile, errfile, project, executable);
	check_stat(stat, cmd);
	more(errfile->filename);
	init_compiler_lines(files, options);
	end_full_screen(EFS_PAUSE);
	free(expanded_cmd);
	//sprintw(2, 50, "build status = %d", stat);
	return P_EXIT_CODE(stat);
    }
    else
    {
	popup_mesg(CANT_COMPILE_MSG, ok_button, options);
	return 0;
    }
}


/****************************************************
 * Compile and link the program in the active window 
 ****************************************************/

int     clean(files, af, project, errfile, options)
file_t  files[];
int     af;
proj_t  *project;
err_t   *errfile;
opt_t *options;

{
    char    cmd[APE_CMD_MAX + 1] = "",
	    *expanded_cmd,
	    *argv[MAX_ARGS],
	    obj[APE_PATH_MAX+1],
	    *p,
	    *executable;
    int     stat = 0;
    
    /* For single file programs, check mod times to avoid unnecessary build */
    if (*project->makefile == '\0')
    {
	executable = files[af].executable;
	strlcpy(obj, files[af].source, APE_PATH_MAX);
	if ( (p = strrchr(obj, '.')) != NULL )
	    strlcpy(p, ".o", 3);
	snprintf(cmd, APE_CMD_MAX, "rm -f %s %s", executable, obj);
    }
    else
    {
	executable = project->executable;
	snprintf(cmd, APE_CMD_MAX, "%s -f %s clean",
		project->make_cmd, project->makefile);
    }
    
    begin_full_screen();
    expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
    err_close(errfile);
    stat = spawn_build_cmd(argv, NULL, errfile, project, executable);
    check_stat(stat, cmd);
    more(errfile->filename);
    init_compiler_lines(files, options);
    end_full_screen(EFS_PAUSE);
    free(expanded_cmd);
    return P_EXIT_CODE(stat);
}


/****************************************************************************
 * Description: 
 *  Run the make install target, if it exists
 *
 * History: 
 *  May, 2009.  Temporary measure until general make target submenu
 *  is added to build menu.
 *
 ***************************************************************************/

int     install(files, af, project, errfile, options)
file_t  files[];
int     af;
proj_t  *project;
err_t   *errfile;
opt_t *options;

{
    char    cmd[APE_CMD_MAX + 1] = "",
	    *expanded_cmd,
	    *argv[MAX_ARGS],
	    *executable;
    int     stat = 0;
    
    /* For single file programs, check mod times to avoid unnecessary build */
    if (*project->makefile == '\0')
    {
	sprintw(2, TWC_ST_LEN, "Sorry, no Makefile.");
	return 0;
    }
    else
    {
	executable = project->executable;
	snprintf(cmd, APE_CMD_MAX, "%s -f %s install",
		project->make_cmd, project->makefile);
    }
    
    prompt_save_all(files, options);
    
    begin_full_screen();
    expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
    err_close(errfile);
    stat = spawn_build_cmd(argv, NULL, NULL, project, executable);
    check_stat(stat, cmd);
    init_compiler_lines(files, options);
    end_full_screen(EFS_PAUSE);
    free(expanded_cmd);
    return P_EXIT_CODE(stat);
}


/*******************************************************
 * Determine how to build the file in the active window
 *******************************************************/

void    get_build_cmd(file_t *file, proj_t *project, char *cmd,
		      size_t cmd_max, char **outfile)

{
    *outfile = NULL;
    if (*project->makefile == '\0')
    {
	if ( !compiled_language(file->lang) )
	    *cmd = '\0';
	else if ( strcmp(file->lang->executable_spec, "Command Line Flag") == 0 )
	{
	    snprintf(cmd, APE_CMD_MAX, "%s %s %s %s%s %s",
		file->lang->compiler_cmd,
		file->lang->compile_flags, file->source, 
		file->lang->compile_output_flag,
		file->executable, file->lang->link_flags);
	}
	else if ( strcmp(file->lang->executable_spec, "Fixed") == 0 )
	{
	    snprintf(cmd, APE_CMD_MAX, "%s %s %s %s", file->lang->compiler_cmd,
		file->lang->compile_flags, file->source,
		file->lang->link_flags);
	}
	else if ( strcmp(file->lang->executable_spec, "Standard Output") == 0 )
	{
	    snprintf(cmd, APE_CMD_MAX, "%s %s %s %s", file->lang->compiler_cmd,
		file->lang->compile_flags, file->source,
		file->lang->link_flags);
	    *outfile = file->executable;
	}
    }
    else
	snprintf(cmd, APE_CMD_MAX, "%s -f %s %s", 
		project->make_cmd, project->makefile, project->make_args);
}


/*
 * Get name of makefile, executable, and execute command from user
 */
 
void    set_makefile(proj_t *project, char *makefile, file_t *file,
		    opt_t *options)

{
    win_t   *win;
    tw_panel_t panel = TWC_PANEL_INIT;
    int     nogood, status;
    char    *ok_button[2] = OK_BUTTON;
    
    win = centered_panel_win(13, 65, options);
    
    if ( *project->makefile == '\0' )
	init_makefile(project, makefile, file, options);
	
    tw_init_string(&panel, 2, 2, APE_CMD_MAX, 40, TWC_VERBATIM,
	"Make command:    ",
	    " Name of make command (make, gmake, etc.) ", project->make_cmd);
    tw_init_string(&panel, 3, 2, APE_PATH_MAX, 40, TWC_VERBATIM,
	"Makefile:        ",
	    " Enter name of makefile in the current directory. ", project->makefile);
    tw_init_string(&panel, 4, 2, APE_PATH_MAX, 40, TWC_VERBATIM,
	"Executable:      ",
	    " Name of executable produced by makefile. ", project->executable);
    tw_init_string(&panel, 5, 2, APE_PATH_MAX, 40, TWC_VERBATIM,
	"Directory:       ",
	    " Directory where makefile is located. ", project->make_directory);
    tw_init_string(&panel, 6, 2, OPTION_LEN, 40, TWC_VERBATIM,
	"Make Arguments:  ",
	    " Additional command line arguments to make command.",
	    project->make_args);
    tw_init_string(&panel, 7, 2, APE_CMD_MAX, 40, TWC_VERBATIM,
	"Run prefix:      ",
	    " Prefixed to executable when running program. ",
	    project->run_prefix);
    tw_init_string(&panel, 8, 2, OPTION_LEN, 40, TWC_VERBATIM,
	"Run Arguments:   ",
	    " Command line arguments to executable.",
	    project->run_args);
    do
    {
	status = tw_input_panel(win, &panel, TW_LINES(win)-2);
	nogood = ( (TW_EXIT_KEY(status) == TWC_INPUT_DONE)
	    && (*project->makefile != '\0')
	    && (*project->executable == '\0') );
	if ( nogood )
	{
	    popup_mesg("You must enter an executable name here or assign\n"
			"it to the variable BIN or BIN1 in the makefile.",
		    ok_button, options);
	    TW_RESTORE_WIN(win);
	}
    }   while ( nogood );
    
    /* Reset run command */
    set_project_run_cmd(project,file);
    tw_del_win(&win);
}


void    init_makefile(proj_t *project,char *makefile,file_t *file,opt_t *options)

{
    strlcpy(project->make_cmd, "make", MAKE_CMD_MAX);
    strlcpy(project->makefile, makefile, APE_PATH_MAX);
    
    /*
     *  Attempt to parse makefile for executable name, etc.
     *  Recursively parse included Makefiles.
     */
    *project->executable = '\0';
    project->make_vars = 0;
    parse_makefile(project, project->makefile, options);
    
    getcwd(project->make_directory,APE_PATH_MAX);
    strlcpy(project->run_prefix,"time",APE_PATH_MAX);
    set_makefile(project,"makefile",file,options);
}


void    set_project_run_cmd(proj_t *project,file_t *file)

{
    /*if ( ((p=strrchr(project->executable,'.')) != NULL) &&
	 (strcmp(p,".a") == 0) )
	    snprintf(project->run_cmd,APE_CMD_MAX,"ranlib %s",project->executable);
    else if ( check_build_opts(file) != NULL )
	snprintf(project->run_cmd,APE_CMD_MAX,"%s ./%s",
		file->lang->run_prefix,project->executable);
    else
    */
	snprintf(project->run_cmd,APE_CMD_MAX,"%s ./%s %s",
	    project->run_prefix, project->executable, project->run_args);
}


int     syntax_check(files,af,options,errfile)
file_t  files[];
int     af;
opt_t *options;
err_t   *errfile;

{
    char    cmd[APE_CMD_MAX+1],
	    *expanded_cmd,
	    *argv[MAX_ARGS];
    int     status = 0;
    
    /* Make sure build options exist */
    if ( check_build_opts(files+af) == NULL )
	return 0;
    
    /* Give user chance to save files or cancel */
    if (prompt_save_all(files,options) != 'c')
    {
	snprintf(cmd,APE_CMD_MAX, "%s %s %s %s", files[af].lang->compiler_cmd,
		files[af].lang->syntax_check_flag,
		files[af].lang->compile_flags, files[af].source);
	expanded_cmd = parse_cmd(argv, MAX_ARGS, cmd);
	err_close(errfile);
	begin_full_screen();
	status = spawnvp(P_WAIT,P_ECHO,argv, NULL, errfile->filename, errfile->filename);
	more(errfile->filename);
	init_compiler_lines(files,options);
	end_full_screen(EFS_PAUSE);
	free(expanded_cmd);
    }
    return P_EXIT_CODE(status);
}


int     preprocess(file_t files[], int af, opt_t *options)

{
    char    cmd[APE_CMD_MAX+1] = "",
	    cmd2[APE_CMD_MAX+1] = "";
    int     status;
    
    /* Make sure build options exist */
    if ( check_build_opts(files+af) != NULL )
    {
	if ( prompt_save_all(files,options) == 'c')
	    return 0;
	snprintf(cmd,APE_CMD_MAX,"%s %s %s %s | uniq | more",    /* more -s? */
	    files[af].lang->compiler_cmd, PP_OPTIONS,
	    files[af].lang->compile_flags, files[af].source);
	strshellcpy(cmd2,cmd,APE_CMD_MAX);
	status = run_command(P_WAIT,P_ECHO,cmd2,"sh");
	init_compiler_lines(files,options);
	return status;
    }
    else
	return 0;
}


int     spawn_build_cmd(char *argv[], char *outfile, err_t *errfile,
			proj_t *project, char *executable)

{
    int     status,exe_status;
    char    save_dir[APE_PATH_MAX+1];
    struct stat stats;
    
    /*
    fprintf(stderr, "Spawning command...\n");
    for (int c = 0; argv[c] != NULL; ++c)
	fprintf(stderr, "%s\n", argv[c]);
    fprintf(stderr, "outfile = %p  errfile = %s  executable = %s\n",
	    outfile, errfile->filename, executable);
    */
    
    if ( *project->makefile != '\0' )
    {
	getcwd(save_dir,APE_PATH_MAX);
	chdir(project->make_directory);
    }
    exe_status = stat(executable,&stats);
    /* If compiler does not output code to stdout, redirect both
       stdout and stderr to error messages file */
    if ( outfile == NULL )
	outfile = errfile->filename;
    
    status = spawnvp(P_WAIT,P_ECHO,argv, NULL, outfile, errfile->filename);

    /* FIXME: Kills output from subprocesses, e.g. vexctl.  Find another way...
    mkfifo(APE_FIFO, 0700);
    printf("tee %s\n", errfile->filename);
    spawnlp(P_NOWAIT, P_NOECHO, APE_FIFO, NULL, NULL,
	"tee", errfile->filename, NULL);
    status = spawnvp(P_WAIT, P_ECHO, argv, NULL, APE_FIFO, APE_FIFO);
    
    unlink(APE_FIFO);
    */
    
    if ( exe_status == 0 )
    {
	stats.st_mode |= 0100;  /* Make sure it's owner-executable */
	chmod(executable,stats.st_mode);
    }
    if ( *project->makefile != '\0' )
	chdir(save_dir);
    return status;
}


void    profile(proj_t *project,file_t files[],int af,opt_t *options)

{
    char    cmd[APE_CMD_MAX+1],
	    mesg[MESG_LEN+1],
	    *prog;
    struct stat st;
    char    *ok_button[2] = OK_BUTTON;

    if ( stat(PROF_OUT,&st) != 0 )
    {
	snprintf(mesg,MESG_LEN, "No %s file found.\nCompile with '%s' and run the program first.",
		PROF_OUT,PROF_OPT);
	popup_mesg(mesg,ok_button,options);
    }
    else
    {
	if ( project->makefile[0] == '\0' )
	    prog = files[af].executable;
	else
	    prog = project->executable;
	snprintf(cmd,APE_CMD_MAX,"%s %s | more",PROF_CMD,prog);
	run_command(P_WAIT,P_ECHO,cmd,"sh");
    }
}


/**********************************************************************
  Description:
    Determine if a non-empty error file exists.  This is useful
    immediately following a compile command.
  Return values:
    Nonzero (true) if errors, 0 (false) if not.
  Bugs:
    Doesn't handle compilers that spew informative messages even though
    the compile succeeded.  (e.g. f2c)
 **********************************************************************/

int     errors(int status, err_t *errfile)

{
    struct stat st;
    
    if ( stat(errfile->filename,&st) == 0 )
    {
	return status && st.st_size;
    }
    else
	return status;
}


void    skip_space(char **p)

{
    while ( isspace(**p) )
	++*p;
}


void    skip_nonspace(char **p)
{
    while ( !isspace(**p) )
	++*p;
}


/****************************************************************************
 * Description: 
 *  Resolve a Makefile variable reference and return a pointer to the value.
 *
 * History: 
 *  May, 2009       J Bacon
 *
 ***************************************************************************/

char    *resolve_make_var(proj_t *project, char *var, char **endp)

{
    char    *end;
    int     c;
    
    /* Skip over $, {, etc. */
    while ( !(isalnum(*var) || (*var == '_')) )
	++var;
    
    /* Find end of var name */
    for (end = var; isalnum(*end) || (*end == '_'); ++end)
	;
    *end = '\0';
    *endp = end + 1;
    
    for (c=0; c<project->make_vars; ++c)
	if ( strcmp(project->make_var_names[c], var) == 0 )
	    return project->make_var_values[c];
    return var;
}


/****************************************************************************
 * Description: 
 *  Parse the value portion of a Makefile variable def, and return
 *  a fully resolved string.  The returned string is a local static
 *  so it should be copied to a safe location before calling this
 *  function again.
 *
 * History: 
 *  May, 2009   J Bacon
 *
 ***************************************************************************/

char    *parse_value(proj_t *project, char *p)
{
    static char val[MAKEFILE_LINE_MAX+1];
    char        *v,
		*v2;
    
    v = val;
    while ( *p != '\0' )
    {
	while ( isspace(*p) )
	    ++p;
	while ( (*p != '$') && !isspace(*p) && (*p != '\0') )
	    *v++ = *p++;
	if ( *p == '$' )
	{
	    v2 = resolve_make_var(project, p, &p);
	    while ( *v2 != '\0' )
		*v++ = *v2++;
	}
    }
    *v = '\0';
    return val;
}


/****************************************************************************
 * Description: 
 *  Parse a Makefile variable definition from line, and add the new
 *  variable to the project.
 *
 * History: 
 *  May, 2009       J Bacon
 *
 ***************************************************************************/

void    add_make_var(proj_t *project, char *ident, char *p)

{
    char    *val;
    
    skip_space(&p);     /* Space after operator */

    /* Parse out value */
    val = parse_value(project, p);

    project->make_var_names[project->make_vars] = strdup(ident);
    project->make_var_values[project->make_vars] = strdup(val);
    ++project->make_vars;

    /* First qualifying var is used as the executable */
    if ( *project->executable == '\0' )
    {
	/* Attempt to identify the main target of the Makefile */
	if ( strcmp(ident, "LIB1") == 0 )
	    strlcpy(project->executable, val, APE_PATH_MAX);
	if ( strcmp(ident, "LIB") == 0 )
	    strlcpy(project->executable, val, APE_PATH_MAX);
	if ( strcmp(ident, "BIN1") == 0 )
	    strlcpy(project->executable, val, APE_PATH_MAX);
	if ( strcmp(ident, "BIN") == 0 )
	    strlcpy(project->executable, val, APE_PATH_MAX);
    }
}


void    makefile_get_ident(char *line, char *ident, char **endp)

{
    int     c = 0;
    
    while ( (c < MAKEFILE_TOKEN_MAX) && (isalnum(line[c]) || (line[c] == '_')) )
    {
	ident[c] = line[c];
	++c;
    }
    ident[c] = '\0';
    *endp = line + c;
}


/****************************************************************************
 * Description: 
 *  Scan Makefile for variables and targets, and store them in the
 *  project structure.  Resolve variable references in the value
 *  of new variables.
 *
 * History: 
 *  May, 2009       J Bacon
 *
 ***************************************************************************/

void    parse_makefile(proj_t *project, char *makefile, opt_t *options)

{
    FILE    *fp;
    char    line[MAKEFILE_LINE_MAX+1],
	    *p,
	    ident[MAKEFILE_TOKEN_MAX+1],
	    *filename,
	    next_token[MAKEFILE_TOKEN_MAX+1],
	    *p2,
	    *ok_button[2] = OK_BUTTON;
    
    fp = fopen(makefile, "r");
    if ( fp == NULL )
    {
	sprintw(2, TWC_ST_LEN, "Error: Cannot open %s.", makefile);
	return;
    } 
    while ( fgets(line, MAKEFILE_LINE_MAX, fp) != NULL )
    {
	makefile_get_ident(line, ident, &p);
	
	/* Process includes, variable defs and targets.  Ignore the rest. */
	if ( *ident != '\0' )
	{
	    //sprintw(2,50,"Parsed %s", ident);
	    //tgetc(Terminal);
	    skip_space(&p);
	    
	    if ( (strcmp(ident, "include") == 0) ||
		 (strcmp(ident, ".include") == 0) )
	    {
		/* Recursively parse included makefile */
		filename = parse_value(project, p);
		parse_makefile(project, filename, options);
	    }
	    
	    /* Get next token after variable/target/keyword. */
	    for (p2 = next_token; ispunct(*p); ++p, ++p2 )
		*p2 = *p;
	    *p2 = '\0';
	    
	    /* If next token contains a '=', this is a variable assignment. */
	    if ( strchr(next_token,'=') != NULL )
	    {
		/*
		 *  If variable BIN exists in the Makefile, use it as the project
		 *  executable.  BIN1 is a second choice.
		 */
		add_make_var(project, ident, p);
		if ( project->make_vars == MAKEFILE_MAX_VARS )
		{
		    popup_mesg("Maximum makefile variables reached.", ok_button, options);
		    break;
		}
	    }
	}
    }
    fclose(fp);
}


int     compiled_language(lang_t *lang)

{
    return ( (lang != NULL) && (*lang->compiler_cmd != '\0') );
}

