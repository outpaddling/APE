/* Macros */
#define MACRO_MAX_MENU_ITEMS        30
#define MACRO_END                   '\004'
#define MACRO_MISSING_ACTION_KEY    -1
#define MACRO_FIELDS                3
#define MACRO_MARKUP_NAME_MAX       40

#define MACRO_BAD_MARKUP            -1
#define MACRO_NOT_MARKUP            -2

/* Macro flags */
#define MACRO_AUTO_INDENT           0x01
#define MACRO_AUTO_LOAD             0x02

typedef enum
{
    MACRO_NO_EXPAND,
    MACRO_EXPAND
}   macro_expand_t;

