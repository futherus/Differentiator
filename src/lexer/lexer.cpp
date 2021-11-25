#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "../hash.h"
#include "lexer.h"
#include "../stack/include/Stack.h"
#include "../dumpsystem/dumpsystem.h"
#include "Array.h"

static Stack HASH_PTR_STK = {};
static Array STR_ARR      = {};

static const char* find_name_(uint64_t hash)
{
    const uint64_t* ptr = (uint64_t*) HASH_PTR_STK.buffer;

    for(size_t iter = 0; iter < HASH_PTR_STK.size; iter += 2)
    {
        if(*(ptr + iter) == hash)
            return (const char*) *(ptr + iter + 1);
    }

    return nullptr;
}

static lexer_err add_new_name_(const char* name, size_t name_sz, uint64_t hash)
{
    assert(name);

    if(find_name_(hash))
        return LEXER_NOERR;
    
    const char* new_str = nullptr;
    ASSERT_RET$(!array_add(&STR_ARR, &new_str, name, name_sz),  LEXER_ARRAY_FAIL);

    ASSERT_RET$(!stack_push(&HASH_PTR_STK, hash),               LEXER_STACK_FAIL);
    ASSERT_RET$(!stack_push(&HASH_PTR_STK, (uint64_t) new_str), LEXER_STACK_FAIL);

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

#define DEF_SYMB(TYPE, SYMB, CODE)            \
    case (SYMB):                              \
        TMP_LEX_.type = LEXT_##TYPE;          \
        TMP_LEX_.value.code = LEX_##CODE;     \
        IS_TMP_LEX_ = true;                   \
        pos += 1;                             \
        return LEXER_NOERR;                   \

#define DEF_NAME(TYPE, HASH, NAME)            \
    case (HASH):                              \
        TMP_LEX_.type = LEXT_##TYPE;          \
        TMP_LEX_.value.code = LEX_##NAME;     \
        IS_TMP_LEX_ = true;                   \
        pos += n_read;                        \
        return LEXER_NOERR;                   \

static Lexem TMP_LEX_    = {};
static bool  IS_TMP_LEX_ = false;

static lexer_err lexer_(char* data)
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

        return LEXER_NOERR;
    }

    switch(txt[pos])
    {
        #include "reserved_symbols.inc"

        default:
        { /* fallthrough */ }
    }

    wordlen_(txt + pos, &n_read);

    ASSERT_RET$(n_read > 0, LEXER_UNKNWN_SYMB);

    uint64_t hash = fnv1_64(txt + pos, n_read);
    switch(hash)
    {
        #include "reserved_names.inc"
        
        default:
        { /*fallthrough*/ }
    }

    TMP_LEX_.type       = LEXT_VAR;
    IS_TMP_LEX_         = true;
    TMP_LEX_.value.hash = hash;

    PASS$(!add_new_name_(txt + pos, n_read, hash), return LEXER_PASS_ERR; );

    pos += n_read;

    return LEXER_NOERR;
}

#undef DEF_NAME
#undef DEF_SYMB

lexer_err lexer(char* data)
{
    ASSERT_RET$(!stack_init(&HASH_PTR_STK, 0), LEXER_STACK_FAIL);
    ASSERT_RET$(!array_init(&STR_ARR, 0),      LEXER_ARRAY_FAIL);

    return lexer_(data);
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

#define DEF_NAME(TYPE, HASH, NAME)      \
    case(LEX_##NAME):                   \
        sprintf(buffer, "%s", #NAME);   \
        break;                          \

char* demangle(const Lexem* lex)
{
    static char buffer[1000] = "";

    switch(lex->type)
    {
        case LEXT_IMMCONST:
        {
            sprintf(buffer, "%lg", lex->value.num);
            break;
        }
        case LEXT_OP : case LEXT_PAREN : case LEXT_DELIM :
        {
            sprintf(buffer, "%c", lex->value.code);
            break;
        }
        case LEXT_FUNC:
        {
            switch(lex->value.code)
            {
                #include "reserved_names.inc"

                default:
                    ASSERT$(0, LEXER_NOTFOUND, assert(0); );
            }
            
            break;
        }
        case LEXT_VAR:
        {
            const char* ptr = find_name_(lex->value.hash);
            ASSERT$(ptr, LEXER_NOTFOUND, assert(0); );

            sprintf(buffer, "%s", ptr);

            break;
        }
        default: case LEXT_NOTYPE :
            ASSERT$(0, LEXER_NOTFOUND, assert(0); );
    }

    return buffer;
}
#undef DEF_NAME
