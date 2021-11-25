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

static int add_new_name_(const char* name, size_t name_sz, uint64_t hash)
{
    assert(name);

    if(find_name_(hash))
        return 0;
    
    const char* new_str = nullptr;
    array_add(&STR_ARR, &new_str, name, name_sz);

    stack_push(&HASH_PTR_STK, hash);
    stack_push(&HASH_PTR_STK, (uint64_t) new_str);

    return 0;
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
        return 0;                             \

#define DEF_NAME(TYPE, HASH, NAME)            \
    case (HASH):                              \
        TMP_LEX_.type = LEXT_##TYPE;          \
        TMP_LEX_.value.code = LEX_##NAME;     \
        IS_TMP_LEX_ = true;                   \
        pos += n_read;                        \
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
        #include "reserved_symbols.inc"

        default:
        { /* fallthrough */ }
    }

    wordlen_(txt + pos, &n_read);

    if(n_read == 0)
        return -1;     // UNKNOWN SYMBOLS
      
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

    add_new_name_(txt + pos, n_read, hash);

    pos += n_read;

    return 0;
}

#undef DEF_NAME
#undef DEF_SYMB

int lexer(char* data)
{
    stack_init(&HASH_PTR_STK, 0);
    array_init(&STR_ARR, 0);

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

#define DEF_NAME(TYPE, HASH, NAME)      \
    case(LEX_##NAME):                   \
        sprintf(buffer, "%s", #NAME);   \
        break;                          \

char* demangle(Lexem lex)
{
    static char buffer[1000] = "";

    switch(lex.type)
    {
        case LEXT_IMMCONST:
            sprintf(buffer, "%lg", lex.value.num);
            break;
        case LEXT_OP : case LEXT_PAREN : case LEXT_DELIM :
            sprintf(buffer, "%c", lex.value.code);
            break;
        case LEXT_FUNC:
            switch(lex.value.code)
            {
                #include "reserved_names.inc"

                default:
                    assert(0);
            }
            
            break;
        case LEXT_VAR:
        {
            const char* ptr = find_name_(lex.value.hash);
            if(!ptr)
                assert(0);

            sprintf(buffer, "%s", ptr);

            break;
        }
        default: case LEXT_NOTYPE :
            assert(0);
    }

    return buffer;
}
#undef DEF_NAME
