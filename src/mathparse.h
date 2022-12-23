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
    TOKENTYPE_COMMA,
    TOKENTYPE_EOF,
    TOKENTYPE_ERROR,
} tokentype_e;

typedef enum{
    FUNCTION_SIN = 0,
    FUNCTION_COS,
    FUNCTION_TAN,
    FUNCTION_STEP,
    FUNCTION_INT,
    FUNCTION_DDT,
    _FUNCTION_SIZE_,
} function_e;

typedef struct{
    char * name;
    function_e func;
    int arguments;
    void (*cfunc)();
} function_t;

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
        function_t fvalue;
        operation_t ovalue;
        char * svalue;
        int bisfunc;
    };
} token_t;

typedef struct{
    const char * string;
    size_t position;
    d_array_t output;
    d_array_t stack;
    token_t ptok;
} parserstate_t;

typedef enum{
    PARSE_ERROR_OK = 0,
    PARSE_ERROR_STRINGISNULL,
    PARSE_ERROR_STATEALREADYINITIALIZED,
    PARSE_ERROR_STRINGLENISZERO,
    PARSE_ERROR_STATEISNULL,
    PARSE_ERROR_UNMATCHEDBRACKET,
    PARSE_ERROR_STARTSWITHBRACKET,
    PARSE_ERROR_EXPECTEDOPENBRACKED,
    PARSE_ERROR_UNEXPECTEDCOMMA,
    PARSE_ERROR_TOOMANYARGS,
    PARSE_ERROR_TOOLITTLEARGS,
} parse_error_e;

token_t nexttoken(parserstate_t * state);
void print_token(token_t t);
parse_error_e parse(const char * string, size_t len, parserstate_t ** state);
parse_error_e parser_cleanup(parserstate_t ** state);

extern const function_t functions[_FUNCTION_SIZE_];
extern const operation_t operations[_OPERATION_SIZE_];

#endif
