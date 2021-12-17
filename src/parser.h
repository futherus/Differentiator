#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "tree/Tree.h"

enum parser_err
{
    PARSER_NOERR         =  0,
    PARSER_PASS_ERR      =  1,
    PARSER_NULLPTR       =  2,
    PARSER_FILLED_TREE   =  3,
    PARSER_TREE_FAIL     =  4,
    PARSER_LEXER_FAIL    =  5,
    PARSER_FLTHRGH       =  6,
    PARSER_MISS_PAREN    =  7,
    PARSER_UNEXPCTD_LEX  =  8,
    PARSER_UNEXPCTD_EOF  =  9,
    PARSER_TRAILING_SYMB = 10,
};

parser_err parse(Tree* tree);

#endif // PARSER_H