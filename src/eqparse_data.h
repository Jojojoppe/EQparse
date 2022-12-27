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
#define EQPARSE_FUNCTION_INFO_N 8
static const struct eqparse_function_info eqparse_functions[] = {
    {"sin", 1},
    {"cos", 1},
    {"tan", 1},
    {"log", 1},
    {"ln", 1},
    {"sqrt", 1},
    {"int", 2},
    {"ddt", 1},
};

struct eqparse_operator_info{
    int type;
    int precedence;
    char c;
    char * s;
};
#define EQPARSE_OPERATOR_INFO_N 7
static const struct eqparse_operator_info eqparse_operators[] = {
    {TOKEN_PLUS, 2, '+', "add"},
    {TOKEN_MINUS, 2, '-', "sub"},
    {TOKEN_TIMES, 3, '*', "mul"},
    {TOKEN_DIVIDE, 3, '/', "div"},
    {TOKEN_POWER, 4, '^', "pow"},
    {TOKEN_MODULO, 2, '%', "mod"},
    {TOKEN_EQUAL, 0, '=', "EQ"},
};

// ------------------------------------------

typedef enum{
    EQRULE_NOTE_NONE=0,
    EQRULE_NOTE_NO_NUMBER,
} eqrule_term_note;

typedef struct eqrule_term_s{
    token_e token;
    token_type_e type;
    eqrule_term_note note;
    int valued;
    union{
        long ivalue;
        double dvalue;
    };
    int interested;
    int nrterms;
    struct eqrule_term_s * terms;
} eqrule_term;

typedef struct{
    eqrule_term from;
    eqrule_term to;
} eqrule;

#define DNC(i) {.token=TOKEN_NULL, .type=TOKEN_TYPE_NULL, .valued=0, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){})}
#define DNC_NN(i) {.token=TOKEN_NULL, .type=TOKEN_TYPE_NULL, .valued=0, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){}), .note=EQRULE_NOTE_NO_NUMBER}

#define OP_ADD(t) {.token=TOKEN_PLUS, .type=TOKEN_TYPE_OPERATOR, .valued=0, .interested=0, .nrterms=2, .terms=(t)}
#define OP_SUB(t) {.token=TOKEN_MINUS, .type=TOKEN_TYPE_OPERATOR, .valued=0, .interested=0, .nrterms=2, .terms=(t)}
#define OP_MUL(t) {.token=TOKEN_TIMES, .type=TOKEN_TYPE_OPERATOR, .valued=0, .interested=0, .nrterms=2, .terms=(t)}
#define OP_DIV(t) {.token=TOKEN_DIVIDE, .type=TOKEN_TYPE_OPERATOR, .valued=0, .interested=0, .nrterms=2, .terms=(t)}
#define OP_POW(t) {.token=TOKEN_POWER, .type=TOKEN_TYPE_OPERATOR, .valued=0, .interested=0, .nrterms=2, .terms=(t)}

#define INUMBER(i) {.token=TOKEN_INUMBER, .type=TOKEN_TYPE_VALUE, .valued=0, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){})}
#define INUMBER_V(i, v) {.token=TOKEN_INUMBER, .type=TOKEN_TYPE_VALUE, .valued=1, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){}), .ivalue=(v)}
#define DNUMBER(i) {.token=TOKEN_DNUMBER, .type=TOKEN_TYPE_VALUE, .valued=0, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){}))}
#define DNUMBER_V(i, v) {.token=TOKEN_DNUMBER, .type=TOKEN_TYPE_VALUE, .valued=1, .interested=(i), .nrterms=0, .terms=((eqrule_term[]){}), .dvalue=(v)}

// NORMALISATION RULES
static const eqrule eqrule_normalisation[] = {
    {   // ?*iN -> iN*?
        OP_MUL(((eqrule_term[]){DNC_NN(1), INUMBER(2)})),
        OP_MUL(((eqrule_term[]){INUMBER(2), DNC_NN(1)})),
    },
    {   // ?+iN -> iN+?
        OP_ADD(((eqrule_term[]){DNC_NN(1), INUMBER(2)})),
        OP_ADD(((eqrule_term[]){INUMBER(2), DNC_NN(1)})),
    },
    {   // (+)+? -> ?+(+)
        OP_ADD(((eqrule_term[]){DNC_NN(1),OP_ADD(((eqrule_term[]){DNC(2), DNC(3)}))})),
        OP_ADD(((eqrule_term[]){OP_ADD(((eqrule_term[]){DNC(2), DNC(3)})),DNC_NN(1)})),
    },
    {   // (*)*? -> ?*(*)
        OP_MUL(((eqrule_term[]){DNC_NN(1), OP_MUL(((eqrule_term[]){DNC(2), DNC(3)}))})),
        OP_MUL(((eqrule_term[]){OP_MUL(((eqrule_term[]){DNC(2), DNC(3)})), DNC_NN(1)})),
    },
    {   // (+)*? -> ?*(+)
        OP_MUL(((eqrule_term[]){DNC_NN(1), OP_ADD(((eqrule_term[]){DNC(2), DNC(3)}))})),
        OP_MUL(((eqrule_term[]){OP_ADD(((eqrule_term[]){DNC(2), DNC(3)})), DNC_NN(1)})),
    },
    {   // (*)+? -> ?+(*)
        OP_ADD(((eqrule_term[]){DNC_NN(1), OP_MUL(((eqrule_term[]){DNC(2), DNC(3)}))})),
        OP_ADD(((eqrule_term[]){OP_MUL(((eqrule_term[]){DNC(2), DNC(3)})), DNC_NN(1)})),
    },
};

