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
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <twintk.h>
#include <xtend/string.h>
#include <xtend/proc.h>
#include <xtend/file.h>
#include "edit.h"
#include "protos.h"


int     options_menu(file_t file[], int af, opt_t *options,buff_t *cut_buff,event_t *event)

{
    extern term_t   *Terminal;
    int     ch, save=1, start_row = 1,
            max_files = options->max_files,
            old_no_color = MONO_MODE(options);
    win_t  *options_pop;
    opt_t   old_options;
    lang_t   old_lang;
    char    /* *buttons[3] = YES_NO_BUTTONS, */
            config_dir[APE_PATH_MAX+1],
            filename[APE_PATH_MAX+13],
            *reset_buttons[3] = YES_NO_BUTTONS;
            
    struct stat st;
    static char *options_text[] = {
                                ".Language",
                                ".Debug",
                                ".Screen and keyboard",
                                "Syntax .Highlighting",
                                ".Miscellaneous",
                                TWC_HLINE,
                                ".Reset built-in defaults",
                                /*
                                "S.Ave as personal defaults",
                                "Load .Personal defaults",
                                */
                                ""};

    old_options = *options;

    /* See if options.rc has been modified */
    if (get_config_dir(config_dir,APE_PATH_MAX) != NULL)
    {
        snprintf(filename, APE_PATH_MAX + 12, "%s/options.rc", config_dir);
        if ( (stat(filename,&st) == 0) &&
             (st.st_size > 0) &&
             (st.st_mtime > options->loaded) )
        {
            redraw_screen(options,file+af,cut_buff);
            //sprintw(2,50,"Loading options... %d",options->no_acs);
            //tgetc(Terminal);
            if ( load_options(APERC,options) != CANT_LOAD )
                stat_mesg("Options have changed - reloaded.");
            //sprintw(2,50,"Loaded options... %d",options->no_acs);
            //tgetc(Terminal);
            redraw_globals(options,old_no_color);
            /* FIXME: Crashes program if scroll_bar is off and 
              vscroll_bar == NULL */
            redraw_screen(options,file+af,cut_buff);
        }
    }

    /* Save current options in case user cancels */
    /* Is this still necessary with new tw_input_panel cancel button? */
    if ( file[af].lang != NULL )
        old_lang = *file[af].lang;
    else
        memset(&old_lang,0,sizeof(old_lang));
    
    /* Create options pop-down menu */
    options_pop = tw_menu(Terminal, 1, 27, options_text, &options->border,
                     options->no_acs, MONO_MODE(options),
                     options->menu_fg, options->menu_bg,
                     options->menu_hl_fg, options->menu_hl_bg);
    tw_set_win_attr(options_pop,REVERSE_MODE,options->menu_fg,options->menu_bg,
                    BOLD_MODE,options->menu_hl_fg,options->menu_hl_bg);
    
    /* Get selection from pop-down menu */
    ch = tw_get_item(options_pop, options_text, event, &start_row,
                    options->reverse_menu,NULL);
    switch (ch)
    {
        case 'l':   /* Edit language options if they exist */
            save = language_options(file+af, options);
            break;
        case 'd':   /* Edit debug options if they exist */
            save = debug_options(file+af, options);
            break;
        case 's':   /* Edit screen options */
            save = screen_options(options);
            break;
        case 'h':   /* Edit syntax highlighting options if they exist */
            save = synhigh_options(file+af, options, cut_buff);
            break;
        case 'm':   /* Miscellaneous options */
            save = misc_options(options);
            break;
        /*
        case 'a':
            save_options(APE_DEFAULTS,options);
            save = 0;
            break;
        case 'p':
            load_options(APE_DEFAULTS,options);
            for (c=0; c<options->max_files; ++c)
                if ( file[c].window != NULL )
                    select_compiler(file+c,options);
            save = 1;
            break;
        */
        /* Reset hard-coded defaults */
        case 'r':
            if ( tolower(popup_mesg("Are you sure you want to reset defaults?",
                    reset_buttons,options)) == 'y' )
            {
                save = 1;
                init_options(options);
                /* Reset compiler options for each open file */
                for (int c=0; c<options->max_files; ++c)
                    if ( file[c].window != NULL )
                        select_compiler(file+c,options);
            }
            else
                save = 0;
            break;
        default:
            save = 0;
            break;
    }
    
    /* Save options if changed */
    if ( save )
    {
        redraw_globals(options,old_no_color);
        save_options(APERC,options);
        synhigh_tag_lines(file+af,0,file[af].total_lines-1);
        if ( (ch == 'r') || (ch == 's') )
            redraw_screen(options,file+af,cut_buff);
    }
    else    /* Is this necessary since adding cancel option? */
    {
        /* Restore options to original values */
        *options = old_options;
        if ( file[af].lang != NULL )
            *file[af].lang = old_lang;
    }
    
    /* Restore original max_files until next edit session */
    /* New value is saved in config_dir/options.rc and will be loaded next time */
    if ( options->max_files != max_files )
    {
        char    *buttons[2] = OK_BUTTON;
        popup_mesg(" APE must be restarted to effect the new open file maximum ",
            buttons,options);
    }
    options->max_files = max_files;
    
    tw_del_win(&options_pop);
    return (ch);
}


