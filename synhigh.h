/* Syntax highlighting */
#define MAX_PATTERNS    8       /* # of different patterns in one language */
#define MAX_RE          1024    /* Max length of a pattern */
#define TEXT_BG         -1      /* Specifies same color as text fg or bg */
#define TEXT_FG         -1
#define END_PATTERNS    -2
#define HAS_START_PATTERN(f,l)\
	((f)->line[l].start_pattern >= 0)
	
/*
 * Each line is marked with starting pattern # for update_line() 
 * to initialize to.
 * (e.g. for lines like this one that begin within a comment token)
 */
#define NO_PATTERN      -1  
#define START_PATTERN   -2  /* Maybe useful in determining when to retag */
#define END_PATTERN     -3
#define IN_PATTERN      -4

typedef struct
{
    cre_t   compiled_re;
    char    *re;
    char    foreground;         /* Highlight colors for color terminals */
    char    background;
    char    modes;              /* Highlight modes for monochrome terminals */
}   pattern_t;

