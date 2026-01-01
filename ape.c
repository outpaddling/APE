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
#include <unistd.h>
#include <fcntl.h>
#ifdef SCO_SV
#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <time.h>
#include <sysexits.h>
#include <twintk.h>
#include <xtend/time.h>
#include "edit.h"
#include "protos.h"

/* Globals. Ick. */
win_t  *Bar_win = NULL, *Swin = NULL;
term_t *Terminal;

/**************************************************************************
 * Title:   Ape ( Another Programmers Editor )
 * Author:  Jason W. Bacon
 * History:
 *      Started 1993 after years of disgust with vi and emacs
 * 
 * FIXME:   This main is a monster function.  Much of it should be
 *          abstracted out.
 **************************************************************************/

int     main (int argc, char *argv[])

{
    file_t *files;              /* Array of open files */
    opt_t   options;            /* User configurable options */
    buff_t  cut_buff;           /* Buffer for cut and paste operations */
    int     key,                /* Key pressed while waiting for text */
            menu_key,           /* Returned from menus */
            af = 0,             /* Active file window */
            status,             /* Compile status */
            last_page = 0,      /* last page move for vbar click */
            marked;             /* Area marked in this file */
    buff_len_t   len;
    boolean restore_bar_help = FALSE;

    event_t event;              /* Keyboard and mouse event */
    char   *key_seq = event.seq,
            temp_seq[SEQ_LEN + 3] = "",    /* For displaying invalid keys */
            search_string[SEARCH_STR_LEN + 1] = "",
            help_str[APE_CMD_MAX + 1] = "",
           *curchar;
    proj_t  project;            /* Must run proj_init() */
    err_t   errfile;            /* Error file information */
    size_t  curline;            /* For caching files[af].curline */
    extern err_t *Errfile;      /* For kamakaze() signal handler */
    extern char __Syms[];       /* For group char macros */
    extern  file_t  *Files;
    extern  opt_t   *Options;
    
    struct timeval  new_tv, old_tv = {0,0};
    unsigned long   time_diff;

    err_init(&errfile);
    proj_init(&project);

    /* Initialize - don't mess with the order of these calls! */
    setup_terminal();

    check_args (argv);          /* Check for command-line options */
    //set_globals_for_signal(files, &options);
    
    /* Clear cut buffer */
    init_cut_buff (&cut_buff);

    /* Set terminal driver for raw mode */
    traw_mode (Terminal);

    /* 
     *  Set built-in default options so install_default_user_config
     *  can display a popup if necessary.
     *  FIXME: Is init_options() even needed anymore?
     */
    init_options(&options);
    check_missing_user_config(&options);
    
    /* Now load user config options */
    if (load_options (APERC, &options) == CANT_LOAD)
    {
        init_options (&options);
    }
    Terminal->use_delays = options.use_delays;

    /* Allocate array of open file structures - after load_options */
    if ( (files = MALLOC(options.max_files, file_t)) == NULL )
    {
        fprintf(stderr, "Cannot allocate file structure.\n");
        return EX_UNAVAILABLE;
    }
    memset (files, 0, sizeof (file_t) * options.max_files);
    setup_resize (files, &af, &options, &cut_buff);

    Errfile = &errfile;         /* Need global for kamakaze */
    Files = files;
    Options = &options;

    /* Create menu bar and status bar - After load_options */
    global_wins (&options);

    /* Set up key bindings for APE */
    load_keymap (Terminal);

    /* Initialize edit windows and open files listed on command line */
    /* do_args() must come after files[] is allocated */
    af = do_args (argc, argv, files, &options);
    
    /* Various signal() and sigset() calls */
    register_signal_handlers ();

    /* Look for makefile in the startup directory */
    if (options.seek_make)
    {
        if (check_makefile (&project, "Makefile", files + af, &options) == -1)
        {
            /* If "Makefile" doesn't exist, check for "makefile" */
            check_makefile (&project, "makefile", files + af, &options);
        }
        stat_mesg(BAR_HELP);
    }

    /* Begin edit session */
    do
    {
        /* Highlight balancing paren if necessary */
        curchar = files[af].curchar;
        if (ISGROUP ((int) *curchar))
            match_group (files + af, &options, &cut_buff);

        if ( options.scroll_bars_at_startup )
        {
            /* Update scroll bars if needed */
            if (files[af].curline != files[af].old_curline)
            {
                tw_move_scroll_bar (files[af].vscroll_bar,
                                  files[af].curline, files[af].total_lines);
                files[af].old_curline = files[af].curline;
            }
            
            if (files[af].leftcol != files[af].old_leftcol )
            {
                buff_len_t   maxlen = 0;
                size_t  c, endline;
                
                c = files[af].curline;
                endline = c+TW_LINES(files[af].text);
                while ( c < endline )
                {
                    if ( files[af].line[c].length > maxlen )
                        maxlen = files[af].line[c].length;
                    ++c;
                }
                tw_move_scroll_bar(files[af].hscroll_bar,
                        files[af].leftcol, maxlen+1);
                files[af].old_leftcol = files[af].leftcol;
            }
        }

        /* Display line, and possibly col in status bar */
        show_cursor_pos (&options, files + af);
        
        /* Make sure cursor in right place */
        TW_SYNC_CURSOR (files[af].text);

        /* Read keystroke */
        /* If signal received, read might fail and return -1 */
        while ((key = tgetevent (Terminal, &event)) == -1)
            ;

        /* Record time of keystroke.  If next keystroke happens too
           soon after, the input must be coming from a paste operation,
           and we'll disable autoindent in order to get a clean paste. */
        gettimeofday(&new_tv, NULL);
        time_diff = xt_difftimeofday(&new_tv, &old_tv);
        old_tv = new_tv;
        
        /* Set alarm for menu help message */
        alarm(10);
        
        /* Unhighlight old group char */
        if (ISGROUP ((int) *curchar))
            unmatch_group (files + af, &options, &cut_buff);

        menu_key = 0;
        do  /* Repeat inner loop only to switch menus */
        {
            /* Translate mouse events into codes for switch below */
            if ( key == KEY_MOUSE )
            {
                switch (event.type)
                {
                    /* Check first to override button being held */
                case MOTION:
                    break;
                case BUTTON1_PRESS:
                case BUTTON1_MOTION:
                    key = button1_press (&event, &options); 
                    /* Ignore if same as last menu selection */
                    /*
                    if ( key == last_menu )
                        key = KEY_MOUSE;
                    else
                        last_menu = key;
                    */
                    break;
                case BUTTON2_PRESS:
                    break;
                case BUTTON3_PRESS:     /* Button 3 marks area */
                case BUTTON3_MOTION:
                    if ((event.mouse_line > 1) &&
                        (event.mouse_line < TLINES (Terminal) - 1))
                    {

                        move_to (files + af, &options, &cut_buff,
                                 event.mouse_line + files[af].topline - 2,
                                 event.mouse_col + files[af].leftcol);
                        key = MARK_AREA;
                        if (area_started (&cut_buff) &&
                            (cut_buff.file == files + af))
                            adjust_highlight (files + af, &cut_buff, &options);
                    }
                    break;
                case BUTTON1_RELEASE:
                    /*if ( event.mouse_line != grab_line )
                    {
                        line = get_release_line(files+af,&event);
                        move_to (files + af, &options, &cut_buff, line,0);
                        update_win(files + af,&options,&cut_buff);
                    }*/
                    break;
                case BUTTON2_RELEASE:
                case BUTTON3_RELEASE:
                    break;
                default:
                    break;
                }
            }
            
            /* Process final key codes */
            switch (key)
            {
            case KEY_MOUSE:
                break;
            case POSITION_CURSOR:       /* Button 1 press in text */
                move_to (files + af, &options, &cut_buff,
                         event.mouse_line + files[af].topline - 2,
                         event.mouse_col + files[af].leftcol);
                menu_key = 0;       /* Stop menu loop */
                if ( area_started(&cut_buff) )
                {
                    unmark_area(&cut_buff);
                    update_win(files+af,&options,&cut_buff);
                }
                break;
            case VBAR_CLICK:
                /* Scroll up or down if user clicks on scroll bar */
                if ((event.mouse_line == 2) && ((int) files[af].topline > 0))
                {
                    move_up (files + af, &options, &cut_buff);
                    if (files[af].curline != files[af].topline)
                    {
                        int     save = options.smooth_scroll;

                        /* Force smooth scroll if user clicked on arrow */
                        options.smooth_scroll = 1;
                        scroll_file (files + af, &options, &cut_buff,
                                    SCROLL_REVERSE);
                        options.smooth_scroll = save;
                    }
                }
                else if ((event.mouse_line == TLINES (Terminal) - 3) &&
                         ((int) files[af].topline <
                            (int) (files[af].total_lines -
                            TW_LINES (files[af].text))))
                {
                    move_down (files + af, &options, &cut_buff);
                    if (files[af].curline != files[af].topline + TW_LINES (files[af].text))
                    {
                        int     save = options.smooth_scroll;

                        options.smooth_scroll = 1;
                        scroll_file (files + af, &options, &cut_buff, SCROLL_FORWARD);
                        options.smooth_scroll = save;
                    }
                }
                else if (event.mouse_line - 2 < TW_CUR_LINE (files[af].vscroll_bar))
                {
                    if ( (event.repeat == 0) || (last_page != NEXT_PAGE) )
                    {
                        page_up (files + af, &options, &cut_buff);
                        last_page = PREVIOUS_PAGE;
                    }
                }
                else if (event.mouse_line - 2 > TW_CUR_LINE (files[af].vscroll_bar))
                {
                    if ( (event.repeat == 0) || (last_page != PREVIOUS_PAGE) )
                    {
                        page_down (files + af, &options, &cut_buff);
                        last_page = NEXT_PAGE;
                    }
                }
                break;

            case HBAR_CLICK:
                if ( (event.mouse_col < TW_CUR_COL(files[af].hscroll_bar)) &&
                     (files[af].leftcol > 0) )
                {
                    curline = files[af].curline;
                    files[af].curcol =
                        MAX((int)(TW_CUR_COL(files[af].text)-TW_COLS(files[af].text)),0);
                    files[af].curchar = files[af].line[curline].buff + files[af].curcol;
                    check_column(files+af,&options,&cut_buff);
                }
                else if ( (event.mouse_col > TW_CUR_COL(files[af].hscroll_bar)) &&
                          (files[af].leftcol < files[af].max_line_len - TW_COLS(files[af].text)) )
                {
                    curline = files[af].curline;
                    files[af].curcol =
                        MIN(TW_CUR_COL(files[af].text)+TW_COLS(files[af].text),
                            (int)files[af].line[curline].length);
                    files[af].curchar = files[af].line[curline].buff + files[af].curcol;
                    check_column(files+af,&options,&cut_buff);
                }
                break;
            case KEY_ENTER:
            case '\012':
            case '\015':
                if (files[af].insert)   /* Split current line */
                    new_line (files + af, &options, &cut_buff, time_diff);
                else            /* Move to start of next line */
                    move_ret (files + af, &options, &cut_buff);
                break;
            case KEY_IL:        /* Insert-line key, if present */
                new_line (files + af, &options, &cut_buff, time_diff);
                break;
            case FILE_TWC_MENU:
                menu_key = file_menu (files, &af, &options, &cut_buff,
                                  &event);
                break;
            case EDIT_TWC_MENU:
                menu_key = edit_menu (files, &af, &cut_buff, &options,
                                  &event);
                TW_RESTORE_WIN (files[af].window);
                break;
            case KEY_F9:
            case INVOKE_MACRO:
                /* Pretend edit menu was invoked for menu switching */
                key = EDIT_TWC_MENU;
                menu_key = macro_menu (files, &af, &options, &event, MACRO_EXPAND);
                TW_RESTORE_WIN (files[af].window);
                break;
            case MARK_COL1:
                begin_line (files + af, &options, &cut_buff);
                /* No break */
            case MARK_AREA:
            case KEY_F4:
            case KEY_MARK:
                if (!area_started (&cut_buff))
                    begin_area (files + af, &cut_buff);
                else if (!area_ended (&cut_buff) &&
                         (cut_buff.file == files + af))
                    end_area (files + af, &options, &cut_buff);
                else
                    cancel_area (files+af,&cut_buff, &options);
                break;
            case KEY_PASTE_AREA:
            case KEY_COPY:
                paste_area (files + af, &options, &cut_buff, 0);
                break;
            case SEARCH_TWC_MENU:
                menu_key = search_menu (files + af, search_string, &options, &cut_buff, &event);
                break;
            case FIND_NEXT:
            case KEY_F3:
                if (*search_string != '\0')
                {
                    size_t  curline = files[af].curline;

                    move_search (files + af, search_string, &options,
                                 &cut_buff, files[af].curline,
                           files[af].curchar - files[af].line[curline].buff,
                                 TW_LINES (files[af].text));
                    break;
                }
            case QUICK_FIND:
            case KEY_FIND:
                find_string(files+af, search_string, &options, &cut_buff);
                break;
            case RESUME_SEARCH:
                replace_string (files + af, search_string, &options, &cut_buff, TRUE);
                break;
            case BUILD_TWC_MENU:
                menu_key = build_menu (files, &af, &project, &options,
                                   &errfile, &cut_buff, &event);
                break;
            case OPTIONS_TWC_MENU:
                menu_key = options_menu (files, af, &options, &cut_buff,
                                     &event);
                TW_RESTORE_WIN (files[af].window);
                break;
            case CUSTOM_TWC_MENU:
                menu_key = custom_menu (files, &af, &options, &event);
                break;
            case HELP_TWC_MENU:
                menu_key = help_menu (files + af, &options, &event);
                break;
            case KEY_F1:        /* Context sensitive help */
            case KEY_HELP:
                get_word_at_cursor (files + af, help_str);
                man (help_str,NULL);
                TW_RESTORE_WIN (files[af].window);
                break;
            case TOGGLE_FILE:   /* Switch to another file */
                toggle_file (files, &af, &options, &cut_buff);
                break;
            case SAVE_FILE:     /* Esc-Esc */
            case KEY_SAVE_FILE:
            case KEY_SAVE:
                /* Check for invalid sequences like Esc-F2 */
                if (eat_keys (12500, key_seq) == 0)
                {
                    synhigh_update(files+af,files[af].curline,&options,&cut_buff);
                    if (strcmp (files[af].source, "untitled") == 0)
                        save_as (files, af, &options);
                    else
                        save_file (files + af, &options);
                }
                else
                {
                    snprintf (temp_seq, SEQ_LEN + 3, "\033\033%s", key_seq);
                    invalid_key (27, temp_seq);
                }
                restore_bar_help = TRUE;
                break;
            case KEY_UP:
                move_up (files + af, &options, &cut_buff);
                break;
            case KEY_DOWN:
                move_down (files + af, &options, &cut_buff);
                break;
            case KEY_LEFT:
                move_left (files + af, &options, &cut_buff);
                break;
            case KEY_RIGHT:
                move_right (files + af, &options, &cut_buff);
                break;
            case KEY_BACKSPACE:
            case BACKSPACE:
                /* Unindent if area marked, delete otherwise */
                if (area_started (&cut_buff) &&
                    (cut_buff.file == files + af))
                {
                    if ( !area_ended(&cut_buff) )
                        end_area (files + af, &options, &cut_buff);
                    unindent_area (files + af, &options, &cut_buff,
                                    options.indent_size);
                }
                else
                    del_left (files + af, &options, &cut_buff);
                break;
            case KEY_DC:
            case DELETE_TWC_CHAR:
                del_under (files + af, &options, &cut_buff);
                break;
            case CUT_AREA:
            case KEY_DL:
                /* Delete area if marked or character otherwise */
                if (area_started (&cut_buff) && (cut_buff.file == files + af))
                    cut_area (files + af, &options, &cut_buff);
                else
                    cut_line (files + af, &options, &cut_buff);
                break;
            case DEL_WORD:
                del_word (files + af, &options, &cut_buff);
                break;
            case DEL_TO_EOLN:
            case KEY_EOL:
                cut_to_end (files + af, &options, &cut_buff);
                break;
            case UNDO:
            case KEY_UNDO:
                // Disabled until undo feature is finished
                // file_undo (files + af, &options, &cut_buff);
                break;
            case KEY_NPAGE:
            case NEXT_PAGE:
                page_down (files + af, &options, &cut_buff);
                break;
            case KEY_PPAGE:
            case PREVIOUS_PAGE:
                page_up (files + af, &options, &cut_buff);
                break;
            case TOGGLE_INS:    /* Toggle insert and replace mode */
            case KEY_IC:
            case KEY_REPLACE:
                files[af].insert = !files[af].insert;
                display_mode (files + af);
                break;
            case FIRST_LINE:    /* Top of file */
                home_file (files + af, &options, &cut_buff);
                break;
            case LAST_LINE:
                end_file (files + af, &options, &cut_buff);
                break;
            case NEXT_WORD:
                next_word (files + af, &options, &cut_buff);
                break;
            case PREVIOUS_WORD:
                last_word (files + af, &options, &cut_buff);
                break;
            case PAGE_TOP:
                page_top (files + af, &options, &cut_buff);
                break;
            case END_LINE:      /* Go to eoln */
            case KEY_END:
                end_line (files + af, &options, &cut_buff);
                break;
            case BEGIN_LINE:    /* Go to boln */
            case KEY_HOME:
                begin_line (files + af, &options, &cut_buff);
                break;
            case KEY_BTAB:
                /* Unindent area if marked, or goto previous tab stop otherwise */
                if (area_started (&cut_buff) &&
                    (cut_buff.file == files + af))
                {
                    if ( !area_ended(&cut_buff) )
                        end_area(files+af,&options,&cut_buff);
                    unindent_area (files + af, &options, &cut_buff,
                                options.indent_size);
                }
                else
                    back_tab (files + af, &options, &cut_buff);
                break;
            case GOTO_LINE:
                gotoline (files + af, &options, &cut_buff);
                stat_mesg (BAR_HELP);
                break;
            case JOIN_LINES:
                join_lines (files + af, &options, &cut_buff);
                break;
            case RUN_PROG:
            case KEY_F5:
            case UPLOAD_PROG:
                status = run_prog (files, af, &project, &errfile, &options, key);
                if ( errors(status, &errfile) )
                    next_error (files, &af, &errfile, &options, &cut_buff);
                TW_RESTORE_WIN (files[af].window);
                break;
            case COMPILE_PROG:
            case KEY_F6:
                status = compile_prog (files, af, &errfile, &options, OBJECT);
                if ( errors(status, &errfile) )
                    next_error (files, &af, &errfile, &options, &cut_buff);
                TW_RESTORE_WIN (files[af].window);
                break;
            case SYNTAX_CHECK:
            case KEY_F8:
                status = syntax_check (files, af, &options, &errfile);
                if ( errors(status, &errfile) )
                    next_error (files, &af, &errfile, &options, &cut_buff);
                TW_RESTORE_WIN (files[af].window);
                break;
            case BUILD_PROG:
            case KEY_F7:
                status = build(files, af, &project, &errfile, &options);
                if ( errors(status, &errfile) )
                    next_error(files, &af, &errfile, &options, &cut_buff);
                TW_RESTORE_WIN (files[af].window);
                break;
            case UPDATE_WIN:
            case KEY_REFRESH:
                synhigh_tag_lines(files+af,0,files[af].total_lines-1);
                TW_REDRAW_WIN(Bar_win);
                TW_REDRAW_WIN(Swin);
                edit_border(files+af,&options);
                update_win(files+af,&options,&cut_buff);
                break;
            case NEXT_ERROR:
                next_error (files, &af, &errfile, &options, &cut_buff);
                restore_bar_help = FALSE;
                break;
            case KEY_F10:
                /* Old: run_unix (files, &options);
                TW_RESTORE_WIN (files[af].window);*/
                suspend(&options);
                Terminal->button_status = BUTTON_UNKNOWN;
                TW_RESTORE_WIN(files[af].window);
                break;
            case KEY_F11:
                // format_paragraph (files + af, &options, &cut_buff);
                break;
            case UNDEFINED_KEY:
                invalid_key (key, key_seq);
                break;
            default:
                /* Some keys have different meaning when area is marked */
                marked = area_started (&cut_buff) && 
                        (cut_buff.file == files + af);
                if (marked)
                {
                    if ( !area_ended(&cut_buff) )
                        end_area(files+af,&options,&cut_buff);
                    if (key == TAB)
                    {
                        indent_area (files + af, &options, &cut_buff,
                                options.indent_size);
                        break;
                    }
                    else if (key == ' ')
                    {
                        indent_area (files + af, &options, &cut_buff, 1);
                        break;
                    }
                }
                if ((isascii (key) && isprint (key)) || isspace (key))
                {
                    /* Check for auto-wrap */
                    /*len = files[af].line[files[af].curline].length; */
                    len = ACTUAL_COL(files + af);
                    if (USE_AUTO_WRAP (files + af) &&
                        ((int) len >= TW_COLS (files[af].text) - 4))
                        auto_wrap (files + af, &options, &cut_buff);

                    /* Insert new char */
                    insert_char (files + af, key, &options, &cut_buff);

                    /* Restore help message if needed */
                    if (restore_bar_help)
                    {
                        stat_mesg (BAR_HELP);
                        restore_bar_help = FALSE;
                    }
                }
                else
                    invalid_key (key, key_seq);
                break;
            }                   /* switch */

            /* Move to adjacent menu if arrow key was pressed */
            switch (menu_key)
            {
            case KEY_LEFT:
                if (key == FILE_TWC_MENU)
                    key = HELP_TWC_MENU;
                else
                    --key;
                break;
            case KEY_RIGHT:
                if (key == HELP_TWC_MENU)
                    key = FILE_TWC_MENU;
                else
                    ++key;
                break;
            case TWC_OUTSIDE_WIN:   /* Kill menus */
                key = KEY_MOUSE;
                break;
            default:
                break;
            }
            /* Continue while new menus are being selected */
        }
        while ((menu_key == KEY_LEFT) || (menu_key == KEY_RIGHT));
        /* || ((menu_key == TWC_OUTSIDE_WIN) &&
                (event.mouse_line == 0) && (event.repeat == 0))); */

        /* Highlight marked area if area started */
        if ((files + af == cut_buff.file) && area_started (&cut_buff) &&
            !area_ended (&cut_buff))
            adjust_highlight (files + af, &cut_buff, &options);
    }
    while ((key != FILE_TWC_MENU) || 
           ((menu_key != 'q') && (menu_key != 'x')));

    nice_exit (0, &errfile, "");
    return 0;
}


