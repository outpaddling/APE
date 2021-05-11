/*
    Copyright (c) 1994-2003, Jason W. Bacon, Acadix Software Systems
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
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <xtend.h>
#include <direct.h>

int     main(argc,argv)
int     argc;
char    *argv[];

{
    term_t  *term;
    
    term = init_term(stdin,stdout,stderr,NULL,0);
    switch(argc)
    {
	case    2:
	    /* Run more command */
	    spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"more",argv[1],NULL);
	    
	    /* Pause after quitting more */
	    tset_modes(term,HIGHLIGHT_MODE);
	    tpush_tty(term);
	    traw_mode(term);
	    printf("End of %s: Press any key to continue...",argv[1]);
	    fflush(stdout);
	    tset_modes(term,NORMAL_MODE);
	    fflush(stdout);
	    getchar();
	    putchar('\n');  /* Start next program on a new line */
	    tpop_tty(term); /* Restore original mode */
	    break;
	default:
	    usage("%s <file>\n",argv[0]);
    }
    return 0;
}

