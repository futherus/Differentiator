
#include "tree/Tree.h"
#include "cutter.h"

#include <math.h>
#include <assert.h>

static int CONST_EVAL_STATE = 0;

#define IF_NUM(NUM, LEX) if((LEX).type == LEXT_IMMCONST && (LEX).value.num == (NUM))

static void cut_add_(Node** node)
{
    IF_NUM(0, (*node)->left->lex)
    {
        *node = (*node)->right;
        CONST_EVAL_STATE = 1;
        return;
    }

    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->left;
        CONST_EVAL_STATE = 1;
        return;
    }
}

static void cut_sub_(Node** node)
{
    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->left;
        CONST_EVAL_STATE = 1;
    }
}

static void cut_mul_(Node** node)
{
    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->right;
        CONST_EVAL_STATE = 1;
        return;
    }

    IF_NUM(0, (*node)->left->lex)
    {
        *node = (*node)->left;
        CONST_EVAL_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        CONST_EVAL_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->left->lex)
    {
        *node = (*node)->right;
        CONST_EVAL_STATE = 1;
        return;
    }
}

static void cut_div_(Node** node)
{
    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->right;
        CONST_EVAL_STATE = 1;
    }
}

static void cut_pow_(Node** node)
{
    IF_NUM(0, (*node)->right->lex)
    {
        *node = (*node)->right;
        (*node)->lex.value.num = 1;
        CONST_EVAL_STATE = 1;
        return;
    }

    IF_NUM(1, (*node)->right->lex)
    {
        *node = (*node)->left;
        CONST_EVAL_STATE = 1;
        return;
    }
}

static void cut_zero_one_(Node** node)
{
    if((*node)->left)
        cut_zero_one_(&(*node)->left);
        
    if((*node)->right)
        cut_zero_one_(&(*node)->right);

    if((*node)->lex.type == LEXT_OP)
    {
        switch((*node)->lex.value.code)
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
        }
    }
}

static void evaluate_operator_(Node* node)
{
    if(node->left->lex.type != LEXT_IMMCONST || node->right->lex.type != LEXT_IMMCONST)
        return;

    double left   = node->left->lex.value.num;
    double right  = node->right->lex.value.num;
    double result = 0;

    switch(node->lex.value.code)
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
        default:
            assert(0);
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.type      = LEXT_IMMCONST;
    node->lex.value.num = result;
    CONST_EVAL_STATE = 1;
}

static void evaluate_function_(Node* node)
{
    if(node->left->lex.type != LEXT_IMMCONST)
        return;
    
    double left   = node->left->lex.value.num;
    double result = 0;
    switch(node->lex.value.code)
    {
        case LEX_sin:
            result = sin(left);
            break;
        case LEX_cos:
            result = cos(left);
            break;
        default:
            assert(0);
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->lex.type      = LEXT_IMMCONST;
    node->lex.value.num = result;
    CONST_EVAL_STATE = 1;
}

static void const_evaluation_(Node* node)
{
    if(node->left)
        const_evaluation_(node->left);
    
    if(node->right)
        const_evaluation_(node->right);

    switch(node->lex.type)
    {
        case LEXT_FUNC:
            evaluate_function_(node);
            break;
        case LEXT_OP:
            evaluate_operator_(node);
            break;
        default:
            break;
    }
}

разность одинаковых деревьев

расставление скобочек

замены

копирование узлов

частная производная

подстановка параметра

н-арное дерево

void cutter(Tree* tree)
{
    CONST_EVAL_STATE = 1;

    while(CONST_EVAL_STATE)
    {
        CONST_EVAL_STATE = 0;
        const_evaluation_(tree->root);
        cut_zero_one_(&tree->root);
    }
    
}