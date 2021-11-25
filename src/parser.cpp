#include <assert.h>
#include <stdio.h>

#include "parser.h"
#include "dumpsystem/dumpsystem.h"

static parser_err parse_lexem_(Tree* tree, Node** node_ptr)
{
    assert(tree && node_ptr);

    Lexem lex = {};
    Lexem_code expctd = LEX_NOCODE;

    ASSERT_RET$(!tree_add(tree, node_ptr, &lex), PARSER_TREE_FAIL);

    PASS$(!consume(&lex),        return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.type == LEXT_PAREN, PARSER_MISS_PAREN; );

    switch(lex.value.code)
    {
        case LEX_LRPAR:
            expctd = LEX_RRPAR;
            break;
        case LEX_LQPAR:
            expctd = LEX_RQPAR;
            break;
        default:
            ASSERT$(0, PARSER_FLTHRGH, assert(0); );
    }
    
    PASS$(!peek(&lex), return PARSER_LEXER_FAIL; );

    switch(lex.type)
    {
        case LEXT_PAREN:
            PASS$(!parse_lexem_(tree, &(*node_ptr)->left),   return PARSER_PASS_ERR;   );

            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );

            PASS$(!parse_lexem_(tree, &(*node_ptr)->right),  return PARSER_PASS_ERR;   );
            
            break;
        case LEXT_IMMCONST : case LEXT_VAR :
            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );
            
            break;
        case LEXT_FUNC:
            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );

            PASS$(!parse_lexem_(tree, &(*node_ptr)->left),   return PARSER_PASS_ERR;   );

            break;
        default: case LEXT_NOTYPE :
            ASSERT$(0, PARSER_FLTHRGH, assert(0); );
    }

    PASS$(!consume(&lex),                                    return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.type == LEXT_PAREN && lex.value.code == expctd, PARSER_MISS_PAREN; );

    return PARSER_NOERR;
}

parser_err parse(Tree* tree, char* data)
{
    assert(tree && data);

    PASS$(!lexer(data), return PARSER_LEXER_FAIL; );

    PASS$(!parse_lexem_(tree, &tree->root), return PARSER_PASS_ERR; );

    return PARSER_NOERR;
}
