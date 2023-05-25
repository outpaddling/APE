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


int     col_to_menu(int col, term_t *term)

{
    if ( col >= TCOLS(term)-11 )
	return HELP_TWC_MENU;
    else if ( col >= 38 )
	return CUSTOM_TWC_MENU;
    else if ( col >= 29 )
	return OPTIONS_TWC_MENU;
    else if ( col >= 22 )
	return BUILD_TWC_MENU;
    else if ( col >= 14 )
	return SEARCH_TWC_MENU;
    else if ( col >= 8 )
	return EDIT_TWC_MENU;
    else
	return FILE_TWC_MENU;
}


int 
button1_press (event_t *event, opt_t *options)

{
    extern term_t   *Terminal;
    int     line = event->mouse_line,
	    col = event->mouse_col;
    
    switch(event->mouse_line)
    {
	case    0:
	    /*if ( event->repeat != 0 )
		return KEY_MOUSE;*/
	    return col_to_menu(col,Terminal);
	case    1:
	    if ( event->repeat == 0 )
		return TOGGLE_FILE;
	    else
		return KEY_MOUSE;
	default:
	    if ( options->scroll_bars )
	    {
		if ( (line < TLINES(Terminal)-2) && (col < TCOLS(Terminal)-1) )
		    return POSITION_CURSOR;
		else if ( (col == TCOLS(Terminal)-1) &&  /* Vertical bar */
			  (line < TLINES(Terminal)-2) )
		    return VBAR_CLICK;
		else if ( (line == TLINES(Terminal)-2) && /* Horizontal bar */
			  (col < TCOLS(Terminal)-1) )
		    return HBAR_CLICK;
	    }
	    else if ( line < TLINES(Terminal)-1 )
		return POSITION_CURSOR;
    }
    return KEY_MOUSE;
}


size_t  get_release_line(file_t *file,event_t *event)

{
    size_t  line = event->mouse_line;
    
    /* If beyond scroll bar limits, push back in */
    line = line < 3 ? 3 : ((event->mouse_line - 3) % (TW_LINES(file->text)-2));
    
    return line * file->total_lines / TW_LINES(file->text);
}