// SIMPLIFICATION RULES
static const eqrule eqrule_simplification[] = {
    // ZERO SIMPLIFICATIONS
    {   // 0+? -> ?
        OP_ADD(((eqrule_term[]){INUMBER_V(0, 0), DNC(1)})),
        DNC(1),
    }, 
    {   // 0.0+? -> ?
        OP_ADD(((eqrule_term[]){DNUMBER_V(0, 0.0f), DNC(1)})),
        DNC(1),
    }, 
    {   // 0*? -> 0
        OP_MUL(((eqrule_term[]){INUMBER_V(0, 0), DNC(0)})),
        INUMBER_V(0, 0),
    }, 
    {   // 0.0*? -> 0
        OP_MUL(((eqrule_term[]){DNUMBER_V(0, 0.0f), DNC(0)})),
        INUMBER_V(0, 0),
    }, 
    {   // 0/? -> 0
        OP_DIV(((eqrule_term[]){INUMBER_V(0, 0), DNC(0)})),
        INUMBER_V(0, 0),
    }, 
    {   // 0.0/? -> 0
        OP_DIV(((eqrule_term[]){DNUMBER_V(0, 0.0f), DNC(0)})),
        INUMBER_V(0, 0),
    }, 
    {   // 0-? -> -1*?
        OP_SUB(((eqrule_term[]){INUMBER_V(0, 0), DNC(1)})),
        OP_MUL(((eqrule_term[]){INUMBER_V(0, -1), DNC(1)})),
    }, 
    {   // 0.0-? -> -1*?
        OP_SUB(((eqrule_term[]){DNUMBER_V(0, 0.0f), DNC(1)})),
        OP_MUL(((eqrule_term[]){INUMBER_V(0, -1), DNC(1)})),
    },
    {   // ?^0 -> ?
        OP_POW(((eqrule_term[]){DNC(1), INUMBER_V(0, 0)})),
        DNUMBER_V(0, 1.0),
    },
    {   // ?^0.0 -> ?
        OP_POW(((eqrule_term[]){DNC(1), DNUMBER_V(0, 0.0)})),
        DNUMBER_V(0, 1.0),
    },

    // UNITY SIMPLIFICATIONS
    {   // 1*? -> ?
        OP_MUL(((eqrule_term[]){INUMBER_V(0, 1), DNC(1)})),
        DNC(1),
    }, 
    {   // 1.0*? -> ?
        OP_MUL(((eqrule_term[]){DNUMBER_V(0, 1.0), DNC(1)})),
        DNC(1),
    },
    {   // ?+? -> 2*?
        OP_ADD(((eqrule_term[]){DNC(1), DNC(1)})),
        OP_MUL(((eqrule_term[]){INUMBER_V(0, 2), DNC(1)})),
    },
    {   // ?*? -> ?^2
        OP_MUL(((eqrule_term[]){DNC(1), DNC(1)})),
        OP_POW(((eqrule_term[]){DNC(1), INUMBER_V(0, 2)})),
    },
    {   // ?^1 -> ?
        OP_POW(((eqrule_term[]){DNC(1), INUMBER_V(0, 1)})),
        DNC(1),
    },
    {   // ?^1.0 -> ?
        OP_POW(((eqrule_term[]){DNC(1), DNUMBER_V(0, 1.0)})),
        DNC(1),
    },

    // TREE SIMPLIFICATIONS
    {   //(?+x)+x -> ?+(2*x)
        OP_ADD(((eqrule_term[]){OP_ADD(((eqrule_term[]){DNC(1), DNC_NN(2)})), DNC_NN(2)})),
        OP_ADD(((eqrule_term[]){OP_MUL(((eqrule_term[]){INUMBER_V(0, 2), DNC_NN(2)})), DNC(1)})),
    },
    {   //(?*x)+x -> (?+1)*x
        OP_ADD(((eqrule_term[]){OP_MUL(((eqrule_term[]){DNC(1), DNC_NN(2)})), DNC_NN(2)})),
        OP_MUL(((eqrule_term[]){OP_ADD(((eqrule_term[]){DNC(1), INUMBER_V(0, 1)})), DNC_NN(2)})),
    },
    {   //(?*x)+(?*x) -> (?+?)*x
        OP_ADD(((eqrule_term[]){OP_MUL(((eqrule_term[]){DNC(1), DNC_NN(2)})), OP_MUL(((eqrule_term[]){DNC(3), DNC_NN(2)}))})),
        OP_MUL(((eqrule_term[]){OP_ADD(((eqrule_term[]){DNC(1), DNC(3)})),DNC_NN(2)})),
    },
    {   // (x^?)*x -> x^(?+1)
        OP_MUL(((eqrule_term[]){OP_POW(((eqrule_term[]){DNC(1), DNC(2)})), DNC(1)})),
        OP_POW(((eqrule_term[]){DNC(1), OP_ADD(((eqrule_term[]){DNC(2), DNUMBER_V(0, 1.0)}))})),
    },
    {   //(x^x)*(x^x) -> x^(?+?)
        OP_MUL(((eqrule_term[]){OP_POW(((eqrule_term[]){DNC_NN(1), DNC(2)})), OP_POW(((eqrule_term[]){DNC_NN(1), DNC(3)}))})),
        OP_POW(((eqrule_term[]){DNC_NN(1), OP_ADD(((eqrule_term[]){DNC(2), DNC(3)}))})),
    },

};

#endif
