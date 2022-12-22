#ifndef __H_MATHPARSE
#define __H_MATHPARSE

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

typedef enum{
    PARSE_ERROR_OK = 0,
    PARSE_ERROR_STRINGISNULL,
    PARSE_ERROR_STATEALREADYINITIALIZED,
    PARSE_ERROR_STRINGLENISZERO,
    PARSE_ERROR_STATEISNULL,
    PARSE_ERROR_UNMATCHEDBRACKET,
} parse_error_e;

token_t nexttoken(parserstate_t * state);
void print_token(token_t t);
parse_error_e parse(const char * string, size_t len, parserstate_t ** state);
parse_error_e parser_cleanup(parserstate_t ** state);

#endif
