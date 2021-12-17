#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

enum lexer_err
{
    LEXER_NOERR          = 0,
    LEXER_PASS_ERR       = 1,
    LEXER_NULLPTR        = 2,
    LEXER_STACK_FAIL     = 3,
    LEXER_ARRAY_FAIL     = 4,
    LEXER_UNKNWN_SYMB    = 5,
    LEXER_NOTFOUND       = 6,
    LEXER_NAME_EXISTS    = 7,
    LEXER_RESERVED_NAMES = 8,
    LEXER_EMPTY_DATA     = 9,
};

#define DEF_OP(SYMB, CODE)    LEX_##CODE = SYMB,
#define DEF_PAREN(SYMB, CODE) LEX_##CODE = SYMB,
#define DEF_FUNC(HASH, NAME)  LEX_##NAME,
#define DEF_SUBST(HASH, NAME) LEX_##NAME,

enum Lexem_code
{
    LEX_EOF      = -1,
    LEX_NOCODE   =  0,
    LEX_IMMCONST =  1,
    LEX_VAR      =  2,

    #include "../reserved_functions.inc"

    #include "../reserved_parentheses.inc"

    #include "../reserved_operators.inc"

    #include "../reserved_substitutions.inc"

};
#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC
#undef DEF_SUBST

struct Lexem 
{
    Lexem_code code = LEX_NOCODE;
    
    union Location
    {
        size_t pos;      //< for position in string
        void*  head;     //< for pointer to head structure, e.g. Tree for Lexem used in Node
    } location;

    union Value
    {
        double      num; //< for immensive constants
        uint64_t   hash; //< for string names
    } value;
};

lexer_err lexer_init(char* data, size_t data_sz);
lexer_err lexer_dstr();

lexer_err consume(Lexem* lex);

lexer_err peek(Lexem* lex);

lexer_err add_new_name(const char* name, size_t name_sz, uint64_t hash);

char* demangle(const Lexem* lex);

#endif // LEXER_H
