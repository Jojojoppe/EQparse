#include "mathparse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const function_t functions[_FUNCTION_SIZE_] = {
    {"sin", FUNCTION_SIN, 1},
    {"cos", FUNCTION_COS, 1},
    {"tan", FUNCTION_TAN, 1},
    {"step", FUNCTION_STEP, 1},
    {"int", FUNCTION_INT, 1},
    {"ddt", FUNCTION_DDT, 1},
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
    int ignore_minus = 0;

    token_t ntok = {
        .type = TOKENTYPE_EOF,
        .dvalue = 0,
    };

    while(1){
        char c = state->string[state->position++];
        /*printf("c=%c\n", c);*/

        // Ignore whitespaces
        if(c=='\t' || c==' ' || c=='\n'){
            continue;
        }

        // Add characters to buf when not in NUMBER mode
        else if(((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_') && ntok.type!=TOKENTYPE_NUMBER){
            // Check if a - is added, then must correct this action since we are not in number mode
            if(buf[0]=='-' && bufpos!=0){
                state->position-=2;
                ignore_minus=1;
                continue;
            }
            buf[bufpos++] = c;
            ntok.type = TOKENTYPE_VARIABLE;
            continue;
        }

        // If came here while ntok is VARIABLE that means end of variable or function
        if(ntok.type == TOKENTYPE_VARIABLE){
            // Check to see if buf contains known function name
            for(int i=0; i<_FUNCTION_SIZE_; i++){
                if(!strcmp(functions[i].name, buf)){
                    ntok.type = TOKENTYPE_FUNCTION;
                    ntok.fvalue = functions[i];
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
        // if it is a '-' add to buffer (only when not in the middle of a number, previous was a number or variable
        else if(ignore_minus!=1 && c=='-' && ntok.type!=TOKENTYPE_NUMBER && (state->ptok.type!=TOKENTYPE_NUMBER && state->ptok.type!=TOKENTYPE_VARIABLE)){
            buf[bufpos++] = c;
            ignore_minus = 0;
            continue;
        } else if(c>='0' && c<='9'){
            buf[bufpos++] = c;
            ntok.type = TOKENTYPE_NUMBER;
            continue;
        }else if((c=='.' || c=='e' || c=='E') && ntok.type==TOKENTYPE_NUMBER){
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
        parseoperation:
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

        // comma
        else if(c==','){
            ntok.type = TOKENTYPE_COMMA;
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
    state->ptok = ntok;
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
            printf("function  ] %s\n", t.fvalue.name);
            break;
        case TOKENTYPE_VARIABLE:
            printf("variable  ] %s\n", t.svalue);
            /*free(t.svalue);*/
            break;
        case TOKENTYPE_COMMA:
            printf("comma     ] ,\n");
            break;
        default:
            printf("ERROR: unknown token type\n");
            break;
    }
}

parse_error_e parse(const char * string, size_t len, parserstate_t ** state){
    if(string==NULL) return PARSE_ERROR_STRINGISNULL;
    /*if(*state!=NULL) return PARSE_ERROR_STATEALREADYINITIALIZED;*/
    if(len==0) return PARSE_ERROR_STRINGLENISZERO;

    if(*state!=NULL) parser_cleanup(state);

    *state = calloc(sizeof(parserstate_t), 1);
    D_ARRAY_INIT(token_t, &(*state)->output);
    D_ARRAY_INIT(token_t, &(*state)->stack);
    (*state)->string = string;
    (*state)->ptok.type = TOKENTYPE_EOF;

    // Check for starting with bracket (causes error somehow)
    /*if(string[0]=='(') return PARSE_ERROR_STARTSWITHBRACKET;*/

    d_array_t funcstack, funcstackargs;
    D_ARRAY_INIT(token_t, &funcstack);
    D_ARRAY_INIT(int, &funcstackargs);
    int bracketexpected = 0;

    parse_error_e err = PARSE_ERROR_OK;

    token_t t;
    while(1){
        t = nexttoken(*state);

        /*print_token(t);*/

        if(bracketexpected && t.type!=TOKENTYPE_BRACKOPEN){
            err = PARSE_ERROR_EXPECTEDOPENBRACKED;
            goto parseend;
        }

        if(t.type==TOKENTYPE_EOF || t.type==TOKENTYPE_ERROR){
            break;
        } else if(t.type == TOKENTYPE_NUMBER || t.type == TOKENTYPE_VARIABLE){
            d_array_insert(&(*state)->output, &t);
        } else if(t.type == TOKENTYPE_FUNCTION){
            d_array_insert(&(*state)->stack, &t);
            d_array_insert(&funcstack, &t);
            int args = 0;
            d_array_insert(&funcstackargs, &args);
            bracketexpected = 1;
        } else if(t.type == TOKENTYPE_COMMA){
            // Check if amount of parameters is not too much
            if(D_ARRAY_LEN(funcstack)==0){
                err = PARSE_ERROR_UNEXPECTEDCOMMA;
                goto parseend;
            }
            if(*D_ARRAY_END(int, funcstackargs)+2>D_ARRAY_END(token_t, funcstack)->fvalue.arguments){
                err = PARSE_ERROR_TOOMANYARGS;
                goto parseend;
            }
            (*D_ARRAY_END(int, funcstackargs))++;
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
            t.bisfunc = 0;
            if(bracketexpected){
                bracketexpected = 0;
                t.bisfunc = 1;
            }
            d_array_insert(&(*state)->stack, &t);
        } else if(t.type == TOKENTYPE_BRACKCLOSE){
            if(D_ARRAY_LEN((*state)->stack)==0){
                err = PARSE_ERROR_UNMATCHEDBRACKET;
                goto parseend;
            }
            while(
                    D_ARRAY_LEN((*state)->stack)>0 &&
                    D_ARRAY_END(token_t, (*state)->stack)->type!=TOKENTYPE_BRACKOPEN){
                token_t * top = D_ARRAY_END(token_t, (*state)->stack);
                d_array_insert(&(*state)->output, top);
                d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            }
            if(D_ARRAY_LEN((*state)->stack)==0){
                err = PARSE_ERROR_UNMATCHEDBRACKET;
                goto parseend;
            }
            if(D_ARRAY_END(token_t, (*state)->stack)->type!=TOKENTYPE_BRACKOPEN){
                err = PARSE_ERROR_UNMATCHEDBRACKET;
                goto parseend;
            }
            // If the bracket is accompanied by a function pop from funcstacks
            if(D_ARRAY_END(token_t, (*state)->stack)->bisfunc==1){
                // Check if enough parameters
                if(*D_ARRAY_END(int, funcstackargs)+2<=D_ARRAY_END(token_t, funcstack)->fvalue.arguments){
                    err = PARSE_ERROR_TOOLITTLEARGS;
                    goto parseend;
                }
                d_array_erase(&funcstack, D_ARRAY_LEN(funcstack)-1);
                d_array_erase(&funcstackargs, D_ARRAY_LEN(funcstackargs)-1);
            }
            d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            if(D_ARRAY_LEN((*state)->stack)>0 && D_ARRAY_END(token_t, (*state)->stack)->type==TOKENTYPE_FUNCTION){
                token_t * top = D_ARRAY_END(token_t, (*state)->stack);
                d_array_insert(&(*state)->output, top);
                d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
            }
        }
    }
    while(D_ARRAY_LEN((*state)->stack)>0){
        if(D_ARRAY_END(token_t, (*state)->stack)->type==TOKENTYPE_BRACKOPEN){
            err = PARSE_ERROR_UNMATCHEDBRACKET;
            goto parseend;
        }
        token_t * top = D_ARRAY_END(token_t, (*state)->stack);
        d_array_insert(&(*state)->output, top);
        d_array_erase(&(*state)->stack, D_ARRAY_LEN((*state)->stack)-1);
    }

parseend:
    d_array_deinit(&funcstack);
    d_array_deinit(&funcstackargs);
    return err;
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
