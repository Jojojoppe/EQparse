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
                // Check if unary minus is there
                if(buf[0]=='-'){
                    eq->position--;
                    tok.tok = TOKEN_MINUS;
                    break;
                }
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
    if(ast->tok.tok==TOKEN_STRING){
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
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    ast_t e1 = *D_STACK_POP_DATA(ast_t, &expstack);
                    ast_t e2 = *D_STACK_POP_DATA(ast_t, &expstack);
                    d_array_insert(&op.children, &e1);
                    d_array_insert(&op.children, &e2);
                    d_stack_push(&expstack, &op);
                } else if(op.tok.type == TOKEN_TYPE_FUNCTION){
                    int args = eqparse_functions[op.tok.ivalue].arguments;
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
                        err = EQPARSE_ERROR_EXPRESSION;
                        goto parse_tokens_return;
                    }
                    ast_t e1 = *D_STACK_POP_DATA(ast_t, &expstack);
                    ast_t e2 = *D_STACK_POP_DATA(ast_t, &expstack);
                    d_array_insert(&op.children, &e1);
                    d_array_insert(&op.children, &e2);
                    d_stack_push(&expstack, &op);
                } else if(op.tok.type == TOKEN_TYPE_FUNCTION){
                    int args = eqparse_functions[op.tok.ivalue].arguments;
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
                    tok.tok = TOKEN_NULL;
                    /*tok.tok = i; // misuse the token field for function entry*/
                    free(tok.svalue);
                    tok.ivalue = i;
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
            int args = eqparse_functions[op.tok.ivalue].arguments;
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


// -----------------------
// normalisation, simplification etc

int _eqparse_equivalence(ast_t * node, const eqrule_term * term){
    if(node==NULL) return 0;

    // Check for notes
    if(term->note){
        if(term->note==EQRULE_NOTE_NO_NUMBER && (node->tok.tok==TOKEN_INUMBER || node->tok.tok==TOKEN_DNUMBER)) return 0;
    }

    // if term is DNC return match
    if(term->token==TOKEN_NULL && term->type==TOKEN_TYPE_NULL) return 1;

    // Check type and token, if not equal no match
    if(node->tok.tok!=term->token || node->tok.type!=term->type) return 0;

    // Check valued and if so value
    if(term->valued && term->type==TOKEN_TYPE_VALUE){
        if(term->token==TOKEN_INUMBER && term->ivalue!=node->tok.ivalue) return 0;
        if(term->token==TOKEN_DNUMBER && term->dvalue!=node->tok.dvalue) return 0;
    }

    // Check for same amount of children
    if(term->nrterms!=node->children.length) return 0;

    int childmatch = 1;

    for(int i=0; i<term->nrterms; i++){
        childmatch &= _eqparse_equivalence(D_ARRAY_AT(ast_t, &node->children, term->nrterms-i-1), &term->terms[i]);
        if(!childmatch) break;
    }

    return childmatch;
}

void _eqparse_apply_rule_visit(ast_t * node, const eqrule_term * term, d_array_t * interested){
    if(term->interested>0){
        d_array_insert(interested, node);
    }
    for(int i=0; i<term->nrterms; i++){
        _eqparse_apply_rule_visit(D_ARRAY_AT(ast_t, &node->children, term->nrterms-i-1), &term->terms[i], interested);
    }
}

void _eqparse_apply_rule_create(ast_t * node, const eqrule_term * term, d_array_t * interested){
    if(node==NULL || term==NULL) return;
    if(term->interested){
        memcpy(node, D_ARRAY_AT(ast_t, interested, term->interested-1), sizeof(ast_t));
        return;
    }

    token_t tok = {
        .tok = term->token,
        .type = term->type,
    };
    node->tok = tok;

    if(term->valued){
        if(term->token==TOKEN_INUMBER) tok.ivalue = term->ivalue;
        else if(term->token==TOKEN_DNUMBER) tok.dvalue = term->dvalue;
    }

    D_ARRAY_CREATE(ast_t, &node->children);
    for(int i=0; i<term->nrterms; i++){
        ast_t ch;
        _eqparse_apply_rule_create(&ch, &term->terms[term->nrterms-i-1], interested);
        d_array_insert(&node->children, &ch);
    }
}

int _eqparse_apply_rule(ast_t * node, const eqrule * rule){
    if(node==NULL) return 0;

    d_array_t interested;
    D_ARRAY_CREATE(ast_t, &interested);
    // Get all interested
    _eqparse_apply_rule_visit(node, &rule->from, &interested);

    // Build up new ast
    ast_t new;
    _eqparse_apply_rule_create(&new, &rule->to, &interested);

    // TODO cleaup old ast

    memcpy(node, &new, sizeof(ast_t));

    d_array_destroy(&interested);
    return 0;
}

int _eqparse_do_rulelist(ast_t * node, const eqrule * list){
    if(node==NULL) return 0;
    int matched = -1; 
    for(int i=0; i<sizeof(eqrule_normalisation)/sizeof(eqrule); i++){
        if(_eqparse_equivalence(node, &list[i].from)){
            matched = i;
            break;
        }
    }

    // Apply rule
    if(matched>=0) _eqparse_apply_rule(node, &list[matched]);
    
    int chdone = matched>=0 ? 1 : 0;
    D_ARRAY_FOREACH_BT(ast_t, n, &node->children){
        chdone += _eqparse_do_rulelist(n, list);
    }
    return chdone;
}

int _eqparse_normalize(eqparse_t * eq){
    return _eqparse_do_rulelist(&eq->AST, &eqrule_normalisation);
}

// -----------------------

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
            printf("%s(", eqparse_functions[ast->tok.ivalue].name);
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
        fprintf(f, "%s%d [label=\"%s\"]\n", parent, i, eqparse_functions[ast->tok.ivalue].name);
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

    // Simplify
    printf("Simplification: ");
    int steps = 100;
    int done = 1;
    while(done && steps){
        printf(".");
        done = 0;
        done += _eqparse_normalize(eq);
        steps -= 1;
    }
    printf("\n");

    return eq;
}

void eqparse_cleanup(eqparse_t * eq){
    if(eq==NULL) return;
    if(eq->string) free(eq->string);
    _clean_ast(&eq->AST);
    free(eq);
}
