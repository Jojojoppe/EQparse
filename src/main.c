#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mathparse.h"

int main(int argc, char ** argv){

    parserstate_t * state = NULL;
    parse_error_e err;
    if((err = parse(argv[1], strlen(argv[1]), &state)) != PARSE_ERROR_OK){
        printf("ERROR: %d\n", err);
        goto cleanup;
    }

    for(D_ARRAY_LOOP(token_t, tok, state->output)){
        print_token(*tok);
    }

    parser_cleanup(&state);
cleanup:
    return 0;
}
