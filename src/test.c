#include <stdio.h>
#include "eqparse.h"

int main(int argc, char ** argv){

    char * s = "pi/(2+4)";
    /*char * s = "pi/2+4";*/
    int err;
    eqparse_t * eq = eqparse(s, 0, &err);
    if(err==EQPARSE_ERROR_CHARACTER){
        printf("Unexpected character: %c\n", eq->token_error.cvalue);
    }else if(err==EQPARSE_ERROR_INTERNAL){
        printf("Internal error\n");
    }else if(err==EQPARSE_ERROR_UNMATCHED){
        printf("Unmatched bracked\n");
    }

    debug_print_ast(&eq->AST);

    eqparse_cleanup(eq);
    return 0;
}
