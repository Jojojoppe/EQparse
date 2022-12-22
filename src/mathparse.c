#include "mathparse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char * functions[_FUNCTION_SIZE_] = {
    "sin",
    "cos",
    "tan",
    "step",
};

const operation_t operations[_OPERATION_SIZE_] = {
    {'+', OPERATION_PLUS, OPERATION_ASS_LEFT, 2},
    {'-', OPERATION_MINUS, OPERATION_ASS_LEFT, 2},
    {'*', OPERATION_MULTIPLY, OPERATION_ASS_LEFT, 3},
    {'/', OPERATION_DIVIDE, OPERATION_ASS_LEFT, 3},
    {'^', OPERATION_POW, OPERATION_ASS_RIGHT, 4},
    {'%', OPERATION_MOD, OPERATION_ASS_LEFT, 3},
};

token_t nexttoken(parserstate_t * state){
    char buf[128] = {0};
    size_t bufpos = 0;

    int error = 0;

    token_t ntok = {
        .type = TOKENTYPE_EOF,
        .dvalue = 0,
    };

    while(1){
        char c = state->string[state->position++];

        // Ignore whitespaces
        if(c=='\t' || c==' ' || c=='\n'){
            continue;
        }

        // Add characters to buf when not in NUMBER mode
        if(((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_') && ntok.type!=TOKENTYPE_NUMBER){
            buf[bufpos++] = c;
            ntok.type = TOKENTYPE_VARIABLE;
            continue;
        }

        // If came here while ntok is VARIABLE that means end of variable or function
        if(ntok.type == TOKENTYPE_VARIABLE){
            // Check to see if buf contains known function name
            for(int i=0; i<_FUNCTION_SIZE_; i++){
                if(!strcmp(functions[i], buf)){
                    ntok.type = TOKENTYPE_FUNCTION;
                    ntok.fvalue = i;
                    goto endofvariable;
                }
            }
            // Not a known function -> variable
            ntok.svalue = calloc(strlen(buf)+1, 1);
            strcpy(ntok.svalue, buf);
        endofvariable:
            state->position--;
            break;
        }

        // Add numbers to buf
        else if(c>='0' && c<='9'){
            buf[bufpos++] = c;
            ntok.type = TOKENTYPE_NUMBER;
            continue;
        }else if((c=='.' || c==',' || c=='e' || c=='E') && ntok.type==TOKENTYPE_NUMBER){
            buf[bufpos++] = c;
            continue;
        }

        // If came here while ntok is NUMBER that means end of number
        if(ntok.type == TOKENTYPE_NUMBER){
            // interpret number
            ntok.dvalue = atof(buf);
            state->position--;
            break;
        }

        // Operations
        if(c=='+' || c=='-' || c=='*' || c=='/' || c=='^' || c=='%'){
            ntok.type = TOKENTYPE_OPERATOR;
            for(int i=0; i<_OPERATION_SIZE_; i++){
                if(c==operations[i].opchar){
                    ntok.ovalue = operations[i];
                    goto endofoperations;
                }
            }
            error = 1; 
        endofoperations:
            break;
        }

        // Brackets
        else if(c=='('){
            ntok.type = TOKENTYPE_BRACKOPEN;
            break;
        }else if(c==')'){
            ntok.type = TOKENTYPE_BRACKCLOSE;
            break;
        }

        // Stop parsing at the end of a string
        else if(c==0){
            break;
        }

        // Should not come here
        else{
            error = 1;
            break;
        }
    }

    if(error) ntok.type = TOKENTYPE_ERROR;
    return ntok;
}

void print_token(token_t t){
    switch(t.type){
        case TOKENTYPE_EOF: 
            break;
        case TOKENTYPE_ERROR:
            printf("ERROR: unexpected character\n");
            break;
        case TOKENTYPE_NUMBER:
            printf("number    ] %lf\n", t.dvalue);
            break;
        case TOKENTYPE_OPERATOR:
            printf("operator  ] %c\n", t.ovalue.opchar);
            break;
        case TOKENTYPE_BRACKOPEN:
            printf("bracket   ] (\n");
            break;
        case TOKENTYPE_BRACKCLOSE:
            printf("bracket   ] )\n");
            break;
        case TOKENTYPE_FUNCTION:
            printf("function  ] %s\n", functions[t.fvalue]);
            break;
        case TOKENTYPE_VARIABLE:
            printf("variable  ] %s\n", t.svalue);
            /*free(t.svalue);*/
            break;
        default:
            printf("ERROR: unknown token type\n");
            break;
    }
}

parse_error_e parse(const char * string, size_t len, parserstate_t ** state){
    if(string==NULL) return PARSE_ERROR_STRINGISNULL;
    if(*state!=NULL) return PARSE_ERROR_STATEALREADYINITIALIZED;
    if(len==0) return PARSE_ERROR_STRINGLENISZERO;

    *state = calloc(sizeof(parserstate_t), 1);
    D_ARRAY_INIT(token_t, &(*state)->output);
    D_ARRAY_INIT(token_t, &(*state)->stack);
    (*state)->string = string;

    token_t t;
    while(1){
        t = nexttoken(*state);

        if(t.type==TOKENTYPE_EOF || t.type==TOKENTYPE_ERROR){
            break;
        } else if(t.type == TOKENTYPE_NUMBER || t.type == TOKENTYPE_VARIABLE){
            d_array_insert(&(*state)->output, &t);
        } else if(t.type == TOKENTYPE_FUNCTION){
            d_array_insert(&(*state)->stack, &t);
        } else if(t.type == TOKENTYPE_OPERATOR){
            while(
                    D_ARRAY_LEN((*state)->stack)>0 && 
                    D_ARRAY_END(token_t, (*state)->stack)->type!=TOKENTYPE_BRACKOPEN &&
                    (D_ARRAY_END(token_t, (*state)->stack)->ovalue.precedence > t.ovalue.precedence ||
                    (D_ARRAY_END(token_t, (*state)->stack)->ovalue.precedence==t.ovalue.precedence && t.ovalue.ass==OPERATION_ASS_LEFT))){
                token_t * top = D_ARRAY_END(token_t, (*state)->stack);
                d_array_insert(&(*state)->output, top);
                d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            }
            d_array_insert(&(*state)->stack, &t);
        } else if(t.type == TOKENTYPE_BRACKOPEN){
            d_array_insert(&(*state)->stack, &t);
        } else if(t.type == TOKENTYPE_BRACKCLOSE){
            if(D_ARRAY_LEN((*state)->stack)==0){
                return PARSE_ERROR_UNMATCHEDBRACKET;
            }
            while(
                    D_ARRAY_LEN((*state)->stack)>0 &&
                    D_ARRAY_END(token_t, (*state)->stack)->type!=TOKENTYPE_BRACKOPEN){
                token_t * top = D_ARRAY_END(token_t, (*state)->stack);
                d_array_insert(&(*state)->output, top);
                d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            }
            if(D_ARRAY_LEN((*state)->stack)==0){
                return PARSE_ERROR_UNMATCHEDBRACKET;
            }
            if(D_ARRAY_END(token_t, (*state)->stack)->type!=TOKENTYPE_BRACKOPEN){
                return PARSE_ERROR_UNMATCHEDBRACKET;
            }
            d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            if(D_ARRAY_END(token_t, (*state)->stack)->type==TOKENTYPE_FUNCTION){
                token_t * top = D_ARRAY_END(token_t, (*state)->stack);
                d_array_insert(&(*state)->output, top);
                d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            }
        }
    }
    while(D_ARRAY_LEN((*state)->stack)>0){
        if(D_ARRAY_END(token_t, (*state)->stack)->type==TOKENTYPE_BRACKOPEN){
            return PARSE_ERROR_UNMATCHEDBRACKET;
        }
        token_t * top = D_ARRAY_END(token_t, (*state)->stack);
        d_array_insert(&(*state)->output, top);
        d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
    }

    return PARSE_ERROR_OK;
}

parse_error_e parser_cleanup(parserstate_t ** state){
    if(*state==NULL) return PARSE_ERROR_STATEISNULL;

    // Free all tokens if needed
    for(D_ARRAY_LOOP(token_t, tok, (*state)->output)){
        if(tok->type==TOKENTYPE_VARIABLE) free(tok->svalue);
    }
    for(D_ARRAY_LOOP(token_t, tok, (*state)->stack)){
        if(tok->type==TOKENTYPE_VARIABLE) free(tok->svalue);
    }
    
    d_array_deinit(&(*state)->output);
    d_array_deinit(&(*state)->stack);

    free(*state);
    *state = NULL;

    return PARSE_ERROR_OK;
}
