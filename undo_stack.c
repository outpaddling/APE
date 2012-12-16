#include <string.h>
#include "edit.h"
#include "protos.h"

int     undo_stack_push(
	    undo_t *record,
	    size_t line,
	    size_t col,
	    undo_action_t action,
	    char *deleted_text )

{
    /*
     *  Circular stack.  When array is full, loop back to index 0
     *  and overwrite oldest items.
     */
    
    if ( record->count < UNDO_STACK_MAX_ITEMS )
	++record->count;
    
    //sprintw(2, 50, "Adding undo item at %d", record->top);
    undo_item_set(record->items + record->top, line, col, strlen(deleted_text),
		action, deleted_text);
    record->top = (record->top + 1) % UNDO_STACK_MAX_ITEMS;
    return OK;
}


undo_item_t *undo_stack_pop(undo_t *record)

{
    if ( record->count > 0 )
    {
	--record->count;
	if ( record->top-- == 0 )
	    record->top = UNDO_STACK_MAX_ITEMS - 1;
	//sprintw(2, 50, "Popping item at %d", record->top);
	return record->items + record->top;
    }
    else
    {
	stat_mesg("Undo stack empty.");
	return NULL;
    }
}


void    undo_stack_init(undo_t *record)

{
    record->count = 0;
    record->top = 0;
}

