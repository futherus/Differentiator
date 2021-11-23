#include "parser.h"
#include "dumpsystem/dumpsystem.h"

#include <assert.h>
#include <stdio.h>

static parser_err parse_lexem_(Tree* tree, Node** node_ptr)
{
    Lexem lex = {};
    Lexem_code expctd = LEX_NOCODE;

    if(!tree->root)
    {
        tree_init(tree, lex);
        node_ptr = &tree->root;
    }
    else
        tree_add(tree, node_ptr, lex);

    ASSERT_RET$(!consume(&lex), PARSER_BAD_FORMAT);
    ASSERT_RET$(lex.type == LEXT_PAREN, PARSER_BAD_FORMAT);

    switch(lex.value.code)
    {
        case LEX_LRPAR:
            expctd = LEX_RRPAR;
            break;
        case LEX_LQPAR:
            expctd = LEX_RQPAR;
            break;
        default:
            assert(0);
    }
    
    ASSERT_RET$(!peek(&lex), PARSER_BAD_FORMAT);

    switch(lex.type)
    {
        case LEXT_PAREN:
            parse_lexem_(tree, &(*node_ptr)->left);

            consume(&(*node_ptr)->lex);

            parse_lexem_(tree, &(*node_ptr)->right);
            
            break;
        case LEXT_IMMCONST : case LEXT_VAR :
            consume(&(*node_ptr)->lex);

            break;
        default:
            assert(0);
    }

    ASSERT_RET$(!consume(&lex), PARSER_BAD_FORMAT);
    ASSERT_RET$(lex.type == LEXT_PAREN && lex.value.code == expctd, PARSER_BAD_FORMAT);

    return PARSER_NOERR;
}

parser_err parse(Tree* tree, char* data)
{
    assert(tree && data);

    ASSERT_RET$(!lexer(data), PARSER_BAD_DATA);

    PASS$(!parse_lexem_(tree, nullptr), return PARSER_BAD_FORMAT; );

    return PARSER_NOERR;
}
