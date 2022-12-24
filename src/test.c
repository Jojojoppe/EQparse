#include <stdio.h>
#include "eqparse.h"

int main(int argc, char ** argv){

    char * s = "x = -1+y*-21.3";
    int err = EQPARSE_ERROR_OK;
    eqparse_t * eq = eqparse(s, 0, &err);
    if(err==EQPARSE_ERROR_CHARACTER){
        printf("Unexpected character: %c\n", eq->token_error.cvalue);
    }else if(err==EQPARSE_ERROR_INTERNAL){
        printf("Internal error\n");
    }else if(err==EQPARSE_ERROR_UNMATCHED){
        printf("Unmatched bracked\n");
    }else if(err==EQPARSE_ERROR_EXPRESSION){
        printf("Unfinished expression\n");
    }else if(err==EQPARSE_ERROR_PARAM){
        printf("Parameter error\n");
    }else if(err==EQPARSE_ERROR_INVALID){
        printf("Invalid expression\n");
    }

    if(!err){
        debug_print_ast(&eq->AST);
        debug_write_eq(eq, "testout.dot");
    }

    eqparse_cleanup(eq);
    return 0;
}