/*
 * Author:
 * Created:
 * Description:
 * Arguments:
 */

void    redraw_globals(opt_t *options, int old_no_color)

{
    extern win_t   *Swin, *Bar_win;
    
    /* Check for changes to color options */
    if ( MONO_MODE(options) != old_no_color )
    {
        tw_del_win(&Swin);
        tw_del_win(&Bar_win);
        global_wins(options);
    }
}


/*
 *  FIXME: scroll_bars must be on, or this will crash the program
 *  due to scroll_bar_win being NULL.
 */
 
void    redraw_screen(opt_t *options, file_t *file, buff_t *cut_buff)

{
    draw_menu_bar(options);
    draw_status_bar(options);
    
    /* Redraw file window in case colors or use_color changed */
    edit_border(file,options);
    if ( MONO_MODE(options) )
    {
        tw_set_foreground(file->text,WHITE);
        tw_set_background(file->text,BLACK);
    }
    else
    {
        tw_set_foreground(file->text,options->text_fg);
        tw_set_background(file->text,options->text_bg);
    }
    update_win(file,options,cut_buff);
}


/*******************************************
 * Load the user's options file if present.
 * FIXME: Save as name=value so new options can be added without breaking
 * compatibility
 *******************************************/

int     load_options (char *filename, opt_t *options)

{
    extern          win_t *Swin;
    FILE            *fp;
    int             extra;
    char            pathname[FULL_PATH_MAX + 2], 
                    config_dir[APE_PATH_MAX + 1],
                    extra_buff[200];
    unsigned char   *cp;
    boolean         *bp;
    unsigned int    *ip,
                    temp_ch;

    /* Determine pathname */
    if (get_config_dir (config_dir, APE_PATH_MAX) == NULL)
        return CANT_LOAD;
    snprintf (pathname, FULL_PATH_MAX + 1, "%s/%s", config_dir, filename);

    /* Attempt to read */
    if ((fp = fopen (pathname, "r")) == NULL)
        return CANT_LOAD;
    else
    {
        /* Load all but build options */
        /* TWIN border_t */
        for (cp = &(options->border.upper_left);
             cp <= &(options->border.right_tee); ++cp)
        {
            if (fscanf (fp, "%u ", &temp_ch) != 1)
            {
                fclose (fp);
                return CANT_LOAD;
            }
            *cp = temp_ch;
        }
        
        /* Booleans */
        for (bp = &options->use_delays; bp <= &options->prompt_tabs; ++bp)
        {
            if (fscanf (fp, "%u ", &temp_ch) != 1)
            {
                fclose (fp);
                return CANT_LOAD;
            }
            *bp = temp_ch;
        }
        options->scroll_bars_at_startup = options->scroll_bars;
        
        /* Integers */
        for (ip = &options->indent_size; ip <= &options->edit_border; ++ip)
        {
            if (fscanf (fp, "%u ", &temp_ch) != 1)
            {
                fclose (fp);
                return CANT_LOAD;
            }
            *ip = temp_ch;
        }

        xt_fgetline (fp, options->install_prefix, APE_PATH_MAX);
        xt_fgetline (fp, options->browser, APE_PATH_MAX);
        xt_fgetline (fp, options->shell, APE_PATH_MAX);
        xt_fgetline (fp, options->ishell, APE_PATH_MAX);
        // Override stored ishell if SHELL set in env
        if ( getenv("SHELL") != NULL )
            strlcpy(options->ishell, getenv("SHELL"), APE_PATH_MAX);
        xt_fgetline (fp, options->file_spec, TWC_SPEC_LEN);
        xt_fgetline (fp, options->include_path, APE_PATH_MAX);
        xt_fgetline (fp, options->lib_path, APE_PATH_MAX);
        
        /* Check for extra stuff */
        extra = fread (extra_buff, 199, 1, fp);

        /* See if there's any trailing garbage */
        if ( extra != 0 )
        {
            if (Swin == NULL)
            {
                fflush (stderr);        /* BSD */
                fflush (stdout);
                extra_buff[extra] = '\0';
                fprintf (stderr, "%s is corrupt.  Using defaults.\r\n", filename);
                fprintf (stderr, "extra = %d %s.\r\n",
                         extra, extra_buff);
                fprintf (stderr, "Press any key to continue...");
                getchar ();
            }
            else
                sprintw (2, TWC_ST_LEN, "%s is corrupt.  Using defaults.",
                         filename);
            return CANT_LOAD;
        }
        fclose (fp);

        /* Read language options */
        options->lang_head = NULL;
        read_language_opts (&options->lang_head);
    }
    time (&options->loaded);
    return OK;
}


