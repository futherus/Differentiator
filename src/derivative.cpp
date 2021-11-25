#include "derivative.h"
#include "dumpsystem/dumpsystem.h"
#include <assert.h>

#define NUM(BASE_NODE, VAL) tree_add_num(expr_out, (BASE_NODE), (VAL))
inline tree_err tree_add_num(Tree* tree, Node** base_node, double val)
{
    Lexem tmp = {};
    tmp.type      = LEXT_IMMCONST;
    tmp.value.num = val;

    return tree_add(tree, base_node, tmp);
}

#define OP(BASE_NODE, NAME)   tree_add_code(expr_out, (BASE_NODE), LEXT_OP,   LEX_##NAME)
#define FUNC(BASE_NODE, NAME) tree_add_code(expr_out, (BASE_NODE), LEXT_FUNC, LEX_##NAME)
inline tree_err tree_add_code(Tree* tree, Node** base_node, Lexem_type type, Lexem_code code)
{
    Lexem tmp = {};
    tmp.type       = type;
    tmp.value.code = code;

    return tree_add(tree, base_node, tmp);
}

#define IN_L   (node_in->left)
#define IN_R   (node_in->right)
#define IN_LEX (node_in->lex)

static derivative_err derivate_(Tree* expr_out, Node** base, Node* node_in)
{
    assert
    switch(node_in->lex.type)
    {
        case LEXT_IMMCONST:
            NUM(base, 0);

            break;
        case LEXT_VAR:
            NUM(base, 1);

            break;
        case LEXT_OP:
            switch(node_in->lex.value.code)
            {
                case LEX_ADD : case LEX_SUB :
                    tree_add(expr_out, base, IN_LEX);

                    derivate_(expr_out, &(*base)->left, IN_L);
                    derivate_(expr_out, &(*base)->right, IN_R);

                    break;
                case LEX_MUL:
                    OP(base, ADD);

                    OP(&(*base)->left,  MUL);
                    OP(&(*base)->right, MUL);
                    
                    derivate_(expr_out, &(*base)->left->left, IN_L);
                    (*base)->left->right = IN_R;
                    (*base)->right->left = IN_L;
                    derivate_(expr_out, &(*base)->right->right, IN_R);

                    break;
                case LEX_DIV:
                    OP(base, DIV);

                    OP(&(*base)->left, SUB);
                    OP(&(*base)->right, MUL);

                    OP(&(*base)->left->left, MUL);
                    OP(&(*base)->left->right, MUL);
                    (*base)->right->left  = IN_R;
                    (*base)->right->right = IN_R;

                    derivate_(expr_out, &(*base)->left->left->left, IN_L);
                    (*base)->left->left->right = IN_R;
                    (*base)->left->right->left  = IN_L;
                    derivate_(expr_out, &(*base)->left->right->right, IN_R);

                    break;
                case LEX_POW:
                    OP(base, MUL);

                    OP(&(*base)->left, MUL);
                    derivate_(expr_out, &(*base)->right, IN_L);

                    (*base)->left->left = IN_R;
                    OP(&(*base)->left->right, POW);

                    (*base)->left->right->left = IN_L;
                    OP(&(*base)->left->right->right, SUB);

                    (*base)->left->right->right->left = IN_R;
                    NUM(&(*base)->left->right->right->right, 1);
                        
                    break;
                default:
                    ASSERT$(0, op_error, assert(0); );
            }
            
            break;
        case LEXT_FUNC:
            switch(node_in->lex.value.code)
            {
                case LEX_sin:
                    OP(base, MUL);

                    FUNC(&(*base)->left, cos);
                    derivate_(expr_out, &(*base)->right, IN_L);

                    (*base)->left->left = IN_L;

                    break;
                case LEX_cos:
                    OP(base, MUL);

                    OP(&(*base)->left, SUB);
                    derivate_(expr_out, &(*base)->right, IN_L);

                    NUM(&(*base)->left->left, 0);

                    FUNC(&(*base)->left->right, sin);
                    
                    (*base)->left->right->left = IN_L;

                    break;
                default:
                    assert(0);
            }

            break;
        default:
            ASSERT$(0, type_error, assert(0); );
    }

    return DRVTV_NOERR;
}

derivative_err derivate(Tree* expr_out, Tree* expr_in)
{
    ASSERT_RET$(expr_out && expr_in, DRVTV_NULLPTR);

    derivate_(expr_out, &(expr_out->root), expr_in->root);

    return DRVTV_NOERR;
}
