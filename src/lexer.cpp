#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static void clean_whitespaces_(char* txt, size_t* pos)
{
    assert(txt && pos);

    while(isspace(txt[*pos]))
        (*pos)++;
}

static void wordlen_(char* ptr, int* n_read)
{
    assert(ptr && n_read);

    while(isalnum(ptr[*n_read]) || ptr[*n_read] == '_')
        (*n_read)++;
}

#define DEF_SYMB(SYMB, TYPE, CODE)            \
    case (SYMB):                              \
        TMP_LEX_.type = LEXT_##TYPE;          \
        TMP_LEX_.value.code = LEX_##CODE;     \
        IS_TMP_LEX_ = true;                   \
        pos += 1;                             \
        return 0;                             \

static Lexem TMP_LEX_    = {};
static bool  IS_TMP_LEX_ = false;

static int lexer_(char* data)
{
    static char*  txt = nullptr;
    static size_t pos = 0;

    if(data)
    {
        TMP_LEX_    = {};
        IS_TMP_LEX_ = false;
        txt = data;
        pos = 0;
    }

    clean_whitespaces_(txt, &pos);

    TMP_LEX_.pos = pos;
    
    int n_read = 0;

    if(sscanf(txt + pos, "%lg%n", &TMP_LEX_.value.num, &n_read) > 0)
    {
        TMP_LEX_.type = LEXT_IMMCONST;
        IS_TMP_LEX_   = true;
        pos += n_read;

        return 0;
    }

    switch(txt[pos])
    {
        DEF_SYMB('+', OP, ADD)
        DEF_SYMB('-', OP, SUB)
        DEF_SYMB('*', OP, MUL)
        DEF_SYMB('/', OP, DIV)
        DEF_SYMB('^', OP, POWER)
        DEF_SYMB('(', PAREN, LRPAR)
        DEF_SYMB(')', PAREN, RRPAR)
        DEF_SYMB('[', PAREN, LQPAR)
        DEF_SYMB(']', PAREN, RQPAR)
        DEF_SYMB(',', DELIM, COMMA)
        DEF_SYMB(':', DELIM, COLON)
        DEF_SYMB(';', DELIM, SEMICOLON)

        default:
        { /* fallthrough */ }
    }

    wordlen_(txt + pos, &n_read);

    if(n_read == 0)
        return -1;     // UNKNOWN SYMBOLS

    if(n_read > LEXER_MAX_NAME)
        return -1;
        
    TMP_LEX_.type = LEXT_VAR;
    IS_TMP_LEX_   = true;
    memcpy(TMP_LEX_.value.name, txt + pos, n_read);
    TMP_LEX_.value.name[n_read] = '\0';

    pos += n_read;

    return 0;
}

int lexer(char* data)
{
    return lexer_(data);
}

int consume(Lexem* lex)
{
    int state = 0;

    if(!IS_TMP_LEX_)
    {
        state = lexer_(nullptr);

        if(state)
            return state;
    }

    *lex = TMP_LEX_;
    IS_TMP_LEX_ = false;

    return 0;
}

int peek(Lexem* lex)
{
    int state = 0;

    if(!IS_TMP_LEX_)
    {
        state = lexer_(nullptr);

        if(state)
            return state;
    }

    *lex = TMP_LEX_; 

    return 0;
}