/****************************************************
 * Save user's options file as config_dir/options.rc
 ****************************************************/

void    save_options(char *filename, opt_t *options)

{
    FILE            *fp;
    struct stat     st;
    char            pathname[FULL_PATH_MAX+2],
                    config_dir[APE_PATH_MAX+1], 
                    *button[2] = OK_BUTTON;
    unsigned char   *cp;
    boolean         *bp;
    unsigned int    *ip;
    
    /* Get pathname for user's options file */
    if (get_config_dir(config_dir,APE_PATH_MAX) == NULL)
    {
        stat_mesg("save_options(): Cannot find config directory!");
        return;
    }
    
    if ( stat(config_dir,&st) != 0 )
    {
        if ( xt_rmkdir(config_dir,0755) != 0 )
            stat_mesg("save_options(): Cannot create config directory.");
    }
    
    /* Write options file */
    snprintf(pathname, FULL_PATH_MAX + 1, "%s/%s", config_dir, filename);
    fp = fopen(pathname, "w");
    if (fp == NULL)
    {
        popup_mesg( "Cannot save options!.", button, options);
        return;
    }
    
    /* Save all but build options */
    /* TWIN border_t */
    for (cp = &options->border.upper_left; cp <= &options->border.right_tee; ++cp)
        fprintf(fp,"%u ",*cp);
    putc('\n',fp);
    
    /* Booleans */
    for (bp = &options->use_delays; bp <= &options->prompt_tabs; ++bp)
        fprintf(fp,"%u ",*bp);
    putc('\n',fp);
    
    /* Integers */
    for (ip = &options->indent_size; ip <= &options->edit_border; ++ip)
    {
        fprintf(fp,"%u ",*ip);
    }
    putc('\n',fp);

    /* Strings */
    fprintf(fp,"%s\n",options->install_prefix);
    fprintf(fp,"%s\n",options->browser);
    fprintf(fp,"%s\n",options->shell);
    fprintf(fp,"%s\n",options->ishell);
    fprintf(fp,"%s\n",options->file_spec);
    fprintf(fp,"%s\n",options->include_path);
    fprintf(fp,"%s\n",options->lib_path);

    fclose(fp);

    /* Save language options */
    save_language_opts(options->lang_head);
    
    sprintw(2, TWC_ST_LEN, "Saved options to %s", filename);
    time(&options->loaded);
}


int     create_lang_options_if_missing(file_t *file, opt_t *options)

{
    char    msg[128],*buttons[3] = YES_NO_BUTTONS;
    int     status = 0;
    
    if ( file->lang == NULL )
    {
        snprintf(msg,127,"No language options for %s.  Create?",
            file->short_src);
        if ( tolower(popup_mesg(msg,buttons,options)) == 'y' )
        {
            if ( (file->lang = new_bop(&options->lang_head,file->source))
                == NULL )
            {
                sprintw(2,TWC_ST_LEN,"Out of memory.");
                status = 1;
            }
        }
        else
            status = 1;
    }
    TW_RESTORE_WIN(file->window);
    return status;
}


