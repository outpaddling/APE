#ifndef _UNDO_STACK_H_
#define _UNDO_STACK_H_

#ifndef _SYS_STDINT_H_
#include <stdint.h>
#endif

#include "undo_item.h"

#define UNDO_STACK_MAX_ITEMS    64
#define UNDO_STACK_EMPTY        NULL

typedef struct
{
    size_t          count;
    size_t          top;
    undo_item_t     items[UNDO_STACK_MAX_ITEMS];
}   undo_t;

#endif  /* _UNDO_STACK_H_ */

