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
                buf[0] = c;
                bufpos = 1;
                mode = NEXT_TOKEN_MODE_NUMBER;
                continue;
            }

            // Check if start of string
            else if((c>='a' && c<='z') || (c>='A' && c<='Z')){
                buf[0] = c;
                bufpos = 1;
                mode = NEXT_TOKEN_MODE_STRING;
                continue;
            }

            // Check if simple character
            else if(c=='+') {tok.tok = TOKEN_PLUS; break;}
            else if(c=='-') {tok.tok = TOKEN_MINUS; break;}
            else if(c=='*') {tok.tok = TOKEN_TIMES; break;}
            else if(c=='/') {tok.tok = TOKEN_DIVIDE; break;}
            else if(c=='^') {tok.tok = TOKEN_POWER; break;}
            else if(c=='%') {tok.tok = TOKEN_MODULO; break;}
            else if(c=='(') {tok.tok = TOKEN_BROPEN; break;}
            else if(c==')') {tok.tok = TOKEN_BRCLOSE; break;}
            else if(c==',') {tok.tok = TOKEN_COMMA; break;}
            else if(c=='=') {tok.tok = TOKEN_EQUAL; break;}

            // Ignore whitespaces
            else if(c==' ' || c=='\t' || c=='\n') continue;

            // Unknown character
            tok.cvalue = c;
            break;

        } else if(mode==NEXT_TOKEN_MODE_NUMBER){
            // Normal number
            if(c>='0' && c<='9'){
                buf[bufpos++] = c;
            // Try for decimal point
            } else if(number_decimal==0 && (c==',' || c=='.')){
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

    return tok;
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

eqparse_t * eqparse(char * string, size_t slen){
    if(string==NULL) return NULL;
    if(strlen(string)==0) return NULL;
    if(slen==0) slen=strlen(string);

    eqparse_t * eq = calloc(sizeof(eqparse_t), 1);
    if(eq==NULL) return NULL;

    eq->string = calloc(slen+1, 1);
    if(eq->string==NULL){
        free(eq);
        return NULL;
    }

    memcpy(eq->string, string, slen);
    eq->slen = slen;

    token_t tok;
    while((tok = next_token(eq)).tok!=TOKEN_EOF){
        debug_print_token(&tok);
    }

    return eq;
}

void eqparse_cleanup(eqparse_t * eq){
    if(eq==NULL) return;
    if(eq->string) free(eq->string);
    free(eq);
}