int     debug_options(file_t *file, opt_t *options)

{
    extern term_t   *Terminal;
    tw_panel_t panel = TWC_PANEL_INIT;
    win_t  *win;
    int     status;

    /* Check for options and create if desired */
    if ( create_lang_options_if_missing(file, options) )
        return FALSE;
    
    win = centered_panel_win(9, 75, options);

    tw_init_string(&panel, 2, 2, APE_PATH_MAX, TW_COLS(win)-22, TWC_VERBATIM,
                "Debugger command:  ",
                " \\ex = executable name ", file->lang->debugger_cmd);
    tw_init_string(&panel, 3, 2, BACKTRACE_LEN, TW_COLS(win)-22, TWC_VERBATIM,
                "Backtrace command: ",
                " Debugger command that shows function call trace. ",
                file->lang->debugger_backtrace_cmd);
    status = tw_input_panel(win, &panel, TW_LINES(win) - 2);
    tw_del_win(&win);
    return TW_EXIT_KEY(status) == TWC_INPUT_DONE;
}


void    draw_color_bar(win_t *win, int  line, opt_t   *options)

{
    int     c, save_fg = TW_CUR_FOREGROUND(win),
            save_bg = TW_CUR_BACKGROUND(win),
            save_modes = TW_CUR_MODES(win),
            col, colors_to_show;
    char    *color_string = " Color sample: ";
    
    /* Draw color bar */
    if ( TCOLOR_TERM(win->terminal) && !MONO_MODE(options) )
    {
        colors_to_show = MIN(TMAX_COLORS(win->terminal), 16);
        col = (TW_COLS(win) - strlen(color_string) - colors_to_show * 3) / 2;
        tw_move_to(win, line, col);
        tw_printf(win, color_string);
        for (c = 0; c < colors_to_show; ++c)
        {
            tw_set_background(win, c);
            if (c == 7)
                tw_set_foreground(win, 0);
            else
                tw_set_foreground(win, 7);
            tw_printf(win, " %d ", c);
        }
        tw_set_foreground(win, save_fg);
        tw_set_background(win, save_bg);
        TW_SET_MODES(win, save_modes);
    }
}


int     screen_options(opt_t *options)

