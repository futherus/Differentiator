#include <math.h>
#include <assert.h>

#include "tree/Tree.h"
#include "cutter.h"
#include "dumpsystem/dumpsystem.h"

static int OPT_STATE = 0;

#define IF_NUM(NUM, LEX) if((LEX).code == LEX_IMMCONST && (LEX).value.num == (NUM))

static void cut_add_(Node** node)
{
    assert(node);

    IF_NUM(0, (*node)->left->lex)
    {
        *node = (*node)->right;
        OPT_STATE = 1;
        return;
    }

    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
        return;
    }
}

static void cut_sub_(Node** node)
{
    assert(node);
    
    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
    }
}

static void cut_mul_(Node** node)
{
    assert(node);

    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->right;
        OPT_STATE = 1;
        return;
    }

    IF_NUM(0, (*node)->left->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->left->lex)
    {
        *node = (*node)->right;
        OPT_STATE = 1;
        return;
    }
}

static void cut_div_(Node** node)
{
    assert(node);

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->right;
        OPT_STATE = 1;
    }
}

static void cut_pow_(Node** node)
{
    assert(node);

    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->right;
        (*node)->lex.value.num = 1;
        OPT_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
        return;
    }
}

#define DEF_OP(SYMB, CODE)            \
    case (LEX_##CODE):                \

#define DEF_PAREN(SYMB, CODE)         \
    case (LEX_##CODE):                \

#define DEF_FUNC(HASH, NAME)          \
    case(LEX_##NAME):                 \

static int cut_zero_one_(Node** node)
{
    assert(node);

    if((*node)->left)
        cut_zero_one_(&(*node)->left);
        
    if((*node)->right)
        cut_zero_one_(&(*node)->right);

    switch((*node)->lex.code)
    {
        case LEX_ADD:
            cut_add_(node);
            break;

        case LEX_SUB:
            cut_sub_(node);
            break;

        case LEX_MUL:
            cut_mul_(node);
            break;
            
        case LEX_POW:
            cut_pow_(node);
            break;

        case LEX_DIV:
            cut_div_(node);
            break;

        case LEX_IMMCONST : case LEX_VAR :
                #include "reserved_functions.inc"
        {
            break;
        }

        default : case LEX_NOCODE :
                #include "reserved_parentheses.inc"
        {
            return 1;
        }
    }

    return 0;
}

static int evaluate_operator_(Node* node)
{
    assert(node);

    if(node->left->lex.code != LEX_IMMCONST || node->right->lex.code != LEX_IMMCONST)
        return 0;

    double left   = node->left->lex.value.num;
    double right  = node->right->lex.value.num;
    double result = 0;

    switch(node->lex.code)
    {
        case LEX_ADD:
            result = left + right;
            break;

        case LEX_SUB:
            result = left - right;
            break;

        case LEX_MUL:
            result = left * right;
            break;
            
        case LEX_DIV:
            result = left / right;
            break;

        case LEX_POW:
            result = pow(left, right);
            break;
        
        default : case LEX_IMMCONST : case LEX_VAR : case LEX_NOCODE :
                #include "reserved_parentheses.inc"
                #include "reserved_functions.inc"
        {
            return 1;
        }
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.code      = LEX_IMMCONST;
    node->lex.value.num = result;
    OPT_STATE = 1;

    return 0;
}

static int evaluate_function_(Node* node)
{
    assert(node);

    if(node->left->lex.code != LEX_IMMCONST)
        return 0;
    
    double left   = node->left->lex.value.num;
    double result = 0;
    switch(node->lex.code)
    {
        case LEX_sin:
            result = sin(left);
            break;
            
        case LEX_cos:
            result = cos(left);
            break;

        case LEX_ln:
            result = log(left);
            break;

        default : case LEX_IMMCONST : case LEX_VAR : case LEX_NOCODE :
                #include "reserved_parentheses.inc"
                #include "reserved_operators.inc"
        {
            return 1;
        }
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.code      = LEX_IMMCONST;
    node->lex.value.num = result;
    OPT_STATE = 1;

    return 0;
}

#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC

static int const_evaluation_(Node* node)
{
    assert(node);

    int ret = 0;

    if(node->left)
        const_evaluation_(node->left);
    
    if(node->right)
        const_evaluation_(node->right);

    if(node->left)
        ret = evaluate_function_(node);

    if(node->left && node->right)
        ret = ret && evaluate_operator_(node);

    return ret;
}

void cutter(Tree* tree)
{
    assert(tree);

    OPT_STATE = 1;

    while(OPT_STATE)
    {
        OPT_STATE = 0;
        ASSERT$(!const_evaluation_(tree->root), UNKNWN_LEXEM, assert(0); );
        ASSERT$(!cut_zero_one_(&tree->root),    UNKNWN_LEXEM, assert(0); );
    }
}