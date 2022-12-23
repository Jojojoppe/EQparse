#include <stdio.h>
#include "stack.h"

int main(int argc, char ** argv){

    d_stack_t stack;
    D_STACK_CREATE(int, &stack);

    int i;

    i = 5; d_stack_push(&stack, &i);
    i = 4; d_stack_push(&stack, &i);
    i = 3; d_stack_push(&stack, &i);
    i = 2; d_stack_push(&stack, &i);
    i = 1; d_stack_push(&stack, &i);
    i = 50; d_stack_push(&stack, &i);

    D_STACK_FOREACH_BT(int, i, &stack){
        printf("bt: %d\n", *i);
    }
    printf("\n");
    D_STACK_FOREACH_TB(int, i, &stack){
        printf("tb: %d\n", *i);
    }

    int popped = *D_STACK_POP_DATA(int, &stack); // pop 50
    printf("popped: %d\n", popped);

    int top = *D_STACK_TOP(int, &stack);
    printf("top now = %d\n", top);
    int at1 = *D_STACK_AT(int, &stack, 1);
    printf("at 1 now %d\n", at1);

    d_stack_destroy(&stack);

    return 0;
}
