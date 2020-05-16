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

#include <twintk.h>
#include "edit.h"
#include "protos.h"

char    __Syms[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0x02,0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0x02,0,0x01,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0x02,0,0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};

void    match_group(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    char    *p = file->curchar;
    int     curline = file->curline, level=0, done=0, ochar, cchar,
	    lastline, lastcol, found=0;
    
    /* Don't bother with marked areas */
    if ( area_started(cut_buff) && cursor_in_area(file,cut_buff) )
	return;
    
    /*  Look for any closing group char */
    p = file->curchar;
    if ( ISCLOSING((int)*p) && TOKEN_MATCH(p,*p) )
    {
	cchar = *p;
	ochar = MATCHING_OPEN_TWC_CHAR(*p);
	while ( !done )
	{
	    if ( TOKEN_MATCH(p,cchar) )
		++level;
	    else if ( TOKEN_MATCH(p,ochar) )
		--level;
	    done = ( (found = (TOKEN_MATCH(p,ochar) && (level==0))) ||
		    (p==file->line[file->topline].buff) );
	    if ( !done )
	    {
		if ( p == file->line[curline].buff )
		{
		    --curline;
		    p = file->line[curline].buff + file->line[curline].length;
		}
		else
		    --p;
	    }
	}
	if ( found )
	    highlight_char(file,p,curline,options,cut_buff);
    }
    else if ( ISOPENING((int)*p) && TOKEN_MATCH(p,*p) )
    {
	ochar = *p;
	cchar = MATCHING_CLOSE_TWC_CHAR(*p);
	lastline = MIN(file->topline+TW_LINES(file->text)-1,file->total_lines-1);
	lastcol = MIN(file->line[lastline].length,
		      file->leftcol+TW_COLS(file->text)-1);
	while ( !done )
	{
	    if ( TOKEN_MATCH(p,ochar) )
		++level;
	    else if ( TOKEN_MATCH(p,cchar) )
		--level;
	    done = (p==file->line[lastline].buff+lastcol) ||
		    ( found = (TOKEN_MATCH(p,cchar) && (level==0)) );
	    if ( !done )
	    {
		if (*p == '\0')
		    p = file->line[++curline].buff;
		else
		    ++p;
	    }
	}
	if ( found )
	    highlight_char(file,p,curline,options,cut_buff);
    }
}


void    set_group_standout(file_t *file,opt_t *options)

{
    int     mode;
    win_t   *win = file->text;
    
    /* Leave this in place in case NCV problems show up in the future */
    if ( TCOLOR_TERM(win->terminal) && !MONO_MODE(options) &&
	 TNCV(win->terminal,NCV_BOLD) )
    {
	TW_SET_MODES(win,STANDOUT_MODE);
    }
    else
    {
	if ( THAS_BOLD_MODE(win->terminal) &&
	    !TNCV(win->terminal,NCV_BOLD) )
	    mode = BOLD_MODE;
	else if ( THAS_STANDOUT_MODE(win->terminal) &&
	    !TNCV(win->terminal,NCV_STANDOUT) )
	    mode = STANDOUT_MODE;
	else if ( THAS_REVERSE_MODE(win->terminal) &&
	    !TNCV(win->terminal,NCV_REVERSE) )
	    mode = REVERSE_MODE;
	else if ( THAS_DIM_MODE(win->terminal) &&
	    !TNCV(win->terminal,NCV_DIM) )
	    mode = DIM_MODE;
	else
	    mode = HIGHLIGHT_MODE;
	TW_SET_MODES(win,mode);
    }
}


void    unmatch_group(file,options,cut_buff)
file_t  *file;
opt_t   *options;
buff_t  *cut_buff;

{
    /* Erase old highlighting */
    if ( file->high_line != -1 )
    {
	/* Restore orignal color of character that was highlighted */
	TW_SET_MODES(file->text,file->high_modes);
	if ( TCOLOR_TERM(file->text->terminal) && !MONO_MODE(options) )
	{
	    tw_set_foreground(file->text,file->high_fg);
	    tw_set_background(file->text,file->high_bg);
	}
	
	/* Redraw the character */
	rewrite(file,options,cut_buff,file->high_line,file->high_col,
		file->high_line,file->high_col+1);
	TW_SET_MODES(file->text,NORMAL_MODE);
	if ( TCOLOR_TERM(file->text->terminal) && !MONO_MODE(options) )
	{
	    tw_set_foreground(file->text,options->text_fg);
	    tw_set_background(file->text,options->text_bg);
	}
	file->high_line = -1;
    }
}


void    highlight_char(file,p,line,options,cut_buff)
file_t  *file;
char    *p;
size_t  line;
opt_t   *options;
buff_t  *cut_buff;

{
    int     win_line, win_col;
    
    /* Don't highlight group char in marked area */
    if ( in_marked_area(file,cut_buff,line,(buff_len_t)(p-file->line[line].buff)) )
	return;
    
    file->high_line = line;
    file->high_col = p-file->line[line].buff;
    
    win_line = file->high_line - file->topline;
    win_col = file->high_col - file->leftcol;
    file->high_fg = TW_FOREGROUND(file->text,win_line,win_col);
    file->high_bg = TW_BACKGROUND(file->text,win_line,win_col);
    file->high_modes = TW_MODES(file->text,win_line,win_col);
    
    set_group_standout(file,options);
    rewrite(file,options,cut_buff,file->high_line,file->high_col,
	    file->high_line,file->high_col+1);
    TW_SET_MODES(file->text,NORMAL_MODE);
}

