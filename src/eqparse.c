#include "eqparse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum{
    NEXT_TOKEN_MODE_IDLE,
    NEXT_TOKEN_MODE_NUMBER,
    NEXT_TOKEN_MODE_STRING
};

token_t next_token(eqparse_t * eq){
    token_t tok = {.tok=TOKEN_NULL, .type=TOKEN_TYPE_NULL};
    char buf[64] = {0};
    size_t bufpos = 0;

    int mode = NEXT_TOKEN_MODE_IDLE;

    int number_decimal = 0;
    int number_scientific = 0;

    while(1){
        // EOF
        if(eq->position==eq->slen){
            if(mode==NEXT_TOKEN_MODE_NUMBER){
                goto token_next_emit_number;
            }else if(mode==NEXT_TOKEN_MODE_STRING){
                goto token_next_emit_string;
            }
            // Not busy with a token yet so just emit EOF
            tok.tok = TOKEN_EOF;
            break;
        }
        char c = eq->string[eq->position++];

        if(mode==NEXT_TOKEN_MODE_IDLE){

            // Check if start of number
            if(c>='0' && c<='9'){
                buf[bufpos++] = c;
                mode = NEXT_TOKEN_MODE_NUMBER;
                continue;
            }

            // Check if start of string
            else if((c>='a' && c<='z') || (c>='A' && c<='Z')){
                buf[bufpos++] = c;
                mode = NEXT_TOKEN_MODE_STRING;
                continue;
            }

            // Check if simple character
            else if(c=='+') {tok.tok = TOKEN_PLUS; break;}
            else if(c=='*') {tok.tok = TOKEN_TIMES; break;}
            else if(c=='/') {tok.tok = TOKEN_DIVIDE; break;}
            else if(c=='^') {tok.tok = TOKEN_POWER; break;}
            else if(c=='%') {tok.tok = TOKEN_MODULO; break;}
            else if(c=='(') {tok.tok = TOKEN_BROPEN; break;}
            else if(c==')') {tok.tok = TOKEN_BRCLOSE; break;}
            else if(c==',') {tok.tok = TOKEN_COMMA; break;}
            else if(c=='=') {tok.tok = TOKEN_EQUAL; break;}

            // Check if minus is token or part of number
            else if(c=='-'){
                if(
                        eq->_prev.tok==TOKEN_NULL || 
                        eq->_prev.tok==TOKEN_PLUS ||
                        eq->_prev.tok==TOKEN_MINUS ||
                        eq->_prev.tok==TOKEN_TIMES ||
                        eq->_prev.tok==TOKEN_DIVIDE ||
                        eq->_prev.tok==TOKEN_MODULO ||
                        eq->_prev.tok==TOKEN_EQUAL ||
                        eq->_prev.tok==TOKEN_BROPEN ||
                        eq->_prev.tok==TOKEN_COMMA
                        ){
                    buf[0] = c;
                    bufpos+=1;
                    mode == NEXT_TOKEN_MODE_NUMBER;
                    continue;
                }
                tok.tok = TOKEN_MINUS;
                break;
            }

            // Ignore whitespaces
            else if(c==' ' || c=='\t' || c=='\n'){
                eq->_prev.tok = TOKEN_NULL;
                continue;
            }

            // Unknown character
            tok.cvalue = c;
            break;

        } else if(mode==NEXT_TOKEN_MODE_NUMBER){
            // Normal number
            if(c>='0' && c<='9'){
                buf[bufpos++] = c;
            // Try for decimal point
            } else if(number_decimal==0 && c=='.'){
                buf[bufpos++] = c;
                number_decimal = 1;
            // Try for scientific notation
            } else if(number_scientific==0 && (c=='e' || c=='E')){
                buf[bufpos++] = c;
                number_scientific = 1;
            // If last character added was for scientific, accept a minus sign
            } else if(c=='-' && (buf[bufpos-1]=='e' || buf[bufpos-1]=='E')){
                buf[bufpos++] = c;
            } else{
                eq->position--;
                token_next_emit_number:
                // NO CHARACTER FOR NUMBER SO EMIT TOKEN
                buf[bufpos] = 0;
                if(number_decimal||number_scientific){
                    tok.tok = TOKEN_DNUMBER;
                    sscanf(buf, "%lf", &tok.dvalue);
                }else{
                    tok.tok = TOKEN_INUMBER;
                    sscanf(buf, "%ld", &tok.ivalue);
                }
                break;
            }
        } else if(mode==NEXT_TOKEN_MODE_STRING){
            // string characters
            if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_'){
                buf[bufpos++] = c;
            } else{
                eq->position--;
                token_next_emit_string:
                // NO CHARACTER FOR STRING SO EMIT TOKEN
                buf[bufpos] = 0;
                tok.tok = TOKEN_STRING;
                tok.svalue = calloc(bufpos+1, 1);
                strcpy(tok.svalue, buf);
                break;
            }
        }
    }

    eq->_prev = tok;
    return tok;
}

