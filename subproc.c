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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <xtend/proc.h>
#include <xtend/file.h>
#include "edit.h"
#include "protos.h"

int     Shelled_out;

/****************************************************************
 * Restore the terminal screen for execution of another program.
 * ( Prepare to shell out )
 ****************************************************************/

void 
begin_full_screen (void)

{
    extern term_t   *Terminal;

    Shelled_out = TRUE;
    
    /* Don't accept mouse motion as an end to the pause */
    tpush_event_mask(Terminal);
    TSELECT_INPUT(Terminal,KEY_PRESS|BUTTON1_PRESS);
    
    /* Shut off key_mouse reporting for child process */
#if 0
    if ( Terminal->windowid != -1 )
	TERMINFO_MOUSE_OFF(Terminal);
#endif

    /* Hide console mouse cursor during shell-out */
    TMOUSE_POINTER_INVISIBLE(Terminal);
    
    /* Save edit tty mode */
    tpush_tty(Terminal);
    
    /* Restore startup tty mode */
    tset_startup_tty(Terminal);
    
    /* Put terminal in normal video mode */
    tset_modes(Terminal,NORMAL_MODE);
    tset_foreground(Terminal,WHITE);
    tset_background(Terminal,BLACK);
    tclear_screen(Terminal);
    TFLUSH_OUT(Terminal);
}


/*************************************************************
 * Restore the terminal screen to edit mode after a shell out
 *************************************************************/

void 
end_full_screen (int pause)

{
    extern term_t   *Terminal;
    extern win_t    *Swin, *Bar_win;
    event_t event;
    
    /* Restore xterm mouse */
    if ( Terminal->windowid != -1 )
    {
#if 0
	define_cursor(XC_left_ptr);
	TERMINFO_MOUSE_ON(Terminal,NORMAL_MOUSE);
#endif
    }
    
    if ( pause )
    {
	printf("APE: Press return to continue...");
	fflush(stdout);
	/* Purge leftover input chars not read by child process */
	fpurge(stdin);

	tpop_tty(Terminal);
	/* Ignore button-down repeats */
	//do
	while ( tgetevent(Terminal,&event) != KEY_ENTER )
	    ;
	//while ( event.repeat > 0 );
    }
    else
	tpop_tty(Terminal);
    
    if ( Terminal->windowid == -1 )
	TMOUSE_POINTER_VISIBLE(Terminal);

    /* Restore console mouse */
    tpop_event_mask(Terminal);

    /* Restore screen */
    tset_foreground(Terminal,WHITE);
    tset_background(Terminal,BLACK);
    tset_modes(Terminal,NORMAL_MODE);
    tclear_screen(Terminal);
    
    /*
     *  If terminal was resized while shelled out, the signal handler would
     *  have held off on recreating the APE windows and redrawing them.
     *  The resize signal is not received by APE in some situations,
     *  depending on the terminal emulator, OS, etc.  Hence, we always
     *  resize upon return from a subprocess for good measure.
     */
    
    Shelled_out = FALSE;    /* Win resize aborts if Shelled_out is TRUE */
    win_resize();
    
    TW_RESTORE_WIN(Swin);
    tmove_to(Terminal,TLINES(Terminal)-1,TCOLS(Terminal)-2);
    tinsert_ch(Terminal,' ');
    TW_RESTORE_WIN(Bar_win);
    TFLUSH_OUT(Terminal);
    
    /* Empty garbage mouse input that occurred during shell-out */
    if ( MOUSE_FD(Terminal->mouse) != -1 )
	xt_fd_purge(MOUSE_FD(Terminal->mouse));
}


void 
more (char *filename)

{
    struct stat st;
    
    if ( stat(filename,&st) != -1 )
	if ( (int)st.st_size )
	    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"more",filename,NULL);
}


/**************************************************************************
 * Shell out - run any command in cooked mode starting with a clear screen
 **************************************************************************/

int 
run_command (int parent_action, int echo, char *cmd, char *shell)

{
    extern pid_t    Pid;
    int     stat;
    
    begin_full_screen();
    
    /* Run the command */
    stat = spawnlp(parent_action,echo,NULL,NULL,NULL,shell,"-c",cmd,NULL);
    if ( parent_action == P_NOWAIT )
	Pid = stat;     /* For notification when child terminates */
    check_stat(stat,cmd);
    end_full_screen(EFS_PAUSE);
    return (stat);
}


/****************************************************************************
 * Name:
 *  Quick check the return status from spawnlp() or spawnvp().
 *
 * Description: 
 *  This function performs a quick check on the return value from
 *  spawnlp(3) or spawnvp(3), printing error messages to stderr
 *  for common failures, and the raw exit status for others.
 * 
 * Author: 
 *  Jason W. Bacon
 *
 * Returns: 
 *  The value of stat, unaltered.  This allows compact usage such as
 *  if ( check_stat(spawnvp(...),cmd) )
 ****************************************************************************/


int     check_stat(
	    int     stat,   /* Return value from spawn*p() */
	    char    *cmd    /* Used in error message if stat shows failure */
	)

{
    if ( P_EXEC_FAILED(stat) )
    {
	switch( (stat>>8) & 0x7f )  /* Extract errno returned by spawnvp */
	{
	    case    EPERM:
	    case    EACCES:
		fprintf(stderr,"%s: Permission denied.\n",cmd);
		break;
	    case    ENOENT:
		fprintf(stderr,"%s: Command not found.\n",cmd);
		break;
	    default:
		fprintf(stderr,"Command failed with status %d.\n",stat);
	}
    }
    return stat;
}