{
    extern term_t   *Terminal;
    tw_panel_t panel = TWC_PANEL_INIT;
    win_t  *win;
    bord_t  borders;
    int     status;
    char    *yes_no[3] = YES_NO_ENUM,
            *border_help = " Decimal ASCII code or literal (e.g. '-') for window border. ",
            *indent = " The number of columns to indent each new level of code. ",
            *scroll_options[3] = {"Smooth", "Page", NULL}, scroll_mode[7],
            *color_options[7] = {WHITE_SCHEME_STR, BLUE_SCHEME_STR,
                                BLACK_SCHEME_STR, CYAN_SCHEME_STR,
                                MONO_SCHEME_STR, USER_DEFINED_STR, NULL},
                                color_scheme[25],
            use_acs[4], show_col[4], reverse_menu[4], enable_sb[4];

    strlcpy(scroll_mode, options->smooth_scroll ? "Smooth" : "Page", 7);
    sprintw(2,50,"%d %s", options->smooth_scroll, scroll_mode);
    strlcpy(use_acs,options->no_acs ? "No" : "Yes",4);
    switch(options->color_scheme)
    {
        case    MONO_SCHEME:
            strlcpy(color_scheme,MONO_SCHEME_STR,24);
            break;
        case    WHITE_SCHEME:
            strlcpy(color_scheme,WHITE_SCHEME_STR,24);
            break;
        case    BLUE_SCHEME:
            strlcpy(color_scheme,BLUE_SCHEME_STR,24);
            break;
        case    BLACK_SCHEME:
            strlcpy(color_scheme,BLACK_SCHEME_STR,24);
            break;
        case    CYAN_SCHEME:
            strlcpy(color_scheme,CYAN_SCHEME_STR,24);
            break;
        case    USER_DEFINED:
            strlcpy(color_scheme,USER_DEFINED_STR,24);
            break;
    }
    strlcpy(show_col,options->show_column ? "Yes" : "No",4);
    strlcpy(reverse_menu,options->reverse_menu ? "Yes" : "No",4);
    strlcpy(enable_sb, options->scroll_bars ? "Yes" : "No", 4);

    win = centered_panel_win(20, 76, options);
    
    get_borders(options,&borders);
    
    tw_init_uint(&panel, 2, 4, 1, 40,
             "Indent size: ", indent, &options->indent_size);
    tw_init_enum(&panel, 2, 20, 7, scroll_options, "Scroll mode:  ",
              " Hit <space> to toggle.  Use Page mode if smooth scrolling is slow. ",
              scroll_mode);
    tw_init_enum(&panel, 2, 43, 4, yes_no, "Use alternate char set: ",
        " Hit <space> to toggle.  Select \"No\" to use border characters below.",
        use_acs);
    tw_init_enum(&panel, 3, 4, 4, yes_no,
                "Show column: ",
                " Hit <space> to toggle.  Disable if cursor motion is slow. ",
                show_col);
    tw_init_enum(&panel, 3, 22, 4, yes_no,
                "Reverse video menus? ",
                " Hit <space> to toggle.  Disable if highlighting is slow. ",
                reverse_menu );
    tw_init_enum(&panel, 3, 48, 4, yes_no,
                "Enable scroll bars? ",
                " Hit <space> to toggle. ",
                enable_sb );
        
    tw_init_string(&panel, 5, 4, 4, 4, TWC_VERBATIM, "Top left:     ",
             border_help, borders.top_left);
    tw_init_string(&panel, 5, 24, 4, 4, TWC_VERBATIM, "Top:          ",
             border_help, borders.top);
    tw_init_string(&panel, 5, 44, 4, 4, TWC_VERBATIM, "Top right:    ",
             border_help, borders.top_right);
    tw_init_string(&panel, 6, 4, 4, 4, TWC_VERBATIM, "Right:        ",
             border_help, borders.right);
    tw_init_string(&panel, 6, 24, 4, 4, TWC_VERBATIM, "Bottom right: ",
             border_help, borders.bottom_right);
    tw_init_string(&panel, 6, 44, 4, 4, TWC_VERBATIM, "Bottom:       ",
             border_help, borders.bottom);
    tw_init_string(&panel, 7, 4, 4, 4, TWC_VERBATIM, "Bottom left:  ",
             border_help, borders.bottom_left);
    tw_init_string(&panel, 7, 24, 4, 4, TWC_VERBATIM, "Left:         ",
             border_help, borders.left);
    tw_init_string(&panel, 7, 44, 4, 4, TWC_VERBATIM, "Edit border:  ",
             border_help, borders.edit_border);

    tw_init_uint(&panel,  9, 4, 0, 7, "Menu bar Fg:   ",
             "Foreground color for menu bar (top line of screen).", &options->bar_fg);
    tw_init_uint(&panel,  9, 24, 0, 7, "Bg: ",
             "Background color for menu bar (top line of screen).", &options->bar_bg);
    tw_init_uint(&panel,  9, 34, 0, 7, "Highlight Fg:  ",
             "Foreground color for highlighted characters in menu bar.", &options->bar_hl_fg);
    tw_init_uint(&panel,  9, 54, 0, 7, "Bg: ",
             "Background color for highlighted characters in menu bar.", &options->bar_hl_bg);
    tw_init_uint(&panel,  10, 4, 0, 7, "Menu Fg:       ",
             "Foreground color for pop-down menus.", &options->menu_fg);
    tw_init_uint(&panel,  10, 24, 0, 7, "Bg: ",
             "Background color for pop-down menus.", &options->menu_bg);
    tw_init_uint(&panel,  10, 34, 0, 7, "Highlight Fg:  ",
             "Foreground color for highlighted characters in menus.", &options->menu_hl_fg);
    tw_init_uint(&panel,  10, 54, 0, 7, "Bg: ",
             "Background color for highlighted characters in menus.", &options->menu_hl_bg);
    tw_init_uint(&panel, 11, 4, 0, 7, "Status bar Fg: ",
             "Foreground color for status bar (bottom line of screen).", &options->status_fg);
    tw_init_uint(&panel, 11, 24, 0, 7, "Bg: ",
             "Background color for status bar (bottom line of screen).", &options->status_bg);
    tw_init_uint(&panel, 11, 34, 0, 7, "Filename Fg:   ",
             "Foreground color for file title.", &options->title_fg);
    tw_init_uint(&panel, 11, 54, 0, 7, "Bg: ",
             "Background color for file title.", &options->title_bg);
    tw_init_uint(&panel, 12, 4, 0, 7, "File text Fg:  ",
             "Foreground color for edit window.", &options->text_fg);
    tw_init_uint(&panel, 12, 24, 0, 7, "Bg: ",
             "Background color for edit window.", &options->text_bg);
    tw_init_enum(&panel, 12, 34, 24, color_options, "Color scheme: ",
            "Preconfigured color sets.  Hit <space> to toggle.",
            color_scheme);
    
    draw_color_bar(win,TW_LINES(win)-6,options);
    /* Clears window - tw_draw_border() needed after */
    set_popup_color(win,options);
    tw_draw_border(win);
    
    status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
    set_borders(options,&borders);
    options->smooth_scroll = (strcmp(scroll_mode, "Smooth") == 0);
    //sprintw(2,50,"scroll_mode = %s",scroll_mode);
    //tgetc(Terminal);
    options->no_acs = (strcmp(use_acs, "No") == 0);
    
    if ( strcmp(color_scheme,MONO_SCHEME_STR) == 0)
        options->color_scheme = MONO_SCHEME;
    else if ( strcmp(color_scheme,WHITE_SCHEME_STR) == 0 )
        white_scheme(options);
    else if ( strcmp(color_scheme,BLUE_SCHEME_STR) == 0 )
        blue_scheme(options);
    else if ( strcmp(color_scheme,BLACK_SCHEME_STR) == 0 )
        black_scheme(options);
    else if ( strcmp(color_scheme,CYAN_SCHEME_STR) == 0 )
        cyan_scheme(options);
    else if ( strcmp(color_scheme,USER_DEFINED_STR) == 0 )
        options->color_scheme = USER_DEFINED;
    options->show_column = (strcmp(show_col,"Yes") == 0);
    if ( !options->show_column )
        sprintw(TCOLS(Terminal)-6,4,"");
    options->reverse_menu = (strcmp(reverse_menu,"Yes") == 0);
    options->scroll_bars = (strcmp(enable_sb,"Yes") == 0); 
    tw_del_win(&win);
    return TW_EXIT_KEY(status) == TWC_INPUT_DONE;
}


