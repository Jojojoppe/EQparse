#ifndef __H_EQPARSE_DATA
#define __H_EQPARSE_DATA

#include <math.h>

struct eqparse_constant_info{
    char * name;
    double value;
};
#define EQPARSE_CONSTANT_INFO_N 3
static const struct eqparse_constant_info eqparse_constants[] = {
    {"pi", M_PI},
    {"e", M_E},
    {"phi", 1.6180339887499},
};

struct eqparse_function_info{
    char * name;
    int arguments;
};
#define EQPARSE_FUNCTION_INFO_N 3
static const struct eqparse_function_info eqparse_functions[] = {
    {"sin", 1},
    {"cos", 1},
    {"tan", 1},
};

struct eqparse_operator_info{
    int type;
    int precedence;
    char c;
    char * s;
};
#define EQPARSE_OPERATOR_INFO_N 6
static const struct eqparse_operator_info eqparse_operators[] = {
    {TOKEN_PLUS, 2, '+', "add"},
    {TOKEN_MINUS, 2, '-', "sub"},
    {TOKEN_TIMES, 3, '*', "mul"},
    {TOKEN_DIVIDE, 3, '/', "div"},
    {TOKEN_POWER, 4, '^', "pow"},
    {TOKEN_MODULO, 2, '%', "mod"},
};

#endif
