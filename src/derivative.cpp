#include <assert.h>

#include "derivative.h"
#include "dumpsystem/dumpsystem.h"

#define NUM(BASE_NODE, VAL) PASS$(!tree_add_num(tree, (BASE_NODE), (VAL)), return DRVTV_TREE_FAIL; )

inline tree_err tree_add_num(Tree* tree, Node** base_node, double val)
{
    Lexem tmp = {};
    tmp.type      = LEXT_IMMCONST;
    tmp.value.num = val;

    return tree_add(tree, base_node, &tmp);
}

#define OP(BASE_NODE, NAME)   PASS$(!tree_add_code(tree, (BASE_NODE), LEXT_OP,   LEX_##NAME), return DRVTV_TREE_FAIL; )
#define FUNC(BASE_NODE, NAME) PASS$(!tree_add_code(tree, (BASE_NODE), LEXT_FUNC, LEX_##NAME), return DRVTV_TREE_FAIL; )

inline tree_err tree_add_code(Tree* tree, Node** base_node, Lexem_type type, Lexem_code code)
{
    Lexem tmp = {};
    tmp.type       = type;
    tmp.value.code = code;

    return tree_add(tree, base_node, &tmp);
}

#define MKNODE(BASE_NODE, LEX) PASS$(!tree_add(tree, (BASE_NODE), (LEX)),   return DRVTV_TREE_FAIL; )
#define DERIV(BASE_NODE, NODE) PASS$(!derivate_(tree, (BASE_NODE), (NODE)), return DRVTV_PASS_ERR;  )

static derivative_err derivate_(Tree* tree, Node** base, Node* in)
{
    assert(tree && base && in);

    switch(in->lex.type)
    {
        case LEXT_IMMCONST:
            NUM(base, 0);

            break;
        case LEXT_VAR:
            NUM(base, 1);

            break;
        case LEXT_OP:
            switch(in->lex.value.code)
            {
                case LEX_ADD : case LEX_SUB :
                    MKNODE(base, &in->lex);

                    DERIV(&(*base)->left, in->left);
                    DERIV(&(*base)->right, in->right);

                    break;
                case LEX_MUL:
                    OP(base, ADD);

                    OP(&(*base)->left,  MUL);
                    OP(&(*base)->right, MUL);
                    
                    DERIV(&(*base)->left->left, in->left);
                    (*base)->left->right = in->right;
                    (*base)->right->left = in->left;
                    DERIV(&(*base)->right->right, in->right);

                    break;
                case LEX_DIV:
                    OP(base, DIV);

                    OP(&(*base)->left,  SUB);
                    OP(&(*base)->right, MUL);

                    OP(&(*base)->left->left,  MUL);
                    OP(&(*base)->left->right, MUL);
                    (*base)->right->left  = in->right;
                    (*base)->right->right = in->right;

                    DERIV(&(*base)->left->left->left, in->left);
                    (*base)->left->left->right = in->right;
                    (*base)->left->right->left = in->left;
                    DERIV(&(*base)->left->right->right, in->right);

                    break;
                case LEX_POW:
                {
                    Node* tmp_root = nullptr;

                    OP(base, MUL);

                    (*base)->left = in;
                    OP(&tmp_root, MUL);

                    FUNC(&tmp_root->left, ln);
                    tmp_root->right = in->right;

                    tmp_root->left->left = in->left;

                    DERIV(&(*base)->right, tmp_root);

                    break;
                }
                default:
                    ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
            }
            
            break;
        case LEXT_FUNC:
            switch(in->lex.value.code)
            {
                case LEX_sin:
                    OP(base, MUL);

                    FUNC(&(*base)->left, cos);
                    DERIV(&(*base)->right, in->left);

                    (*base)->left->left = in->left;

                    break;
                case LEX_cos:
                    OP(base, MUL);

                    OP(&(*base)->left, SUB);
                    DERIV(&(*base)->right, in->left);

                    NUM(&(*base)->left->left, 0);

                    FUNC(&(*base)->left->right, sin);
                    
                    (*base)->left->right->left = in->left;

                    break;
                case LEX_ln:
                    OP(base, POW);

                    (*base)->left = in->left;
                    NUM(&(*base)->right, -1);

                    break;
                default:
                    ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
            }

            break;
        default:
            ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
    }

    return DRVTV_NOERR;
}

derivative_err derivate(Tree* tree_out, Tree* tree_in)
{
    ASSERT_RET$(tree_out && tree_in, DRVTV_NULLPTR);

    PASS$(!derivate_(tree_out, &(tree_out->root), tree_in->root), return DRVTV_PASS_ERR; );

    return DRVTV_NOERR;
}
