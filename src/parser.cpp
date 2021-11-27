#include <assert.h>
#include <stdio.h>

#include "parser.h"
#include "dumpsystem/dumpsystem.h"

#define DEF_OP(SYMB, CODE)            \
    case (LEX_##CODE):                \

#define DEF_PAREN(SYMB, CODE)         \
    case (LEX_##CODE):                \

#define DEF_FUNC(HASH, NAME)          \
    case(LEX_##NAME):                 \

static parser_err parse_lexem_(Tree* tree, Node** node_ptr)
{
    assert(tree && node_ptr);

    Lexem lex = {};
    Lexem_code expctd = LEX_NOCODE;

    ASSERT_RET$(!tree_add(tree, node_ptr, &lex), PARSER_TREE_FAIL);

    PASS$(!consume(&lex), return PARSER_LEXER_FAIL; );

    switch(lex.code)
    {
        case LEX_LRPAR:
            expctd = LEX_RRPAR;
            break;

        case LEX_LQPAR:
            expctd = LEX_RQPAR;
            break;

        default: case LEX_NOCODE : case LEX_IMMCONST : case LEX_VAR :
                 case LEX_RRPAR  : case LEX_RQPAR    :
                #include "reserved_functions.inc"
                #include "reserved_operators.inc"
        {
            ASSERT_RET$(0, PARSER_MISS_PAREN);
        }
    }
    
    PASS$(!peek(&lex), return PARSER_LEXER_FAIL; );

    switch(lex.code)
    {
        #include "reserved_parentheses.inc"
            PASS$(!parse_lexem_(tree, &(*node_ptr)->left),   return PARSER_PASS_ERR;   );

            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );
            (*node_ptr)->lex.location.head = tree;

            PASS$(!parse_lexem_(tree, &(*node_ptr)->right),  return PARSER_PASS_ERR;   );
            
            break;
        case LEX_IMMCONST : case LEX_VAR :
            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );
            (*node_ptr)->lex.location.head = tree;
            
            break;
        #include "reserved_functions.inc"
            PASS$(!consume(&(*node_ptr)->lex),               return PARSER_LEXER_FAIL; );
            (*node_ptr)->lex.location.head = tree;

            PASS$(!parse_lexem_(tree, &(*node_ptr)->left),   return PARSER_PASS_ERR;   );

            break;
        #include "reserved_operators.inc"
            ASSERT_RET$(0, PARSER_UNEXPCTD_OP);
            
            break;
        default: case LEX_NOCODE :
            ASSERT$(0, PARSER_FLTHRGH, assert(0); );
    }

    PASS$(!consume(&lex), return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.code == expctd, PARSER_MISS_PAREN);

    return PARSER_NOERR;
}

parser_err parse(Tree* tree, char* data)
{
    assert(tree && data);

    PASS$(!lexer(data), return PARSER_LEXER_FAIL; );

    PASS$(!parse_lexem_(tree, &tree->root), return PARSER_PASS_ERR; );

    return PARSER_NOERR;
}
