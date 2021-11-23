#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

const int LEXER_MAX_NAME = 7;

enum Lexem_type
{
    LEXT_NOTYPE   = 0,
    LEXT_IMMCONST = 1,
    LEXT_VAR      = 2,
    LEXT_OP       = 3,
    LEXT_PAREN    = 4,
    LEXT_DELIM    = 5,
};

enum Lexem_code
{
    LEX_NOCODE    =   0,
    LEX_ADD       = '+',
    LEX_SUB       = '-',
    LEX_MUL       = '*',
    LEX_DIV       = '/',
    LEX_POWER     = '^',
    LEX_LRPAR     = '(',
    LEX_RRPAR     = ')',
    LEX_LQPAR     = '[',
    LEX_RQPAR     = ']',
    LEX_COMMA     = ',',
    LEX_COLON     = ':',
    LEX_SEMICOLON = ';',
};

struct Lexem 
{
    Lexem_type type = LEXT_NOTYPE;
    size_t pos = 0;

    union Value
    {
        double      num;            //< for immensive constants
        Lexem_code code;            //< for reserved names
        char name[LEXER_MAX_NAME];  //< for string   names
    } value;
};

int lexer(char* txt);

int consume(Lexem* lex);

int peek(Lexem* lex);

#endif // LEXER_H
