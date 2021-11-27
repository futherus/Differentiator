#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

enum lexer_err
{
    LEXER_NOERR = 0,
    LEXER_PASS_ERR = 1,
    LEXER_NULLPTR = 2,
    LEXER_STACK_FAIL = 3,
    LEXER_ARRAY_FAIL = 4,
    LEXER_UNKNWN_SYMB = 5,
    LEXER_NOTFOUND = 6,
};

#define DEF_OP(SYMB, CODE) LEX_##CODE = SYMB,
#define DEF_PAREN(SYMB, CODE) LEX_##CODE = SYMB,
#define DEF_FUNC(HASH, NAME) LEX_##NAME,
enum Lexem_code
{
    LEX_NOCODE   = 0,
    LEX_IMMCONST = 1,
    LEX_VAR      = 2,

    #include "../reserved_functions.inc"

    #include "../reserved_operators.inc"

    #include "../reserved_parentheses.inc"

};
#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC

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
        uint64_t   hash; //< for string   names
    } value;
};

lexer_err lexer(char* txt);

lexer_err consume(Lexem* lex);

lexer_err peek(Lexem* lex);

char* demangle(const Lexem* lex);

#endif // LEXER_H
