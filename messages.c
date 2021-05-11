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
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <twintk.h>
#include <xtend.h>
#include "edit.h"
#include "protos.h"

 /* 
  * Display a message for an invalid keystroke
  */

void    invalid_key(key, key_seq)
int     key;
char    key_seq[];

{
    extern win_t    *Swin;
    
    sprintw(2, TWC_ST_LEN, "Invalid key sequence: %d: ", key);
    print_seq(Swin, key_seq);
}


/*
 * Print a character sequence with non-printable chars shown as octal escape sequences
 */
 
void    print_seq(win, key_seq)
win_t  *win;
char   *key_seq;

{
    char   *p;

    for (p = key_seq; *p != '\0'; ++p)
    {
	if (isgraph(*p))
	    tw_putc(win, *p);
	else
	if (*p == 033)
	    tw_printf(win, "<Esc>-");
	else
	if ((*p >= 0) && (*p <= 26))
	    tw_printf(win, "Ctrl+%c", *p + -1 + 'a');
	else
	    tw_printf(win, "\\%03o", (unsigned) *p % 0xff);
    }
}


/*
 * Pause if line noise is detected until user responds with a special
 * key sequence
 */

/*
void    trap_noise()

{
    int     c;

    stat_mesg( "Line noise detected: Press <Ctrl>-q 3 times.");
    for (c = 0; c < 3;)
	if (tgetc(Terminal) == RESUME)
	    ++c;
}
*/

/*
 * Display current editing mode
 */
 
void    display_mode(file)
file_t *file;

{
    extern win_t    *Swin;
    
    if (file->insert)
	sprintw(TW_COLS(Swin) - 22, 9, "<insert>");
    else
	sprintw(TW_COLS(Swin) - 22, 9, "<replace>");
}


void    show_cursor_pos(options,file)
opt_t   *options;
file_t  *file;

{
    extern win_t    *Swin;
    static int  oldline = -1, len, old_len=0;
    char    num_str[20];
    
    if (options->show_column)
	snprintf(num_str,19,"%lu %d",(unsigned long)(file->curline+1),
		ACTUAL_COL(file)+1);
    else if (file->curline != oldline)
	snprintf(num_str,19,"%lu", (unsigned long)(file->curline + 1));
    else
	return;
    len = strlen(num_str);
    oldline = file->curline;
    
    /* Display numbers */
    tw_move_to(Swin,0,TW_COLS(Swin)-12);
    tw_puts(Swin,num_str);
    
    /* Blank out garbage left by previous larger numbers */
    if ( old_len < len )
	old_len = len;
    while ( old_len > len )
    {
	tw_putc(Swin,' ');
	--old_len;
    }
}


int     popup_mesg(char *format,char *buttons[],opt_t *options,...)

{
    extern term_t   *Terminal;
    char    *msg2[20], temp[1024];
    size_t  c;
    int     status;
    va_list ap;
    
    va_start(ap, options);
    vsnprintf(temp, 1023, format, ap);
    
    msg2[0] = strtok(temp,"\n");
    for (c=1; (msg2[c] = strtok(NULL,"\n")) != NULL; ++c)
	;
    status = tw_response_win(Terminal, TWC_CENTER_WIN, TWC_CENTER_WIN,
			msg2, buttons,
			NOCOLOR(options)|NOACS(options),&options->border);
    va_end(ap);
    return status;
}

