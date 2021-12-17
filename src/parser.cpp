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

#define DEF_SUBST(HASH, NAME)         \
    case(LEX_##NAME):                 \

static parser_err parse_lexem_(Tree* tree, Node** node_ptr)
{
    assert(tree && node_ptr);

    Lexem lex = {};
    Lexem_code expctd = LEX_NOCODE;

    ASSERT_RET$(!tree_add(tree, node_ptr, &lex), PARSER_TREE_FAIL);

    PASS$(!consume(&lex), return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.code != LEX_EOF, PARSER_UNEXPCTD_EOF);

    switch(lex.code)
    {
        case LEX_LRPAR:
            expctd = LEX_RRPAR;
            break;

        case LEX_LQPAR:
            expctd = LEX_RQPAR;
            break;

        default: case LEX_NOCODE   : case LEX_EOF   : 
                 case LEX_IMMCONST : case LEX_VAR   :
                 case LEX_RRPAR    : case LEX_RQPAR :
                #include "reserved_functions.inc"
                #include "reserved_operators.inc"
                #include "reserved_substitutions.inc"
        {
            ASSERT_RET$(0, PARSER_MISS_PAREN);
        }
    }
    
    PASS$(!peek(&lex), return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.code != LEX_EOF, PARSER_UNEXPCTD_EOF);

    switch(lex.code)
    {
        case LEX_LRPAR : case LEX_LQPAR :
            PASS$(!parse_lexem_(tree, &(*node_ptr)->left),   return PARSER_PASS_ERR;   );

            PASS$(!consume(&lex),                            return PARSER_LEXER_FAIL; );
            lex.location.head = tree;
            ASSERT_RET$(lex.code != LEX_EOF,                        PARSER_UNEXPCTD_EOF);
            (*node_ptr)->lex = {lex};

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

        case LEX_RRPAR : case LEX_RQPAR :
        #include "reserved_operators.inc"
        #include "reserved_substitutions.inc"
            ASSERT_RET$(0, PARSER_UNEXPCTD_LEX);
            break;
        
        default: case LEX_NOCODE : case LEX_EOF :
            ASSERT$(0, PARSER_FLTHRGH, assert(0); );
    }

    PASS$(!consume(&lex), return PARSER_LEXER_FAIL; );
    ASSERT_RET$(lex.code == expctd, PARSER_MISS_PAREN);

    return PARSER_NOERR;
}

#undef DEF_OP
#undef DEF_PAREN
#undef DEF_SUBST
#undef DEF_FUNC

parser_err parse(Tree* tree)
{
    ASSERT_RET$(tree, PARSER_NULLPTR);
    ASSERT_RET$(tree->root == nullptr, PARSER_FILLED_TREE);

    PASS$(!parse_lexem_(tree, &tree->root), return PARSER_PASS_ERR; );

    Lexem lex = {};
    ASSERT_RET$(!peek(&lex), PARSER_LEXER_FAIL);

    ASSERT_RET$(lex.code == LEX_EOF, PARSER_TRAILING_SYMB);

    return PARSER_NOERR;
}
