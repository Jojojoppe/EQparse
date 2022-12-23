#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mathparse.h"
#include "dynamic_array.h"

typedef struct astnode_s{
    token_t tok;
    d_array_t children;
} astnode_t;

typedef struct{
    char * left;
    char * right;
    astnode_t leftnodes;
    astnode_t rightnodes;
    parserstate_t * lparser;
    parserstate_t * rparser;
} equation_t;

void _eq_debug_dot(astnode_t * node, FILE * f, char * parent, int i){
    if(node->tok.type==TOKENTYPE_NUMBER){
        fprintf(f, "%s_%d [label=\"%.2lf\"]\n", parent, i, node->tok.dvalue);
        fprintf(f, "%s -> %s_%d\n", parent, parent, i);
    }
    else if(node->tok.type==TOKENTYPE_VARIABLE){
        fprintf(f, "%s_%d [label=\"%s\"]\n", parent, i, node->tok.svalue);
        fprintf(f, "%s -> %s_%d\n", parent, parent, i);
    }
    else if(node->tok.type==TOKENTYPE_OPERATOR){
        fprintf(f, "%s_%d [label=\"%c\"]\n", parent, i, node->tok.ovalue.opchar);
        fprintf(f, "%s -> %s_%d\n", parent, parent, i);
    }
    else if(node->tok.type==TOKENTYPE_FUNCTION){
        fprintf(f, "%s_%d [label=\"%s\"]\n", parent, i, node->tok.fvalue.name);
        fprintf(f, "%s -> %s_%d\n", parent, parent, i);
    }
    int j=0;
    char * name = calloc(strlen(parent)+5, 1);
    sprintf(name, "%s_%d", parent, i);
    for(int k=D_ARRAY_LEN(node->children)-1; k>=0; k--){
        _eq_debug_dot(d_array_at(&node->children, k), f, name, j++);
    }
    free(name);
}
void equation_debug_dot(equation_t * eq, const char * fname){
    FILE * f = fopen(fname, "w");

    fprintf(f, "digraph{\n");
    fprintf(f, "root [label=\"=\"]\n");
    _eq_debug_dot(&eq->leftnodes, f, "root", 0);
    _eq_debug_dot(&eq->rightnodes, f, "root", 1);
    fprintf(f, "}\n");

    fclose(f);
}

int _eq_build_ast(d_array_t * rpn, astnode_t * ast, int i){
    D_ARRAY_INIT(astnode_t, &ast->children);

    token_t * tok = (token_t*)d_array_at(rpn, i);
    ast->tok = *tok;

    if(tok->type==TOKENTYPE_OPERATOR || tok->type==TOKENTYPE_FUNCTION){
        int operands = tok->type==TOKENTYPE_FUNCTION ? tok->fvalue.arguments : 2;
        for(int j=0; j<operands; j++){
            astnode_t newnode;
            i = _eq_build_ast(rpn, &newnode, i-1);
            d_array_insert(&ast->children, &newnode);
        }
    }

    return i;
}

int equation_parse(const char * s, size_t slen, equation_t ** eq){
    if(s==NULL) return -1; // nonexistant string
    if(slen==0) return -1; // string length 0
    if(eq==NULL) return -1; // nonexistant equation object
    if(*eq!=NULL) return -1; // equation object already initialized
    
    // Find '='
    char * right = strchr(s, '=');
    if(right==NULL) return -1; // string not an equation
    
    *eq = calloc(sizeof(equation_t), 1);
    if(*eq==NULL) return -1; // could not allocate equation object
    equation_t * _eq = *eq;

    // Copy left and right to own strings
    size_t lsize = (size_t)(right-s);
    size_t rsize = slen-(size_t)(right-s)-1;
    _eq->left = calloc(lsize+1, 1);
    _eq->right = calloc(rsize+1, 1);
    memcpy(_eq->left, s, lsize);
    memcpy(_eq->right, right+1, rsize);

    parserstate_t * parser = NULL;
    parse_error_e perr;

    if((perr = parse(_eq->left, lsize, &parser))!=PARSE_ERROR_OK){
        return -1*perr; // left parse error;
    }
    _eq_build_ast(&parser->output, &_eq->leftnodes, D_ARRAY_LEN(parser->output)-1);
    _eq->lparser = parser;


    parser = NULL;
    if((perr = parse(_eq->right, rsize, &parser))!=PARSE_ERROR_OK){
        return -1*perr; // right parse error;
    }
    _eq_build_ast(&parser->output, &_eq->rightnodes, D_ARRAY_LEN(parser->output)-1);
    _eq->rparser = parser;

    return 0;
}

int equation_cleanup(equation_t ** eq){
    if(eq==NULL) return -1; // nonexistant equation object
    if(*eq==NULL) return -1; // equatin object not initialized
    
    free((*eq)->left);
    free((*eq)->right);

    // TODO traverse AST and free all nodes

    parser_cleanup(&(*eq)->lparser);
    parser_cleanup(&(*eq)->rparser);

    free(*eq);
    *eq = NULL;

    return 0;
}

int main(int argc, char ** argv){

    char * diffequation = "out = (in-ddt(out))/K";
    equation_t * eq = NULL;

    int rval = equation_parse(diffequation, strlen(diffequation), &eq);
    printf("rval = %d\n", rval);
    if(rval!=0) goto cleanup;

    equation_debug_dot(eq, "testout.dot");

cleanup:
    equation_cleanup(&eq);
    return 0;
}