void _clean_ast(ast_t * ast){
    if(ast==NULL) return;
    D_ARRAY_FOREACH_BT(ast_t, n, &ast->children){
        _clean_ast(n);
    }
    if(ast->tok.tok==TOKEN_STRING || ast->tok.type==TOKEN_TYPE_FUNCTION){
        free(ast->tok.svalue);
    }
    d_array_destroy(&ast->children);
}

int parse_tokens(eqparse_t * eq){
    token_t tok;

    d_stack_t oprstack, expstack;
    int err = 0;
    err |= D_STACK_CREATE(ast_t, &oprstack);
    err |= D_STACK_CREATE(ast_t, &expstack);
    if(err){
        err = EQPARSE_ERROR_INTERNAL;
        goto parse_tokens_return;
    }

    int equal = 0;

    while((tok=next_token(eq)).tok!=TOKEN_EOF){

        /*printf("next: ");*/
        /*debug_print_token(&tok);*/
        /*printf("\n");*/

        // Check for unknown character
        if(tok.tok==TOKEN_NULL){
            eq->token_error = tok;
            err = EQPARSE_ERROR_CHARACTER;
            goto parse_tokens_return;
        }

        // double number -> VALUE
        else if(tok.tok==TOKEN_DNUMBER){
            tok.type = TOKEN_TYPE_VALUE;
            ast_t n = {.tok=tok};
            D_ARRAY_CREATE(ast_t, &n.children);
            d_stack_push(&expstack, &n);
            continue;
        }
        // integer number -> VALUE
        else if(tok.tok==TOKEN_INUMBER){
            tok.type = TOKEN_TYPE_VALUE;
            ast_t n = {.tok=tok};
            D_ARRAY_CREATE(ast_t, &n.children);
            d_stack_push(&expstack, &n);
            continue;
        }

        // (
        else if(tok.tok==TOKEN_BROPEN){
            ast_t n = {.tok=tok};
            D_ARRAY_CREATE(ast_t, &n.children);
            d_stack_push(&oprstack, &n);
            continue;
        }

        // )
        else if(tok.tok==TOKEN_BRCLOSE){
            while(
                    oprstack.length && 
                    /*D_STACK_TOP(ast_t, &oprstack)->tok.type==TOKEN_TYPE_OPERATOR &&*/
                    D_STACK_TOP(ast_t, &oprstack)->tok.tok!=TOKEN_BROPEN){
                // pop op, e2 and e1
                ast_t op = *D_STACK_POP_DATA(ast_t, &oprstack);
                if(op.tok.type == TOKEN_TYPE_OPERATOR || op.tok.type == TOKEN_TYPE_EQUALITY){
                    if(expstack.length<2){
                        eq->token_error = tok;
                        printf("A\n");
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    ast_t e1 = *D_STACK_POP_DATA(ast_t, &expstack);
                    ast_t e2 = *D_STACK_POP_DATA(ast_t, &expstack);
                    d_array_insert(&op.children, &e1);
                    d_array_insert(&op.children, &e2);
                    d_stack_push(&expstack, &op);
                } else if(op.tok.type == TOKEN_TYPE_FUNCTION){
                    int args = eqparse_functions[op.tok.tok].arguments;
                    if(expstack.length<args){
                        eq->token_error = tok;
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    for(int i=0; i<args; i++){
                        ast_t e = *D_STACK_POP_DATA(ast_t, &expstack);
                        d_array_insert(&op.children, &e);
                    }
                    d_stack_push(&expstack, &op);
                }
            }
            // Check if still in oprstack, if not unmatched )
            if(!oprstack.length){
                eq->token_error = tok;
                err = EQPARSE_ERROR_UNMATCHED;
                goto parse_tokens_return;
            }
            ast_t * br = D_STACK_POP_DATA(ast_t, &oprstack);
            d_array_destroy(&br->children);
        }

        // Operator -> OPERATOR (or =)
        else if(
                tok.tok==TOKEN_EQUAL ||
                tok.tok==TOKEN_PLUS ||
                tok.tok==TOKEN_MINUS ||
                tok.tok==TOKEN_TIMES ||
                tok.tok==TOKEN_DIVIDE ||
                tok.tok==TOKEN_POWER ||
                tok.tok==TOKEN_MODULO){
            int operator_i = tok.tok - TOKEN_PLUS;
            while(
                    oprstack.length>0 && 
                    ((
                      (D_STACK_TOP(ast_t, &oprstack)->tok.type==TOKEN_TYPE_OPERATOR ||
                       D_STACK_TOP(ast_t, &oprstack)->tok.type==TOKEN_TYPE_EQUALITY
                      ) &&
                    eqparse_operators[D_STACK_TOP(ast_t, &oprstack)->tok.tok-TOKEN_PLUS].precedence >= eqparse_operators[operator_i].precedence) ||
                    D_STACK_TOP(ast_t, &oprstack)->tok.type==TOKEN_TYPE_FUNCTION)){
                // pop op, e2 and e1
                ast_t op = *D_STACK_POP_DATA(ast_t, &oprstack);
                if(op.tok.type == TOKEN_TYPE_OPERATOR || op.tok.type == TOKEN_TYPE_EQUALITY){
                    if(expstack.length<2){
                        eq->token_error = tok;
                        printf("B\n");
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    ast_t e1 = *D_STACK_POP_DATA(ast_t, &expstack);
                    ast_t e2 = *D_STACK_POP_DATA(ast_t, &expstack);
                    d_array_insert(&op.children, &e1);
                    d_array_insert(&op.children, &e2);
                    d_stack_push(&expstack, &op);
                } else if(op.tok.type == TOKEN_TYPE_FUNCTION){
                    int args = eqparse_functions[op.tok.tok].arguments;
                    if(expstack.length<args){
                        eq->token_error = tok;
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    for(int i=0; i<args; i++){
                        ast_t e = *D_STACK_POP_DATA(ast_t, &expstack);
                        d_array_insert(&op.children, &e);
                    }
                    d_stack_push(&expstack, &op);
                }
            }
            if(tok.tok==TOKEN_EQUAL){
                if(equal){
                    eq->token_error = tok;
                    err = EQPARSE_ERROR_INVALID;
                    goto parse_tokens_return;
                }
                equal = 1;
                tok.type = TOKEN_TYPE_EQUALITY;
            }else{
                tok.type = TOKEN_TYPE_OPERATOR;
            }
            ast_t n = {.tok=tok};
            D_ARRAY_CREATE(ast_t, &n.children);
            d_stack_push(&oprstack, &n);
        }

        // If string we must check if function, constant or variable
        else if(tok.tok==TOKEN_STRING){
            // Check constants -> VALUE
            for(int i=0; i<EQPARSE_CONSTANT_INFO_N; i++){
                if(!strcmp(tok.svalue, eqparse_constants[i].name)){
                    tok.type = TOKEN_TYPE_VALUE;
                    ast_t n = {.tok=tok};
                    D_ARRAY_CREATE(ast_t, &n.children);
                    d_stack_push(&expstack, &n);
                    goto parse_string_continue;
                }
            }
            // Check for functions -> FUNCTION
            for(int i=0; i<EQPARSE_FUNCTION_INFO_N; i++){
                if(!strcmp(tok.svalue, eqparse_functions[i].name)){
                    tok.type = TOKEN_TYPE_FUNCTION;
                    tok.tok = i; // misuse the token field for function entry
                    ast_t n = {.tok = tok};
                    D_ARRAY_CREATE(ast_t, &n.children);
                    d_stack_push(&oprstack, &n);
                    goto parse_string_continue;
                }
            }
            // No constant or function -> variable -> VALUE
            tok.type = TOKEN_TYPE_VALUE;
            ast_t n = {.tok=tok};
            D_ARRAY_CREATE(ast_t, &n.children);
            d_stack_push(&expstack, &n);
            parse_string_continue:
            continue;
        }
    }

    while(oprstack.length){
        // pop op, e2 and e1
        ast_t op = *D_STACK_POP_DATA(ast_t, &oprstack);
        if(op.tok.tok==TOKEN_BROPEN || op.tok.tok==TOKEN_BRCLOSE){
            eq->token_error = op.tok;
            err = EQPARSE_ERROR_UNMATCHED;
            goto parse_tokens_return;
        }
        if(op.tok.type == TOKEN_TYPE_OPERATOR || op.tok.type == TOKEN_TYPE_EQUALITY){
            if(expstack.length<2){
                printf("C\n");
                eq->token_error = tok;
                err = EQPARSE_ERROR_EXPRESSION;
                goto parse_tokens_return;
            }
            ast_t e1 = *D_STACK_POP_DATA(ast_t, &expstack);
            ast_t e2 = *D_STACK_POP_DATA(ast_t, &expstack);
            d_array_insert(&op.children, &e1);
            d_array_insert(&op.children, &e2);
            d_stack_push(&expstack, &op);
        } else if(op.tok.type == TOKEN_TYPE_FUNCTION){
            int args = eqparse_functions[op.tok.tok].arguments;
            if(expstack.length<args){
                eq->token_error = tok;
                err = EQPARSE_ERROR_EXPRESSION;
                goto parse_tokens_return;
            }
            for(int i=0; i<args; i++){
                ast_t e = *D_STACK_POP_DATA(ast_t, &expstack);
                d_array_insert(&op.children, &e);
            }
            d_stack_push(&expstack, &op);
        }
    }

    // Last checks
    if(!equal){
        err = EQPARSE_ERROR_INVALID;
        goto parse_tokens_return;
    }
    if(D_STACK_TOP(ast_t, &expstack)->tok.type!=TOKEN_TYPE_EQUALITY){
        err = EQPARSE_ERROR_INVALID;
        goto parse_tokens_return;
    }
    if(expstack.length>1 || oprstack.length>0){
        err = EQPARSE_ERROR_INVALID;
        goto parse_tokens_return;
    }

    eq->AST = *D_STACK_TOP(ast_t, &expstack);

    err = EQPARSE_ERROR_OK;
parse_tokens_return:

    /*printf("EXPSTACK:\n");*/
    int i=0;
    D_STACK_FOREACH_TB(ast_t, n, &expstack){
        /*debug_print_ast(n);*/
        if(i) _clean_ast(n);
        i++;
    }
    /*printf("OPRSTACK:\n");*/
    D_STACK_FOREACH_TB(ast_t, n, &oprstack){
        /*debug_print_ast(n);*/
        _clean_ast(n);
    }

    d_stack_destroy(&oprstack);
    d_stack_destroy(&expstack);
    return err;
}

void debug_print_token(token_t * token){
    switch(token->tok){
        case TOKEN_DNUMBER: printf("%.2f ", token->dvalue); break;
        case TOKEN_INUMBER: printf("%ld ", token->ivalue); break;
        case TOKEN_PLUS: printf("+ "); break;
        case TOKEN_MINUS: printf("- "); break;
        case TOKEN_TIMES: printf("* "); break;
        case TOKEN_DIVIDE: printf("/ "); break;
        case TOKEN_POWER: printf("^ "); break;
        case TOKEN_MODULO: printf("%% "); break;
        case TOKEN_BROPEN: printf("( "); break;
        case TOKEN_BRCLOSE: printf(") "); break;
        case TOKEN_COMMA: printf(", "); break;
        case TOKEN_EQUAL: printf("= "); break;
        case TOKEN_STRING: printf("%s ", token->svalue); break;
        case TOKEN_EOF: printf("EOF "); break;
        case TOKEN_NULL: printf("NULL:%c ", token->cvalue); break;
        default: printf("? "); break;
    }
}

void _debug_print_ast(ast_t * ast){
    if(ast==NULL) return;
    switch(ast->tok.type){
        case TOKEN_TYPE_VALUE:
            if(ast->tok.tok==TOKEN_DNUMBER) printf("%.2f", ast->tok.dvalue);
            else if(ast->tok.tok==TOKEN_INUMBER) printf("%d", ast->tok.ivalue);
            else if(ast->tok.tok==TOKEN_STRING) printf("%s", ast->tok.svalue);
            return;
        case TOKEN_TYPE_OPERATOR:
            printf("%s(", eqparse_operators[ast->tok.tok-TOKEN_PLUS].s);
            break;
        case TOKEN_TYPE_FUNCTION:
            printf("%s(", eqparse_functions[ast->tok.tok].name);
            break;
        case TOKEN_TYPE_EQUALITY:
            printf("EQ(");
            break;
        default: printf("?(");
    }
    D_ARRAY_FOREACH_TB(ast_t, child, &ast->children){
        _debug_print_ast(child);
        printf(", ");
    }
    printf("\b\b)");
}
void debug_print_ast(ast_t * ast){
    _debug_print_ast(ast);
    printf("\n");
}

void _debug_write_ast(ast_t * ast, FILE * f, char * parent, int i){
    if(ast==NULL) return;
    if(ast->tok.type==TOKEN_TYPE_EQUALITY){
        fprintf(f, "%s%d [label=\"=\"]\n", parent, i);
        fprintf(f, "%s -> %s%d\n", parent, parent, i);
    }else if(ast->tok.type==TOKEN_TYPE_OPERATOR){
        fprintf(f, "%s%d [label=\"%c\"]\n", parent, i, eqparse_operators[ast->tok.tok-TOKEN_PLUS].c);
        fprintf(f, "%s -> %s%d\n", parent, parent, i);
    }else if(ast->tok.type==TOKEN_TYPE_VALUE){
        if(ast->tok.tok==TOKEN_DNUMBER){
            fprintf(f, "%s%d [label=\"%lf\"]\n", parent, i, ast->tok.dvalue);
        }
        else if(ast->tok.tok==TOKEN_INUMBER){
            fprintf(f, "%s%d [label=\"%ld\"]\n", parent, i, ast->tok.ivalue);
        }
        else if(ast->tok.tok==TOKEN_STRING){
            fprintf(f, "%s%d [label=\"%s\"]\n", parent, i, ast->tok.svalue);
        }
        fprintf(f, "%s -> %s%d\n", parent, parent, i);
    }else if(ast->tok.type==TOKEN_TYPE_FUNCTION){
        fprintf(f, "%s%d [label=\"%s\"]\n", parent, i, ast->tok.svalue);
        fprintf(f, "%s -> %s%d\n", parent, parent, i);
    }

    int j=0;
    char * name = calloc(strlen(parent)+5, 1);
    sprintf(name, "%s%d", parent, i);
    if(ast->children.length){
        D_ARRAY_FOREACH_TB(ast_t, n, &ast->children){
            _debug_write_ast(n, f, name, j++);
        }
    }
    free(name);
}
void debug_write_ast(ast_t * ast, const char * file, const char * title){
    FILE * f = fopen(file, "w");
    if(!f) return;
    fprintf(f, "digraph{\nroot [label=\"%s\"]\n", title);
    _debug_write_ast(ast, f, "root", 0);
    fprintf(f, "}\n");
    fclose(f);
}
void debug_write_eq(eqparse_t * eq, const char * file){
    debug_write_ast(&eq->AST, file, eq->string);
}

eqparse_t * eqparse(char * string, size_t slen, int * err){
    if(string==NULL) {if(err) *err = EQPARSE_ERROR_PARAM; return NULL; }
    if(strlen(string)==0) {if(err) *err = EQPARSE_ERROR_PARAM; return NULL;}
    if(slen==0) slen=strlen(string);

    eqparse_t * eq = calloc(sizeof(eqparse_t), 1);
    if(eq==NULL) return NULL;

    eq->string = calloc(slen+1, 1);
    if(eq->string==NULL){
        free(eq);
        if(err) *err = EQPARSE_ERROR_INTERNAL;
        return NULL;
    }

    eq->AST.tok.tok = TOKEN_NULL;

    memcpy(eq->string, string, slen);
    eq->slen = slen;

    int rval = parse_tokens(eq);
    if(rval){
        if(err) *err = rval;
        return eq;
    }

    return eq;
}

void eqparse_cleanup(eqparse_t * eq){
    if(eq==NULL) return;
    if(eq->string) free(eq->string);
    _clean_ast(&eq->AST);
    free(eq);
}
