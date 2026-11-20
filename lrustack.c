

#include <stdlib.h>
#include "lrustack.h"

/**
 * 
 *  - get LRU: gets the current index of the LRU block
 *  - set MRU: sets a certain block's index as the MRU. 
 */

/**
 * Function to initialize an LRU stack for a cache set with a given <size>
 * 
 * @param size is the size of the LRU stack to initialize. 
 * @return the dynamically allocated stack. 
 */
lru_stack_t* init_lru_stack(int size) {
    //  Use malloc to dynamically allocate a lru_stack_t
	lru_stack_t* stack = (lru_stack_t*) malloc(sizeof(lru_stack_t));
    //  Set the stack size the caller passed in
	stack->size = size;
    stack->used = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        stack->used[i] = i;
    }

	return stack;
}

/**
 * Function to get the index of the least recently used cache block, as indicated by <stack>
 * 
 * @param stack is the stack to run the operation on.
 * @return the index of the LRU cache block.
 */
int lru_stack_get_lru(lru_stack_t* stack) {
    return stack->used[stack->size - 1];
}

/**
 * Function to mark the cache block with index <n> as MRU in <stack>
 * 
 * @param stack is the stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n) {
    int position = 0;
    for (int i = 0; i < stack->size; i++) {
        if (stack->used[i] == n) {
            position = i;
            break;
        }
    }
    for (int i = position; i >= 1; i--) {
        stack->used[i] = stack->used[i - 1];
    }
    stack->used[0] = n;
}

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack) {

    free(stack->used);
    free(stack);        // Free the stack struct we malloc'd
}