int     misc_options(opt_t *options)

{
    extern term_t   *Terminal;
    tw_panel_t panel = TWC_PANEL_INIT;
    win_t  *win;
    int     status;
    char    *yes_no[] = YES_NO_ENUM,
            *html[] = {"man","HTML",NULL},
            use_delays[4], seek_make[4], docs[5];
            
    win = centered_panel_win(MITW_OPTIONS+5, 75, options);

    tw_init_uint(&panel, 2, 2, 1, 1000, "Maximum open files:   ",
                " Limit on number of files users can edit at once. ",
                &options->max_files);
    tw_init_string(&panel, 2, 32, APE_PATH_MAX,TW_COLS(win)-56, TWC_VERBATIM,
                "Command shell:    ",
                " Shell used to run programs from build and custom menus. ",
                options->shell);
    tw_init_enum(&panel, 3, 2, 4, yes_no,
                "Use terminfo delays:  ",
                " Hit <space> to toggle.  Prevents overrun on slow terminals. ", use_delays);
    tw_init_enum(&panel, 3, 32, 4, yes_no,
                "Detect makefiles: ",
                " Hit <space> to toggle. Looks for makefiles at startup time. ", seek_make );
    tw_init_string(&panel, 4, 2, APE_PATH_MAX,TW_COLS(win)-26, TWC_VERBATIM,
                "Interactive shell:    ",
                " Shell used for File menu/Unix shell prompt. ",
                options->ishell);
    tw_init_string(&panel, 5, 2, APE_PATH_MAX,TW_COLS(win)-26, TWC_VERBATIM,
                "Include search path:  ",
                " Extra include directory for \"Search Header files\" and \"View Header\". ",
                options->include_path);
    tw_init_string(&panel, 6, 2, APE_PATH_MAX,TW_COLS(win)-26, TWC_VERBATIM,
                "Library search path:  ",
                " Additional lib directory for \"Search Libraries\". ",
                options->lib_path);
    tw_init_string(&panel, 7, 2, TWC_SPEC_LEN,TW_COLS(win)-26, TWC_VERBATIM,
                "Default file spec:    ",
                " File specification for File menu \"List Files\" item. ",
                options->file_spec);
    tw_init_enum(&panel, 8, 2, 5, html,
                "Documentation type:   ",
                " Hit <space> to toggle. ", docs);
    tw_init_string(&panel, 9, 2, APE_PATH_MAX,TW_COLS(win)-26, TWC_VERBATIM,
                "Browser:              ",
                " Browser and options to use for HTML docs. ",options->browser);
    tw_init_string(&panel, 10, 2, APE_PATH_MAX,TW_COLS(win)-26, TWC_VERBATIM,
                "Install prefix:       ",
                " Location of APE installation. ",
                options->install_prefix);
    
    strlcpy(use_delays,options->use_delays?"Yes":"No",4);
    strlcpy(seek_make,options->seek_make?"Yes":"No",4);
    strlcpy(docs,options->use_html?"HTML":"man",5);
        
    status = tw_input_panel(win, &panel, TW_LINES(win) - 3);
    tw_del_win(&win);
    Terminal->use_delays = options->use_delays = (strcmp(use_delays,"Yes") == 0);
    options->seek_make = (strcmp(seek_make,"Yes") == 0); 
    options->use_html = (strcmp(docs,"HTML")==0);
    return TW_EXIT_KEY(status) == TWC_INPUT_DONE;
}


