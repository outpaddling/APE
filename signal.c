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

#include <sys/types.h>
#include <sys/wait.h>
#include <twintk.h>
#include "edit.h"
#include "protos.h"

err_t   *Errfile;
pid_t   Pid;
file_t  *Files;
extern opt_t   *Options;

 /* 
  * Die with honor
  */

void    kamakaze()

{
    extern term_t   *Terminal;
    
    tset_modes(Terminal, NORMAL_MODE);
    tset_foreground(Terminal, WHITE);
    tset_background(Terminal, BLACK);
    TFLUSH_OUT(Terminal);
    tpop_tty(Terminal);
    nice_exit(1, Errfile, "APE killed by external SIGTERM signal.");
}


 /* 
  * Print notification that a background job has ended
  */

void    notify()

{
    int     pid, status;
    extern win_t    *Swin;

    pid = wait(&status);
    if (pid == Pid)
    {
	stat_mesg("Background job ended. Type Ctrl+a.");
	TW_FLUSH_OUT(Swin);
	while (tw_getc(Swin) != '\001')
	    ;
	stat_mesg("Thanks for using notify()...");
	Pid = 0;
    }
}


void    restore_help()

{
    stat_mesg(BAR_HELP);
}


void    set_globals_for_signal(file_t files[], opt_t *options)

{
    Files = files;
    Options = options;
}

