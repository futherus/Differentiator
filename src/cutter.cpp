#include <math.h>
#include <assert.h>

#include "differentiator.h"
#include "dumpsystem/dumpsystem.h"

static int OPT_STATE = 0;

#define IF_NUM(NUM, LEX) if((LEX).code == LEX_IMMCONST && abs((LEX).value.num - (NUM)) < EPSILON)

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

static diff_err cut_div_(Node** node)
{
    assert(node);

    IF_NUM(0, (*node)->right->lex)
        ASSERT_RET$(0, DIFF_INDEFINITE);

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
    }

    return DIFF_NOERR;
}

static diff_err cut_pow_(Node** node)
{
    assert(node);

    IF_NUM(0, (*node)->right->lex)
    {
        IF_NUM(0, (*node)->left->lex)
            ASSERT_RET$(0, DIFF_INDEFINITE);

        *node = (*node)->right;
        (*node)->lex.value.num = 1;
        OPT_STATE = 1;
        return DIFF_NOERR;
    }

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        OPT_STATE = 1;
        return DIFF_NOERR;
    }

    return DIFF_NOERR;
}

#define DEF_OP(SYMB, CODE)            \
    case (LEX_##CODE):                \

#define DEF_PAREN(SYMB, CODE)         \
    case (LEX_##CODE):                \

#define DEF_FUNC(HASH, NAME)          \
    case (LEX_##NAME):                \

#define DEF_SUBST(HASH, NAME)         \
    case (LEX_##NAME):                \

static diff_err cut_zero_one_(Node** node)
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
            PASS$(!cut_pow_(node), return DIFF_UNEXPTD_LEX; );
            break;

        case LEX_DIV:
            PASS$(!cut_div_(node), return DIFF_UNEXPTD_LEX; );
            break;

        case LEX_IMMCONST : case LEX_VAR :
                #include "reserved_functions.inc"
        {
            break;
        }

        default : case LEX_NOCODE : case LEX_EOF :
                #include "reserved_parentheses.inc"
                #include "reserved_substitutions.inc"
        {
            ASSERT_RET$(0, DIFF_UNEXPTD_LEX);
        }
    }

    return DIFF_NOERR;
}

static diff_err evaluate_operator_(Node* node)
{
    assert(node);

    if(node->left->lex.code != LEX_IMMCONST || node->right->lex.code != LEX_IMMCONST)
        return DIFF_NOERR;

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

        default : case LEX_IMMCONST : case LEX_VAR : case LEX_NOCODE : case LEX_EOF :
                #include "reserved_functions.inc"
                #include "reserved_parentheses.inc"
                #include "reserved_substitutions.inc"
        {
            ASSERT_RET$(0, DIFF_UNEXPTD_LEX);
        }
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.code      = LEX_IMMCONST;
    node->lex.value.num = result;
    OPT_STATE = 1;

    return DIFF_NOERR;
}

static diff_err evaluate_function_(Node* node)
{
    assert(node);

    if(node->left->lex.code != LEX_IMMCONST)
        return DIFF_NOERR;
    
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

        default : case LEX_IMMCONST : case LEX_VAR : case LEX_NOCODE : case LEX_EOF :
                #include "reserved_parentheses.inc"
                #include "reserved_operators.inc"
                #include "reserved_substitutions.inc"
        {
            ASSERT_RET$(0, DIFF_UNEXPTD_LEX);
        }
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.code      = LEX_IMMCONST;
    node->lex.value.num = result;
    OPT_STATE = 1;

    return DIFF_NOERR;
}

#undef DEF_OP
#undef DEF_PAREN
#undef DEF_FUNC
#undef DEF_SUBST

static diff_err const_evaluation_(Node* node)
{
    assert(node);

    if(node->left)
        PASS$(!const_evaluation_(node->left), return DIFF_PASS_ERR; );
    
    if(node->right)
        PASS$(!const_evaluation_(node->right), return DIFF_PASS_ERR; );

    if(node->left)
    {
        if(node->right)
            PASS$(!evaluate_operator_(node), return DIFF_UNEXPTD_LEX; );
        else
            PASS$(!evaluate_function_(node), return DIFF_UNEXPTD_LEX; );
    }

    return DIFF_NOERR;
}

diff_err cutter(Tree* tree)
{
    ASSERT_RET$(tree, DIFF_NULLPTR);

    OPT_STATE = 1;

    while(OPT_STATE)
    {
        OPT_STATE = 0;
        PASS$(!const_evaluation_(tree->root), return DIFF_UNEXPTD_LEX; );
        PASS$(!cut_zero_one_(&tree->root),    return DIFF_UNEXPTD_LEX; );
    }

    return DIFF_NOERR;
}