void    get_borders(opt_t *options, bord_t *borders)

{
    /* Need unsigned so that ascii codes show up as 0 - 255 in panel */
    unsigned char    *bp, *ip;
    int     c;
    
    ip=(unsigned char *)&(UPPER_LEFT(&options->border));
    bp=(unsigned char *)borders->top_left;
    for (c=0; c<9; ++c)
    {
        if ( isprint(*ip) )
            snprintf((char *)bp,4,"'%c'",*ip);
        else
            snprintf((char *)bp,4,"%d",*ip);
        ++ip;
        bp += 4;
    }
}


void    set_borders(opt_t *options, bord_t  *borders)

{
    char    *bp, *ip;
    int     c;
    
    ip=(char *)&(UPPER_LEFT(&options->border));
    bp=borders->top_left;
    for (c=0; c<9; ++c)
    {
        if ( *bp == '\'' )
            *ip = bp[1];
        else
            *ip = atoi(bp);
        ++ip;
        bp += 4;
    }
}


char    *get_config_dir(char *dir,size_t maxlen)

{
    char    home[APE_PATH_MAX+1],
            base_version[20],
            *p = base_version;  // Just for strsep
    
    if (xt_get_home_dir(home,APE_PATH_MAX) != NULL)
    {
        strlcpy(base_version, APE_VERSION, 20);
        strsep(&p, "-");   // Chop off revisions and commit hashes
        snprintf(dir, maxlen-1, "%s/.ape-%s", home, base_version);
        return dir;
    }
    else
    {
        sprintw(2,TWC_ST_LEN,"Cannot get home directory!");
        return NULL;
    }
}


int     get_language_parent_dir(char language_parent_dir[],size_t maxlen)

{
    char    config_dir[APE_PATH_MAX+1];
    
    get_config_dir(config_dir,APE_PATH_MAX);
    
    /* Use the language name as the macro dir */
    snprintf(language_parent_dir,maxlen, "%s/Languages", config_dir);
    return 0;
}


int     get_language_dir(lang_t *lang,char language_dir[],size_t maxlen)

{
    char    language_parent_dir[APE_PATH_MAX+1];
    
    if ( lang == NULL )
        return NO_LANGUAGE_OPTS;

    get_language_parent_dir(language_parent_dir, APE_PATH_MAX);
    
    /* Use the language name as the macro dir */
    snprintf(language_dir, maxlen, "%s/%s", language_parent_dir,lang->lang_name);
    return 0;
}

