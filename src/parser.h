#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "tree/Tree.h"

enum parser_err
{
    PARSER_NOERR = 0,
    PARSER_PASS_ERR = 1,
    PARSER_TREE_FAIL = 2,
    PARSER_LEXER_FAIL = 3,
    PARSER_FLTHRGH = 4,
    PARSER_MISS_PAREN = 5,
    PARSER_UNEXPCTD_OP = 6,
};

parser_err parse(Tree* tree, char* data);

#endif // PARSER_H