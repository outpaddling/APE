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
#include <sys/utsname.h>
#include <sys/stat.h>
#include <twintk.h>
#include <xtend/string.h>
#include <xtend/proc.h>
#include <xtend/file.h>
#include "edit.h"
#include "protos.h"

/*
 * Set default syntax patterns for C language
 */

void 
add_c_cpp_patterns (pattern_t *patterns[])

{
    synhigh_add_pattern (patterns, "(/*).*(*/)|//.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "#ifdef|#ifndef|#if|#include|#else|#elif|#endif|#define|defined|#undef|#error|inline|typedef|switch|case|default|break|if|else|while|for|do|continue|goto|return|main",
                GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "0[Xx][0-9A-Fa-f]+|[0-9]+", RED, TEXT_BG, BOLD_MODE);
}


void 
add_c_patterns (pattern_t *patterns[])

{
    add_c_cpp_patterns (patterns);
    synhigh_add_pattern (patterns, "task|char|short|int|long|unsigned|float|double|void|struct|enum|static|register|auto|extern|const|volatile",
                 RED, TEXT_BG, DIM_MODE);
}


void 
add_cpp_patterns (pattern_t *patterns[])

{
    add_c_cpp_patterns (patterns);
    synhigh_add_pattern (patterns, "char|short|int|long|unsigned|float|double|void|struct|enum|static|register|auto|extern|const|class|public|private|operator|friend|volatile",
                 RED, TEXT_BG, DIM_MODE);
}


void 
add_csh_patterns (pattern_t *patterns[])

{
    synhigh_add_pattern (patterns, "\\$#?{?[a-zA-Z_][a-zA-Z0-9_]*}?|\\$<|\\$[0-9]+", RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "#.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "set|unset|setenv|unsetenv|alias|umask|echo|cd|source|rehash|switch|case|default|breaksw|endsw|if|then|else|endif|while|foreach|end|goto|exit",
                 GREEN, TEXT_BG, BOLD_MODE);
}


void 
add_sh_patterns (pattern_t *patterns[])

{
    synhigh_add_pattern (patterns, "\\$?[a-zA-Z_][a-zA-Z0-9_]*", RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "#.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "set|export|echo|read|cd|case|esac|if|then|elif|else|fi|while|for|do|done|in|end|goto|exit",
    GREEN, TEXT_BG, BOLD_MODE);
}


void 
add_fortran_patterns (pattern_t *patterns[])

{
    synhigh_add_pattern (patterns, "^[!Cc*].*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "do|enddo|end|if|else|elseif|endif|continue|goto|go to|stop|call|pause|return|data|parameter|entry|format|program|function|subroutine|block +data",
                 GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "integer|real|double +precision|character|logical|complex|common|equivalence|external|intrinsic|save",
                 RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "write|print|read|open|close|abs|acos|aint|anint|asin|atan|atan2|cmplx|cosh|cos|dble|dim|exp|int|log|log10|max|min|mod|nint|real|sign|sinh|sin|sqrt|tanh|tan|char|ichar|index|len|lge|lgt",
                 MAGENTA, TEXT_BG, BOLD_MODE);
}


/*
 * Author:
 * Created:
 * Description:
 * Arguments:
 * Return values:
 * FIXME: Update to include latest structure members
 * FIXME: Is init_options() even needed anymore?
 */

void 
init_options (opt_t *options)

