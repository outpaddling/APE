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

/* Portability stuff */
#define GCC_ERROR_FORMAT    "\\fn: \\ln: \\te"

#if defined(__FreeBSD__)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "/usr/bin/time"
#define DEBUGGER        "lldb -c \\ex.core \\ex"
#define BACKTRACE_CMD   "bt"
#define ERROR_FORMAT    "\\fn: \\ln: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define X11_INCLUDE     "/usr/X11R6/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "gprof"
#define PROF_OUT        "gmon.out"
#define PROF_OPT        "-pg"
/* Use manpath command to get system manpath */
#define MAN             "man -a"
#define MANOPTS         "-a"
#define DYNAMIC_MANPATH

#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "/usr/bin/time"
#define DEBUGGER        "gdb \\ex \\ex.core"
#define BACKTRACE_CMD   "where"
#define ERROR_FORMAT    "\\fn: \\ln: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define X11_INCLUDE     "/usr/X11R6/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "gprof"
#define PROF_OUT        "gmon.out"
#define PROF_OPT        "-pg"
/* Use manpath command to get system manpath */
#define MAN             "man -a"
#define MANOPTS         "-a"
#define DYNAMIC_MANPATH

#elif defined(__APPLE__)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/opt/local/include"
#define CCOMPILE_ONLY   "-c"
#define CLINK_OPTS      "-L/opt/local/lib -lm"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "/usr/bin/time"
#define DEBUGGER        "gdb"
#define BACKTRACE_CMD   "where"
#define ERROR_FORMAT    "\\fn: \\ln: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define X11_INCLUDE     "/usr/X11R6/include/X11"
#define LOCAL_INCLUDE   "/opt/local/include"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "gprof"
#define PROF_OUT        "gmon.out"
#define PROF_OPT        "-pg"
/* Use manpath command to get system manpath */
#define MAN             "man -a"
#define MANOPTS         "-a"
#define DYNAMIC_MANPATH

#elif defined(linux)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "gdb"
#define BACKTRACE_CMD   "where"
#define ERROR_FORMAT    "\\fn: \\ln: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define X11_INCLUDE     "/usr/X11R6/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ISHELL          "bash"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "gprof"
#define PROF_OUT        "gmon.out"
#define PROF_OPT        "-pg"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         ""

/* Patch for non-posix fpurge() */
#define fpurge(s)       __fpurge(s)
#include <stdio_ext.h>

#elif defined(__CYGWIN__)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "/usr/bin/time"
#define DEBUGGER        "gdb"
#define BACKTRACE_CMD   "where"
#define ERROR_FORMAT    "\\fn: \\ln: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define X11_INCLUDE     "/usr/X11R6/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ISHELL          "bash"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "gprof"
#define PROF_OUT        "gmon.out"
#define PROF_OPT        "-pg"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         ""

/* Dummy placeholder for systems with no fpurge */
#define fpurge(s)       fflush(s)

#elif defined(IRIX64) || defined(IRIX)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "dbx"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\\ln: \\fn: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         "/opt/man:/usr/share/man:/usr/freeware/man:/usr/local/man"

#elif defined(sun)  /* Sun Solaris cc/gcc */

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    ""
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "dbx"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\\ln: \\fn: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         "/opt/man:/usr/share/man:/usr/local/man"

/* Patch for non-posix fpurge() */
#define fpurge(s)       __fpurge(s)
#include <stdio_ext.h>

#elif defined(osf1)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-Wall -g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "dbx"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\\ln: \\fn: \\te"
#define CPP_ERROR_FORMAT    "\\fn: \\ln: \\te"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man"
#define MANOPTS         ""
#define MANPATH         "/usr/man:/usr/share/man:/usr/local/man"

#elif defined(SCO_SV)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "CC"
#define CCOMPILE_OPTS   "-g -v -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-c"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "dbxtra"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\"\\fn\", line \\ln: \\te"
#define CPP_ERROR_FORMAT ERROR_FORMAT
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse/bus0"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         "scohelp"
/* Special options for running the SCO OS/5 binary under UnixWare 7 */
#define UW7_ERROR_FORMAT    "\\ig:\\ig: \\ig: \"\\fn\", line \\ln: \\te"
#define UW7_CCOMPILE_OPTS   "-g -v -DDEBUG=1 -I/usr/local/include"
#define UW7_DEBUGGER        "debug"

#elif defined(UnixWare)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "CC"
#define CCOMPILE_OPTS   "-g -v -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-c"
#define CLINK_OPTS      "-L/usr/local/lib -lm"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "debug"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\\ig:\\ig: \\ig: \"\\fn\", line \\ln: \\te"
#define CPP_ERROR_FORMAT ERROR_FORMAT
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse/bus0"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         "/opt/man:/usr/share/man:/usr/freeware/man:/usr/local/man"

#elif defined(hpux)

#define C_COMPILER      "cc"
#define CPP_COMPILER    "c++"
#define CCOMPILE_OPTS   "-g -DDEBUG=1 -I/usr/local/include"
#define CCOMPILE_ONLY   "-c"
#define CSYNTAX_ONLY    "-fsyntax-only"
#define FORTRAN_COMPILER "f90"
#define PP_OPTIONS      "-E -C"
#define RUN_PREFIX      "time"
#define DEBUGGER        "dbx"
#define BACKTRACE_CMD   "where"
#define X11_INCLUDE     "/usr/include/X11"
#define LOCAL_INCLUDE   "/usr/local/include"
#define ERROR_FORMAT    "\\ln: \\fn: \\te"
#define CPP_ERROR_FORMAT    "\\ln: \\fn: \\te"
#define ISHELL          "csh"
#define MOUSE_DEV       "/dev/mouse"
#define PROF_CMD        "prof"
#define PROF_OUT        "mon.out"
#define PROF_OPT        "-p"
#define MAN             "man -a"
#define MANOPTS         "-a"
#define MANPATH         "/usr/share/man:/usr/contrib/man:/usr/local/man"

#else

#error No options defined for this platform.
#endif

#ifndef SCO_SV
#define MOUSE_OPTIONS {"Microsoft Serial","Mouse Systems/BSD Sysmouse",\
			  "ATI/Logitec Bus","PS/2","Custom Mouse",\
			  "No Mouse",NULL}
#else
#define MOUSE_OPTIONS {"Microsoft Serial","Mouse Systems/BSD Sysmouse",\
			  "Bus Mouse","PS/2","Custom Mouse",\
			  "No Mouse", NULL}
#endif

