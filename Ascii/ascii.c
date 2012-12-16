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
#include <fcntl.h>
#include <stdlib.h>
#include "twintk_term.h"
#include "ascii.h"
#include "protos.h"

int     main()

{
    term_t  *term;
    
    term = init_term(stdin,stdout,stderr,NULL,0);
    traw_mode(term);

    pause(term,NORMAL);
    show_chars(term,NORMAL);
    if ( THAS_ACS(term) )
    {
	pause(term,ALT);
	show_chars(term,ALT);
    }
    tset_startup_tty(term);
    tset_modes(term,NORMAL_MODE);
    return 0;
}


void    show_chars(term,mode)
term_t  *term;
charmode_t  mode;

{
    unsigned int     c,i;
    unsigned char    ch;
    char    *names[256] = { "NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL",
			"BS","TAB","LF","VT","FF","CR","SO","SI","DLE",
			"DC1","DC2","DC3","DC4","NAK","SYN","ETB","CAN",
			"EM","SUB","ESC","FS","GS","RS","US" };

    names[127]="DEL";
    //if ( memcmp(term->term_type,"vt",2) == 0 ) Why?
    names[132]=names[133]=names[144]=" ";

    for (i=0; i<4; ++i)
	tprintf(term,"Oct Dec Hex Char   ");
    tputs(term,"\n\r");
    
    for (c=0; c<64; ++c)
    {
	for (i=0; i<256; i+=64)
	{
	    ch = c + i;
	    if ( (ch < 32) || (ch == 127) || (ch == 132) || (ch == 133) || (ch == 144) )
		tprintf(term,"%03o %3u %02X  %-3s    ",ch,ch,ch,names[ch]);
	    else
	    switch(mode)
	    {
		case    NORMAL:
		    tprintf(term,"%03o %3u %02X  '%c'    ",ch,ch,ch,ch);
		    break;
		case    ALT:
		    tprintf(term,"%03o %3u %02X  '",ch,ch,ch);
		    tset_modes(term,ACS_MODE);
		    tputc(term,ch);
		    tset_modes(term,NORMAL_MODE);
		    tprintf(term,"'    ");
		    break;
		default:
		    break;
	    }
	}
	tputs(term,"\n\r");
	if ( (c+1) % 23 == 0 )
	    pause(term,mode);
    }
    pause(term,mode);
}


void    pause(term,mode)
term_t  *term;
int     mode;

{
    int     ch;
    char    seq[20],
	    *msg[2]={"Standard character set", "Alternate character set"};
    
    tprintf(term,"%s: *** Press any key to continue ***\r",msg[mode]);
    TFLUSH_OUT(term);
    ch = tgetseq(term,seq,19);
    if ( (ch == 'q') || (ch == 'Q') )
    {
	tset_modes(term,NORMAL_MODE);
	exit(0);
    }
}