{
    /* Patched during setup - must be static initialized array fixed size */
    static char install_prefix[APE_PATH_MAX + 1] = INSTALL_PREFIX;

    char   *user_shell;
    pattern_t **patterns;
#ifdef SCO_SV   /* OS5 binary runs on UnixWare also, so sense OS */
    struct utsname  name;
#endif
    
    /* NULL all pointers and strings */
    memset (options, 0, sizeof (*options));

    memcpy (&options->border, "*-*|*-*|-||", 11);
    options->edit_border = '_';
    options->scroll_bars = 0;
    options->use_delays = 0;
    options->seek_make = 1;
    options->reverse_menu = 1;
    options->terminfo_mouse = 0;
    options->use_html = 0;
    strlcpy (options->install_prefix, install_prefix, APE_PATH_MAX);
    strlcpy (options->browser, APE_BROWSER, APE_PATH_MAX);
    options->indent_size = 4;
    options->search_forward = 1;
    options->case_sensitive = 0;
    options->show_column = 1;
    options->smooth_scroll = 1;
    options->trap_noise = 1;
    options->prompt_tabs = 1;
    options->max_files = 30;
    options->no_acs = 0;
    strlcpy (options->file_spec, "*", TWC_SPEC_LEN);
    strlcpy (options->shell, "sh", APE_PATH_MAX);
    if ((user_shell = getenv ("SHELL")) != NULL)
        strlcpy (options->ishell, user_shell, APE_PATH_MAX);
    else
        strlcpy (options->ishell, ISHELL, APE_PATH_MAX);
    options->include_path[0] = '\0';
    options->lib_path[0] = '\0';

    /* Colors */
    black_scheme (options);

    /* Clear old list if present */
    destroy_bop_list (&options->lang_head);

    /* add_language(ext,language-id,compiler,compile-only,compile-options,
       link-options,syntax-check,debugger,debug-opts,
       run-prefix,error-format,exe-name,exe-source,auto_wrap,
       compile_to_assembly,preprocess_only,upload_prefix); */

    /* maple */

    /* Matlab */
    options->lang_head = add_language (options->lang_head,"Matlab",
        "*.m", "", "",
        "", "", "", "", "", "", "matlab -nosplash -nodesktop -r", "",
        "\\fn", "", "", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "if|else|elseif|end|for|while|return|function",
                GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "%.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "[0-9]*\\.[0-9]+|[0-9]+", RED, TEXT_BG, BOLD_MODE);
    
    /* Terminfo */
    options->lang_head = add_language (options->lang_head,"Terminfo",
       "*.ti", "", "tic", "", "", "", "", "", "",
       "infocmp", "\"\\fn\", line \\ln: col \\ig: \\te",
       "\\st", "", "Fixed", 
       AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "\\\\E", GREEN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "#[0-9]+", CYAN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "#.*$", YELLOW, TEXT_BG, DIM_MODE);

    /* Postscript */
    options->lang_head = add_language (options->lang_head,"Postscript",
        "*.ps", "", "", "", "", "", "", "", "",
        "gv", "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "/[a-zA-Z.\\-]+", GREEN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "%.*$", YELLOW, TEXT_BG, DIM_MODE);
    
    /* html */
    options->lang_head = add_language (options->lang_head,"HTML",
        "*.html .htm", "", "", "", "", "", "", "", "",
        "webbrowser", "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "</?[a-zA-Z].*>", GREEN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "<!.*>", YELLOW, TEXT_BG, DIM_MODE);
    
    /* html with embedded m4 macros */
    options->lang_head = add_language (options->lang_head,"M4HTML",
        "*.m4html", "", "m4", "", "", "", "", "", "",
        "webbrowser", "", "\\st.html", "", "Standard Output", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "</?[a-zA-Z].*>", GREEN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "<!.*>", YELLOW, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "[a-zA-Z_][a-zA-Z0-9_]*\\(.*\\)", MAGENTA, TEXT_BG, BRIGHT_MODE);

    /* Java */
    options->lang_head = add_language (options->lang_head,"Java",
        "*.java", "", "javac", "", "", "", "", "jdb", "",
        "time runjava", "\\fn: \\ln: \\te", "\\st.class", 
        "", "Fixed", NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "(/*).*(*/)|//.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "import|native|package|break|continue|if|else|switch|while|for|do|true|false|null|this|super|new|instanceof|return|default",
                 GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "boolean|char|byte|short|int|long|float|double|void|static|class|synchronized|transient|volatile|final|throws|extends|implements|interface|public|protected|private|abstract",
                 RED, TEXT_BG, DIM_MODE);

    /* bc */
    options->lang_head = add_language (options->lang_head,"Bc",
        "*.bc", "", "", "", "", "", "", "", "",
        "bc -l", "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "(/*).*(*/)|//.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]", CYAN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "length|read|scale|sqrt|print|if|while|for|break|continue|halt|return|limits|quit|warrantee|define|s\\(.+\\)|c\\(.+\\)|a\\(.+\\)|l\\(.+\\)|e\\(.+\\)|j\\(.+\\)", GREEN, TEXT_BG, DIM_MODE);

    /* Pascal */
    options->lang_head = add_language (options->lang_head,"Pascal",
        "*.p *.pas", "", "fpc", "", "", "", "", DEBUGGER, "",
        "time", "", "\\st", "-o ", 
        "Command Line Flag", 
        NO_AUTO_WRAP,"","","",BACKTRACE_CMD);
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "'.*['$]|\".*[\"$]", CYAN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "{.*}", YELLOW, TEXT_BG, BRIGHT_MODE);
    synhigh_add_pattern (patterns, "program|label|const|type|var|procedure|function|forward|begin|end|if|then|else|case|of|while|for|to|downto|with|do|repeat|until", GREEN, TEXT_BG, BRIGHT_MODE);
    synhigh_add_pattern (patterns, "integer|real|boolean|string|set|array|record|file|packed", RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "writeln|write|readln|read|rewrite|reset|put|get|page|new|dispose|pack|unpack|eoln|eof|abs|sqrt|sqr|sin|cos|arctan|ln|exp|trunc|round|ord|chr|succ|pred|odd", MAGENTA, TEXT_BG, DIM_MODE);

    /* Assembly */
    options->lang_head = add_language (options->lang_head,"ASM386",
        "*.s", "", "as", "-c", "-a", "", "", DEBUGGER, "",
        "time", "\\fn: \\ln: \\te", "\\st", "-o ", 
        "Command Line Flag", NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "\".*[\"$]", CYAN, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "%[a-zA-Z][a-zA-Z0-9]*", RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "0[Xx][0-9a-zA-Z]+|[0-9a-zA-Z]+[Hh]|[0-9]+", MAGENTA, TEXT_BG, DIM_MODE);

    /* Fortran 77 */
    options->lang_head = add_language (options->lang_head,"Fortran77",
        "*.f", "", "g77", "-c", "", "", "-c", DEBUGGER, "",
        "time", "\\ig on line \\ln of \\fn: \\te",
        "\\st", "-o ", 
        "Command Line Flag", 
        NO_AUTO_WRAP,"","","",BACKTRACE_CMD);
    add_fortran_patterns (options->lang_head->patterns);

    /* Fortran 90 */
    options->lang_head = add_language (options->lang_head,"Fortran90",
        "*.f90 .F90", "", FORTRAN_COMPILER, "-c", "", "", "-c", DEBUGGER,
        "", "time", "", "\\st", "-o ", 
        "Command Line Flag", 
        NO_AUTO_WRAP,"","","",BACKTRACE_CMD);
    add_fortran_patterns (options->lang_head->patterns);

    /* Fortran 95 */
    options->lang_head = add_language (options->lang_head,"Fortran95",
        "*.f95 .F95", "", "g95", "-c", "", "", "-c", DEBUGGER,
        "", "time", "", "\\st", "-o ", 
        "Command Line Flag", 
        NO_AUTO_WRAP,"","","",BACKTRACE_CMD);
    add_fortran_patterns (options->lang_head->patterns);

    /* Yacc */
    options->lang_head = add_language (options->lang_head,"Yacc",
        "*.y", "", "yacc", "", "", "", "", "", "",
        "", "", "y.tab.c", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");

    /* Lex */
    options->lang_head = add_language (options->lang_head,"Lex",
        "*.l", "", "lex", "", "", "", "", "", "",
        "", "", "lex.yy.c", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");

    /* Perl */
    options->lang_head = add_language (options->lang_head,"Perl",
        "*.pl", "#!/usr/bin/perl",
        "", "", "", "", "", "", "", "perl",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "\\$?[a-zA-Z_][a-zA-Z0-9_]*|<stdin>", RED, TEXT_BG, DIM_MODE);
    synhigh_add_pattern (patterns, "#.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "switch|case|default|break|if|else|elsif|while|for|do|continue|goto|return",
                 GREEN, TEXT_BG, BOLD_MODE);

    /* Awk script */
    options->lang_head = add_language (options->lang_head,"Awk",
        "*.awk", "", "", "", "", "", "", "", "", "awk -f",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "(/*).*(*/)|#.*$", YELLOW, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "\".*[\"$]|'.*['$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "NR|NF|FS|OFS|RS|ORS|FILENAME|BEGIN|END|for|do|while|if|else|case|switch|default",
                 GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "length|int|index|split|sprintf|substr|printf|print",
                 MAGENTA, TEXT_BG, DIM_MODE);

    /* Sed script */
    options->lang_head = add_language (options->lang_head,"Sed",
        "*.sed", "", "", "", "", "", "", "", "", "sed -f",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");

    /* C-shell */
    options->lang_head = add_language (options->lang_head,"Csh",
        "*.csh *.cshrc *.login",
        "#!/bin/csh", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    add_csh_patterns (options->lang_head->patterns);

    /* T-shell */
    options->lang_head = add_language (options->lang_head,"Tcsh",
        "*.tcsh *.tcshrc",
        "#!/bin/tcsh", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    add_csh_patterns (options->lang_head->patterns);

    /* Bourne-shell */
    options->lang_head = add_language (options->lang_head,"Sh",
        "*.sh *.profile",
        "#!/bin/sh", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    add_sh_patterns (options->lang_head->patterns);

    /* Korn-shell */
    options->lang_head = add_language (options->lang_head,"Ksh",
        "*.ksh *.kshrc",
        "#!/usr/bin/ksh", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    add_sh_patterns (options->lang_head->patterns);

    /* Bourne-again shell */
    options->lang_head = add_language (options->lang_head,"Bash",
        "*.bash *.bashrc",
        "#!/bin/bash", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");
    add_sh_patterns (options->lang_head->patterns);

    /* TCL */
    options->lang_head = add_language (options->lang_head,"TCL",
        "*.tcl",
        "", "", "", "", "", "", "", "", "",
        "", "\\fn", "", "Fixed", 
        NO_AUTO_WRAP,"","","","");

    /* LaTeX */
    options->lang_head = add_language (options->lang_head,"LaTeX",
        "*.tex .sty", "", "latex", "", "", "", "", "",
        "", "xdvi", "", "\\st.dvi", "", "Fixed", 
        AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "\\\\[A-Za-z]+",
                 GREEN, TEXT_BG, BOLD_MODE);    /* Primitives and macros */
    synhigh_add_pattern (patterns, "%.*$", YELLOW, TEXT_BG, BOLD_MODE);         /* Comments */
    synhigh_add_pattern (patterns, "\\\\[\\=<>]", RED, TEXT_BG, DIM_MODE);
    /* Override comments where % is escaped */
    synhigh_add_pattern (patterns, "\\\\%", TEXT_FG, TEXT_BG, NORMAL_MODE);

    /* Nroff */
    options->lang_head = add_language (options->lang_head,"Nroff",
        "*.r *.n *.1 *.2 *.3 *.4 *.5 *.6 *.7 *.8 *.9", "",
        "nroff", "", "-man", "", "", "",
        "", "more", "nroff: \\fn: \\ln: \\te", "\\st.nr",
        "", "Standard Output", 
        AUTO_WRAP,"","","","");
    patterns = options->lang_head->patterns;
    synhigh_add_pattern (patterns, "\\\\\".*$", YELLOW, TEXT_BG, BOLD_MODE);    /* Comments */
    synhigh_add_pattern (patterns, "\".*[\"$]", CYAN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "^\\.[a-z][a-z]?",   /* Primtives */
                 GREEN, TEXT_BG, BOLD_MODE);
    synhigh_add_pattern (patterns, "^\\.[A-Z][A-Z]?",   /* Macros */
                 RED, TEXT_BG, DIM_MODE);

    /* NQC */
    options->lang_head = add_language (options->lang_head,"NQC",
        "*.nqc", "", "nqc", "", "",
        "", "", "", "",
        "nqc -run", "File \"\\fn\" ; line \\ln", "\\st.rcx", "",
        "Fixed", NO_AUTO_WRAP,"","",
        "nqc -d","");
    add_c_patterns (options->lang_head->patterns);

    /* NXC */
    options->lang_head = add_language (options->lang_head,"NXC",
        "*.nxc", "", "nbc", "", "",
        "", "", "", "",
        "legoctl start", "File \"\\fn\" ; line \\ln", "\\st.rxe", "-O=", 
        "Command Line Flag", NO_AUTO_WRAP,"","",
        "legoctl --overwrite upload","");
    add_c_patterns (options->lang_head->patterns);

    /* NBC */
    options->lang_head = add_language (options->lang_head,"NBC",
        "*.nbc", "", "nbc", "", "",
        "", "", "", "",
        "legoctl start", "File \"\\fn\" ; line \\ln", "\\st.rxe", "-O=", 
        "Command Line Flag", NO_AUTO_WRAP,"","","legoctl --overwrite upload","");
    add_c_patterns (options->lang_head->patterns);

    /* NQC header files */
    options->lang_head = add_language (options->lang_head,"NQH",
        "*.nqh", "", "", "", "",
        "", "", DEBUGGER, "",
        "", ERROR_FORMAT, "", "", "", 
        NO_AUTO_WRAP,"","","","");
    add_c_patterns (options->lang_head->patterns);

    /* C++ */
#ifdef SCO_SV   /* OS5 binary runs on UnixWare also, so sense OS */
    uname(&name);
    if ( strcmp(name.sysname,"UNIX_SV") == 0 )
        options->lang_head = add_language (options->lang_head,"C++",
            "*.cc *.C *.cpp *.cxx *.c++", "", CPP_COMPILER, CCOMPILE_ONLY,
            UW7_CCOMPILE_OPTS, CLINK_OPTS, CSYNTAX_ONLY, UW7_DEBUGGER, "",
            RUN_PREFIX, UW7_ERROR_FORMAT, "\\st", "-o ", 
            "Command Line Flag", NO_AUTO_WRAP,"","","","");
    else
#endif
        options->lang_head = add_language (options->lang_head,"C++",
            "*.cc *.C *.cpp *.cxx *.c++", "", CPP_COMPILER, CCOMPILE_ONLY,
            CCOMPILE_OPTS, CLINK_OPTS, CSYNTAX_ONLY, DEBUGGER, "",
            RUN_PREFIX, CPP_ERROR_FORMAT, "\\st", "-o ", 
            "Command Line Flag", NO_AUTO_WRAP,"-S",PP_OPTIONS,"",BACKTRACE_CMD);
    add_cpp_patterns (options->lang_head->patterns);

    /* C */
#ifdef SCO_SV   /* OS5 binary runs on UnixWare also, so sense OS */
    uname(&name);
    if ( strcmp(name.sysname,"UNIX_SV") == 0 )
        options->lang_head = add_language (options->lang_head,"C",
            "*.c", "", C_COMPILER, CCOMPILE_ONLY,
            UW7_CCOMPILE_OPTS, CLINK_OPTS, CSYNTAX_ONLY, UW7_DEBUGGER, "",
            RUN_PREFIX, UW7_ERROR_FORMAT, "\\st", "-o ", 
            "Command Line Flag", NO_AUTO_WRAP,"","","","");
    else
#endif
        options->lang_head = add_language (options->lang_head,"C",
            "*.c", "", C_COMPILER, CCOMPILE_ONLY, CCOMPILE_OPTS,
            CLINK_OPTS, CSYNTAX_ONLY, DEBUGGER, "",
            RUN_PREFIX, ERROR_FORMAT, "\\st", "-o ", 
            "Command Line Flag", NO_AUTO_WRAP,"-S",PP_OPTIONS,"",BACKTRACE_CMD);
    add_c_patterns (options->lang_head->patterns);

    /* C/C++ header files */
    options->lang_head = add_language (options->lang_head,"CHeaders",
        "*.h", "", "", "", "",
        "", "", DEBUGGER, "",
        "", ERROR_FORMAT, "", "", "", 
        NO_AUTO_WRAP,"","","","");
    add_c_patterns (options->lang_head->patterns);
}


void    white_scheme (opt_t * options)

{
    options->color_scheme = WHITE_SCHEME;
    options->menu_fg = BLACK;
    options->menu_bg = WHITE;
    options->menu_hl_fg = BLUE;
    options->menu_hl_bg = WHITE;
    options->bar_fg = BLACK;
    options->bar_bg = WHITE;
    options->bar_hl_fg = BLUE;
    options->bar_hl_bg = WHITE;
    options->status_fg = BLUE;
    options->status_bg = WHITE;
    options->title_fg = MAGENTA;
    options->title_bg = WHITE;
    options->text_fg = BLACK;
    options->text_bg = WHITE;
}


void    blue_scheme (opt_t * options)

{
    options->color_scheme = BLUE_SCHEME;
    options->menu_fg = BLACK;
    options->menu_bg = CYAN;
    options->menu_hl_fg = WHITE;
    options->menu_hl_bg = BLUE;
    options->bar_fg = BLACK;
    options->bar_bg = CYAN;
    options->bar_hl_fg = WHITE;
    options->bar_hl_bg = BLUE;
    options->status_fg = BLACK;
    options->status_bg = CYAN;
    options->title_fg = WHITE;
    options->title_bg = MAGENTA;
    options->text_fg = WHITE;
    options->text_bg = BLUE;
}


void    black_scheme (opt_t * options)

{
    options->color_scheme = BLACK_SCHEME;
    options->menu_fg = BLACK;
    options->menu_bg = WHITE;
    options->menu_hl_fg = WHITE;
    options->menu_hl_bg = BLUE;
    options->bar_fg = BLACK;
    options->bar_bg = WHITE;
    options->bar_hl_fg = WHITE;
    options->bar_hl_bg = BLUE;
    options->status_fg = BLACK;
    options->status_bg = WHITE;
    options->title_fg = YELLOW;
    options->title_bg = BLUE;
    options->text_fg = WHITE;
    options->text_bg = BLACK;
}


void    cyan_scheme (opt_t * options)

{
    options->color_scheme = CYAN_SCHEME;
    options->menu_fg = BLACK;
    options->menu_bg = CYAN;
    options->menu_hl_fg = WHITE;
    options->menu_hl_bg = BLUE;
    options->bar_fg = BLACK;
    options->bar_bg = CYAN;
    options->bar_hl_fg = WHITE;
    options->bar_hl_bg = BLUE;
    options->status_fg = BLACK;
    options->status_bg = CYAN;
    options->title_fg = YELLOW;
    options->title_bg = BLUE;
    options->text_fg = WHITE;
    options->text_bg = BLACK;
}


int     check_missing_user_config(opt_t *options)

{
    char        config_dir[APE_PATH_MAX+1];
    struct stat st;
    
    get_config_dir(config_dir,APE_PATH_MAX);
    if ( stat(config_dir,&st) != 0 )
    {
        xt_rmkdir(config_dir,0755);
        return install_default_user_config(options);
    }
    else
        return 1;
}


int     install_default_user_config(opt_t *options)

{
    char    cmd[APE_CMD_MAX+1],
            *ok_button[2] = OK_BUTTON,
            msg[APE_PATH_MAX + 64], squeezed[128],
            config_dir[APE_PATH_MAX+1];
    
    get_config_dir(config_dir,APE_PATH_MAX);
    snprintf(cmd,APE_CMD_MAX,
        "cp -R %s/share/APE/Languages %s/share/APE/options.rc %s/share/APE/custom_menu %s",
        INSTALL_PREFIX, INSTALL_PREFIX, INSTALL_PREFIX, config_dir);
    if ( xt_spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"sh","-c",cmd,NULL) != 0 )
    {
        popup_mesg("Can't find default Languages in share directory!",
                    ok_button, options);
        xt_spawnlp(P_WAIT,P_NOECHO,NULL,NULL,NULL,"rm","-rf",config_dir,NULL);
        return 0;
    }
    else
    {
        snprintf(msg, APE_PATH_MAX + 63,
                "Default options installed in %s.\n", config_dir);
        xt_strsqueeze(squeezed, msg, 127);
        popup_mesg(squeezed, ok_button, options);
        return 1;
    }
}

