#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "tree/Tree.h"

enum parser_err
{
    PARSER_NOERR = 0,
    PARSER_BAD_DATA = 1,
    PARSER_BAD_FORMAT = 2,
};

parser_err parse(Tree* tree, char* data);

#endif // PARSER_H