#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mathparse.h"

int main(int argc, char ** argv){

    parserstate_t * state = NULL;
    parse_error_e err;
    char * s = "int(sin(t), 5)";
    if((err = parse(s, strlen(s), &state)) != PARSE_ERROR_OK){
        printf("ERROR: %d\n", err);
        goto cleanup;
    }

    for(D_ARRAY_LOOP(token_t, tok, state->output)){
        print_token(*tok);
    }

cleanup:
    parser_cleanup(&state);
    return 0;
}
