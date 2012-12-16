typedef unsigned int    undo_action_t;

#define UNDO_ITEM_INSERT_CHAR       1
#define UNDO_ITEM_DEL_CHAR          2
#define UNDO_ITEM_DEL_PARTIAL_LINE  3
#define UNDO_ITEM_DEL_LINE          4
#define UNDO_ITEM_CUT               5
#define UNDO_ITEM_PASTE             6
#define UNDO_ITEM_ENTER             7

#define UNDO_ITEM_NO_TEXT           ""

#define UNDO_ITEM_LINE(ui)          ((ui)->line)
#define UNDO_ITEM_COL(ui)           ((ui)->column)
#define UNDO_ITEM_ACTION(ui)        ((ui)->action)
#define UNDO_ITEM_LENGTH(ui)        ((ui)->length)
#define UNDO_ITEM_DELETED_TEXT(ui)  ((ui)->deleted_text)

#define UNDO_ITEM_OK                0
#define UNDO_ITEM_STRDUP_FAILED     1

typedef struct
{
    size_t          line;
    size_t          column;
    size_t          length;
    char            *deleted_text;
    undo_action_t   action;
}   undo_item_t;

