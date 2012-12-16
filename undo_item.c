#include <stdio.h>
#include <string.h>
#include "undo_item.h"

int undo_item_set(undo_item_t *item, size_t line, size_t column,
			size_t length, undo_action_t action,
			const char *deleted_text)
{
    item->line = line;
    item->column = column;
    item->length = length;
    item->action = action;
    if ( (item->deleted_text = strdup(deleted_text)) == NULL )
	return UNDO_ITEM_STRDUP_FAILED;
    return UNDO_ITEM_OK;
}

