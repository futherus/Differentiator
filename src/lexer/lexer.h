#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

enum Lexem_type
{
    LEXT_NOTYPE   = 0,
    LEXT_IMMCONST = 1,
    LEXT_OP       = 2,
    LEXT_PAREN    = 3,
    LEXT_DELIM    = 4,
    LEXT_VAR      = 5,
    LEXT_FUNC     = 6,
};

#define DEF_SYMB(TYPE, SYMB, CODE) LEX_##CODE = SYMB,
#define DEF_NAME(TYPE, HASH, NAME) LEX_##NAME,
enum Lexem_code
{
    LEX_NOCODE = 0,

    #include "reserved_names.inc"

    #include "reserved_symbols.inc"

};
#undef DEF_SYMB
#undef DEF_NAME

struct Lexem 
{
    Lexem_type type = LEXT_NOTYPE;
    size_t pos = 0;

    union Value
    {
        double      num; //< for immensive constants
        Lexem_code code; //< for reserved names
        uint64_t   hash; //< for string   names
    } value;
};

int lexer(char* txt);

int consume(Lexem* lex);

int peek(Lexem* lex);

char* demangle(Lexem lex);

#endif // LEXER_H
