#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "../hash.h"
#include "lexer.h"
#include "../stack/include/Stack.h"
#include "../dumpsystem/dumpsystem.h"
#include "Array.h"

static Stack HASH_NAME_STK = {};
static Array NAME_ARR      = {};

static const char* find_name_(uint64_t hash)
{
    const uint64_t* ptr = (uint64_t*) HASH_NAME_STK.buffer;

    for(size_t iter = 0; iter < HASH_NAME_STK.size; iter += 2)
    {
        if(*(ptr + iter) == hash)
            return (const char*) *(ptr + iter + 1);
    }

    return nullptr;
}

lexer_err add_new_name(const char* name, size_t name_sz, uint64_t hash)
{
    ASSERT_RET$(name, LEXER_NULLPTR);

    if(find_name_(hash))
        return LEXER_NAME_EXISTS;
    
    const char* new_str = nullptr;
    ASSERT_RET$(!array_add(&NAME_ARR, &new_str, name, name_sz),  LEXER_ARRAY_FAIL);

    ASSERT_RET$(!stack_push(&HASH_NAME_STK, hash),               LEXER_STACK_FAIL);
    ASSERT_RET$(!stack_push(&HASH_NAME_STK, (uint64_t) new_str), LEXER_STACK_FAIL);

    return LEXER_NOERR;
}

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

#define DEF_OP(SYMB, CODE)          \
    case (SYMB):                    \
        TMP_LEX_.code = LEX_##CODE; \
        IS_TMP_LEX_ = true;         \
        pos += 1;                   \
        return LEXER_NOERR;         \

#define DEF_PAREN(SYMB, CODE)       \
    case (SYMB):                    \
        TMP_LEX_.code = LEX_##CODE; \
        IS_TMP_LEX_ = true;         \
        pos += 1;                   \
        return LEXER_NOERR;         \

#define DEF_FUNC(HASH, NAME)        \
    case (HASH):                    \
        TMP_LEX_.code = LEX_##NAME; \
        IS_TMP_LEX_ = true;         \
        pos += n_read;              \
        return LEXER_NOERR;         \

#define DEF_SUBST(HASH, NAME)       \
    case (HASH):                    \

static Lexem TMP_LEX_    = {};
static bool  IS_TMP_LEX_ = false;
static size_t DATA_SZ_   = 0;

static lexer_err lexer_(char* data)
{
    static char*  txt = nullptr;
    static size_t pos = 0;

    ASSERT_RET$(txt || data, LEXER_NULLPTR);

    if(data)
    {
        TMP_LEX_    = {};
        IS_TMP_LEX_ = false;
        txt = data;
        pos = 0;
    }

    clean_whitespaces_(txt, &pos);

    TMP_LEX_.location.pos = pos;

    if(pos == DATA_SZ_)
    {
        TMP_LEX_.code = LEX_EOF;
        TMP_LEX_.value.hash = 0;
        return LEXER_NOERR;
    }
    
    int n_read = 0;

    if(sscanf(txt + pos, "%lg%n", &TMP_LEX_.value.num, &n_read) > 0)
    {
        TMP_LEX_.code = LEX_IMMCONST;
        IS_TMP_LEX_   = true;
        pos += n_read;

        return LEXER_NOERR;
    }

    switch(txt[pos])
    {
        #include "../reserved_operators.inc"
        
        #include "../reserved_parentheses.inc"

        default:
        { /* fallthrough */ }
    }

    wordlen_(txt + pos, &n_read);

    ASSERT_RET$(n_read > 0, LEXER_UNKNWN_SYMB);

    uint64_t hash = fnv1_64(txt + pos, n_read);
    switch(hash)
    {
        #include "../reserved_functions.inc"
        
        #include "../reserved_substitutions.inc"
            ASSERT_RET$(0, LEXER_RESERVED_NAMES);

        default:
        { /*fallthrough*/ }
    }

    TMP_LEX_.code       = LEX_VAR;
    IS_TMP_LEX_         = true;
    TMP_LEX_.value.hash = hash;

    lexer_err err = add_new_name(txt + pos, n_read, hash);
    PASS$(err == LEXER_NOERR || err == LEXER_NAME_EXISTS, return err; );

    pos += n_read;

    return LEXER_NOERR;
}

#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC
#undef DEF_SUBST

lexer_err lexer_init(char* data, size_t data_sz)
{
    ASSERT_RET$(data, LEXER_NULLPTR);
    ASSERT_RET$(data_sz > 0, LEXER_EMPTY_DATA);

    ASSERT_RET$(!stack_init(&HASH_NAME_STK, 0), LEXER_STACK_FAIL);
    ASSERT_RET$(!array_init(&NAME_ARR, 0),      LEXER_ARRAY_FAIL);

    DATA_SZ_ = data_sz;

    return lexer_(data);
}

lexer_err lexer_dstr()
{
    if(HASH_NAME_STK.buffer == nullptr && NAME_ARR.main_buffer == nullptr)
        return LEXER_NOERR;

    stack_dstr(&HASH_NAME_STK);
    array_dstr(&NAME_ARR);

    HASH_NAME_STK = {};
    NAME_ARR = {};

    IS_TMP_LEX_ = false;
    TMP_LEX_ = {};
    DATA_SZ_ = 0;

    return LEXER_NOERR;
}

lexer_err consume(Lexem* lex)
{
    ASSERT_RET$(lex, LEXER_NULLPTR);

    if(!IS_TMP_LEX_)
        PASS$(!lexer_(nullptr), return LEXER_PASS_ERR; );

    *lex = TMP_LEX_;
    IS_TMP_LEX_ = false;

    return LEXER_NOERR;
}

lexer_err peek(Lexem* lex)
{
    ASSERT_RET$(lex, LEXER_NULLPTR);

    if(!IS_TMP_LEX_)
        PASS$(!lexer_(nullptr), return LEXER_PASS_ERR; );

    *lex = TMP_LEX_;

    return LEXER_NOERR;
}

#define DEF_OP(SYMB, CODE)              \
    case (LEX_##CODE):                  \

#define DEF_PAREN(SYMB, CODE)           \
    case (LEX_##CODE):                  \

#define DEF_FUNC(HASH, NAME)            \
    case(LEX_##NAME):                   \
        sprintf(buffer, "\\%s", #NAME);   \
        break;                          \

#define DEF_SUBST(HASH, NAME)           \
    case(LEX_##NAME):                   \
        sprintf(buffer, "\\%s", #NAME); \
        break;                          \

char* demangle(const Lexem* lex)
{
    assert(lex);

    static char buffer[1000] = "";

    switch(lex->code)
    {
        case LEX_IMMCONST:
        {
            if(lex->value.num < 0)
                sprintf(buffer, "(%lg)", lex->value.num);
            else
                sprintf(buffer, "%lg", lex->value.num);

            break;
        }
        case LEX_VAR:
        {
            const char* ptr = find_name_(lex->value.hash);
            ASSERT$(ptr, LEXER_NOTFOUND, assert(0); );

            sprintf(buffer, "%s", ptr);

            break;
        }
        
        // case LEX_ADD : case LEX_SUB : case LEX_LQPAR : ...
        #include "../reserved_parentheses.inc"
        #include "../reserved_operators.inc"
        {
            sprintf(buffer, "%c", lex->code);
            break;
        }

        #include "../reserved_functions.inc"

        #include "../reserved_substitutions.inc"

        default : case LEX_NOCODE : case LEX_EOF :
            ASSERT$(0, LEXER_NOTFOUND, assert(0); );
            break;
    }

    return buffer;
}

#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC
#undef DEF_SUBST
