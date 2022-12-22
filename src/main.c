#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynamic_array.h"

typedef enum{
    TOKENTYPE_NUMBER,
    TOKENTYPE_OPERATOR,
    TOKENTYPE_FUNCTION,
    TOKENTYPE_BRACKOPEN,
    TOKENTYPE_BRACKCLOSE,
    TOKENTYPE_VARIABLE,
    TOKENTYPE_EOF,
    TOKENTYPE_ERROR,
} tokentype_e;

typedef enum{
    FUNCTION_SIN = 0,
    FUNCTION_COS,
    FUNCTION_TAN,
    FUNCTION_STEP,
    _FUNCTION_SIZE_,
} function_e;
const char * functions[_FUNCTION_SIZE_] = {
    "sin",
    "cos",
    "tan",
    "step",
};

typedef enum{
    OPERATION_PLUS = 0,
    OPERATION_MINUS,
    OPERATION_MULTIPLY,
    OPERATION_DIVIDE,
    OPERATION_POW,
    OPERATION_MOD,
    _OPERATION_SIZE_,
} operation_e;
typedef enum{
    OPERATION_ASS_LEFT,
    OPERATION_ASS_RIGHT,
} operation_ass_e;
typedef struct{
    char opchar;
    operation_e op;
    operation_ass_e ass;
    int precedence;
} operation_t;
const operation_t operations[_OPERATION_SIZE_] = {
    {'+', OPERATION_PLUS, OPERATION_ASS_LEFT, 2},
    {'-', OPERATION_MINUS, OPERATION_ASS_LEFT, 2},
    {'*', OPERATION_MULTIPLY, OPERATION_ASS_LEFT, 3},
    {'/', OPERATION_DIVIDE, OPERATION_ASS_LEFT, 3},
    {'^', OPERATION_POW, OPERATION_ASS_RIGHT, 4},
    {'%', OPERATION_MOD, OPERATION_ASS_LEFT, 3},
};

typedef struct token_s{
    tokentype_e type;
    union{
        double dvalue;
        function_e fvalue;
        operation_t ovalue;
        char * svalue;
    };
} token_t;

typedef struct{
    char * string;
    size_t position;
    d_array_t output;
    d_array_t stack;
} parserstate_t;

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
        if(((c>='a' && c<='z') || (c>='A' && c<='Z')) && ntok.type!=TOKENTYPE_NUMBER){
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

int main(int argc, char ** argv){

    parserstate_t state = {0};
    state.string = argv[1];

    D_ARRAY_INIT(token_t, &state.output);
    D_ARRAY_INIT(token_t, &state.stack);

    token_t t;
    int error = 0;
    while(1){
        t = nexttoken(&state);
        
        if(t.type==TOKENTYPE_EOF || t.type==TOKENTYPE_ERROR){
            break;
        } else if(t.type == TOKENTYPE_NUMBER || t.type == TOKENTYPE_VARIABLE){
            d_array_insert(&state.output, &t);
        } else if(t.type == TOKENTYPE_FUNCTION){
            d_array_insert(&state.stack, &t);
        } else if(t.type == TOKENTYPE_OPERATOR){
            while(
                    D_ARRAY_LEN(state.stack)>0 && 
                    D_ARRAY_END(token_t, state.stack)->type!=TOKENTYPE_BRACKOPEN &&
                    (D_ARRAY_END(token_t, state.stack)->ovalue.precedence > t.ovalue.precedence ||
                    (D_ARRAY_END(token_t, state.stack)->ovalue.precedence==t.ovalue.precedence && t.ovalue.ass==OPERATION_ASS_LEFT))){
                token_t * top = D_ARRAY_END(token_t, state.stack);
                d_array_insert(&state.output, top);
                d_array_erase(&state.stack, D_ARRAY_LEN(state.stack)-1);
            }
            d_array_insert(&state.stack, &t);
        } else if(t.type == TOKENTYPE_BRACKOPEN){
            d_array_insert(&state.stack, &t);
        } else if(t.type == TOKENTYPE_BRACKCLOSE){
            if(D_ARRAY_LEN(state.stack)==0){
                printf("ERROR: stack empty and not found (\n");
                error = 1;
                break;
            }
            while(
                    D_ARRAY_LEN(state.stack)>0 &&
                    D_ARRAY_END(token_t, state.stack)->type!=TOKENTYPE_BRACKOPEN){
                token_t * top = D_ARRAY_END(token_t, state.stack);
                d_array_insert(&state.output, top);
                d_array_erase(&state.stack, D_ARRAY_LEN(state.stack)-1);
            }
            if(D_ARRAY_END(token_t, state.stack)->type!=TOKENTYPE_BRACKOPEN){
                printf("ERROR: unmatched ) (could not find ( in stack)\n");
                error = 1;
                break;
            }
            d_array_erase(&state.stack, D_ARRAY_LEN(state.stack)-1);
            if(D_ARRAY_END(token_t, state.stack)->type==TOKENTYPE_FUNCTION){
                token_t * top = D_ARRAY_END(token_t, state.stack);
                d_array_insert(&state.output, top);
                d_array_erase(&state.stack, D_ARRAY_LEN(state.stack)-1);
            }
        }
    }
    while(D_ARRAY_LEN(state.stack)>0){
        if(D_ARRAY_END(token_t, state.stack)->type==TOKENTYPE_BRACKOPEN){
            printf("ERROR: unmatched ) (could not find ( in stack at end of loop)\n");
            error = 1;
            break;
        }
        token_t * top = D_ARRAY_END(token_t, state.stack);
        d_array_insert(&state.output, top);
        d_array_erase(&state.stack, D_ARRAY_LEN(state.stack)-1);
    }

    printf("OUTPUT:\n");
    for(D_ARRAY_LOOP(token_t, tok, state.output)){
        print_token(*tok);
        if(tok->type==TOKENTYPE_VARIABLE) free(tok->svalue);
    }

    if(error){
        printf("ERROR: syntax error\n");
    }


    d_array_deinit(&state.output);
    d_array_deinit(&state.stack);
    return 0;
}
