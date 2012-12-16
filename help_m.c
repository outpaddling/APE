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
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "twintk.h"
#include "bacon.h"
#include "edit.h"
#include "protos.h"


int     help_menu(file_t *file, opt_t *options, event_t *event)

{
    extern term_t   *Terminal;
    int     ch, start_row = 1;
    win_t *help_pop;
    static char cmd[CMD_LEN + 1] = "",
		topic[CMD_LEN + 1];
    static char *help_text[] = {
				"A.Bout APE",
				TWC_HLINE,
				".Help on APE",
				".Keyboard",
				".ASCII table",
				TWC_HLINE,
				".Unix command or function",
				".Word at cursor (F1 or Esc-?)",
				".Search manuals for topic",
				"GNU .Info",
				""};

    help_pop = tw_menu(Terminal, 1, -33, help_text, &options->border,
		     options->no_acs, MONO_MODE(options),
		     options->menu_fg, options->menu_bg,
		     options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr(help_pop,REVERSE_MODE,options->menu_fg,options->menu_bg,
		    BOLD_MODE,options->menu_hl_fg,options->menu_hl_bg);
    ch = tw_get_item(help_pop, help_text, event, &start_row,
		    options->reverse_menu,NULL);
    
    switch(ch)
    {
	case 'k':
	    if ( options->use_html )
		browse("apekeys.html",options);
	    else
		man("apekeys",INSTALL_PREFIX);
	    break;
	case 'w':
	    get_word_at_cursor(file, cmd);
	    man(cmd, NULL);
	    break;
	case 'u':
	    panel_get_string(file, options, CMD_LEN, 
			    "Command or function? ", "", cmd);
	    man(cmd, NULL);
	    break;
	case 's':
	    panel_get_string(file, options, CMD_LEN, 
			    "Topic? ", "", topic);
	    apropos(topic);
	    break;
	case 'b':
	    _reg(options);
	    break;
	case 'h':
	    if ( options->use_html )
		browse("ape.html",options);
	    else
		man("ape",INSTALL_PREFIX);
	    break;
	case 'a':
	    begin_full_screen();
	    /* Don't use man() here.  We need to stay shelled out for the ascii command. */
	    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"man","-M",INSTALL_PREFIX"/man","apeascii",NULL);
	    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"ascii",NULL);
	    end_full_screen(EFS_PAUSE);
	    break;
	case 'i':
	    begin_full_screen();
	    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"info",NULL);
	    end_full_screen(EFS_PAUSE);
	    break;
	default:
	    break;
    }
    tw_del_win(&help_pop);
    TW_RESTORE_WIN(file->window);
    return(ch);
}


void    man(char *str, char *prefix)

{
    char    cmd[CMD_LEN+1],*argv[MAX_ARGS];
    
    if ( *str != '\0' )
    {
	if ( prefix == NULL )
	    snprintf(cmd,CMD_LEN,"%s %s",MAN,str);
	else
	    snprintf(cmd,CMD_LEN,"%s -M %s/man %s",MAN,prefix,str);
	parse_cmd(argv,cmd);
	begin_full_screen();
	spawnvp(P_WAIT,P_NOECHO,argv,NULL,NULL,NULL);
	end_full_screen(EFS_PAUSE);
    }
}


void    get_word_at_cursor(file,word)
file_t  *file;
char    *word;

{
    char    *ptr = file->curchar, *start = file->line[file->curline].buff;
    
    while ( (ISIDENT(*ptr)) && (ptr>=start) )
	--ptr;
    ++ptr;
    while ( ISIDENT(*ptr) )
	*word++ = *ptr++;
    *word = '\0';
}


void    apropos(topic)
char    *topic;

{
    if ( *topic != '\0' )
    {
	begin_full_screen();
	spawnlp(P_WAIT,P_ECHO,NULL,NULL,NULL,"apropos",topic,NULL);
	end_full_screen(EFS_PAUSE);
    }
}


void    browse(file,options)
char    *file;
opt_t   *options;

{
    int     stat,
	    fd;
    char    path[PATH_LEN+1],
	    errors[PATH_LEN+1];
    
    strlcpy(errors,".ape_browser_errors.XXXXX",PATH_LEN);
    if ( (fd=mkstemp(errors)) != -1 )
	close(fd);
    snprintf(path,PATH_LEN,"%s/share/doc/ape/%s",options->install_prefix,
	    file);
    stat = spawnlp(P_NOWAIT,P_NOECHO,NULL,errors,errors,
		    options->browser,path,NULL);
    check_stat(stat,options->browser);
    unlink(errors);
}