/***************************************
 * Restore terminal and exit gracefully
 ***************************************/

void    nice_exit (int retval, err_t *errfile, char *message)

{
    extern file_t   *Files;
    extern opt_t    *Options;
    
    unlink(".untitled.undo");           /* Created when last file is closed */
    if (Terminal->windowid != -1)
    {
        TERMINFO_MOUSE_OFF (Terminal);
        /* Change cursor back */
    }
    tset_startup_tty (Terminal);
    tset_modes (Terminal, NORMAL_MODE);
    err_close (errfile);
    tclear_screen(Terminal);    /* Needed by cygwin */
    //TFLUSH_OUT(Terminal);
    
    // Fixes color reset problem in KDE3 Terminal
    raw_print(Terminal, Terminal->reset_1string);
    raw_print(Terminal, Terminal->reset_2string);
    raw_print(Terminal, Terminal->reset_3string);
    
    TFLUSH_OUT(Terminal);
    if (*message)
        puts (message);
    
    // xt_spawnlp(P_WAIT, P_NOECHO, NULL, NULL, NULL, "reset", "-Q", NULL);
    
    /* tclose_mouse_server (Terminal); */
    exit (retval);
}


/*
 * Eat keystrokes for the given time interval.
 * This is meant to swallow invalid keystrokes
 * and possibly line noise.
 */

int     eat_keys (unsigned long time, char seq[])

{
    long    flags, count;

    /* Wait for chars to accumulate */
#if !(defined(SCO_SV) || defined(hpux))
    usleep (time);
#endif

    /* First, flush chars already buffered */
    fflush (stdin);

    /* read() without blocking */
    flags = fcntl (0, F_GETFL);
    fcntl (0, F_SETFL, flags | O_NONBLOCK);
    count = fread (seq, sizeof (char), 10, stdin);

    fcntl (0, F_SETFL, flags);
    return count;
}


void    err_init(err_t *errfile)

{
    getcwd(errfile->filename, APE_PATH_MAX+1);
    strlcat(errfile->filename, "/.ape-compile-errors.txt", APE_PATH_MAX+1);
    errfile->fp = NULL;
}
