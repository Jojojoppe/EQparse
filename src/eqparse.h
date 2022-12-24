#ifndef __H_EQPARSE
#define __H_EQPARSE

#include "general/array.h"
#include "general/stack.h"

typedef enum{
    TOKEN_DNUMBER,
    TOKEN_INUMBER,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_MODULO,

    TOKEN_BROPEN,
    TOKEN_BRCLOSE,
    TOKEN_COMMA,
    TOKEN_EQUAL,

    TOKEN_STRING,

    TOKEN_EOF,
    TOKEN_NULL,
} token_e;

typedef enum{
    TOKEN_TYPE_VALUE,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_UNARY_OPERATOR,
    TOKEN_TYPE_BINARY_OPERATOR,
    TOKEN_TYPE_FUNCTION,
    TOKEN_TYPE_NULL,
} token_type_e;

typedef struct{
    token_e tok;
    token_type_e type;
    union{
        double dvalue;
        char * svalue;
        long ivalue;
        char cvalue;
    };
} token_t;

typedef struct{
    token_t tok;
    d_array_t children;
} ast_t;

enum{
    EQPARSE_ERROR_OK=0,
    EQPARSE_ERROR_INTERNAL,
    EQPARSE_ERROR_CHARACTER,
    EQPARSE_ERROR_UNMATCHED,
};

typedef struct{
    char * string;
    size_t position;
    size_t slen;
    token_t token_error;
    ast_t AST;
} eqparse_t;

eqparse_t * eqparse(char * string, size_t slen, int * err);
void eqparse_cleanup(eqparse_t * eq);

void debug_print_token(token_t * token);
void debug_print_ast(ast_t * ast);

#include "eqparse_data.h"

#endif
