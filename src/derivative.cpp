#include <assert.h>

#include "derivative.h"
#include "article.h"
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

#define MKNODE(BASE_NODE, LEX)  PASS$(!tree_add(tree,  (BASE_NODE), (LEX)),    return DRVTV_TREE_FAIL; )
#define COPY(BASE_NODE, ORIGIN) PASS$(!tree_copy(tree, (BASE_NODE), (ORIGIN)), return DRVTV_TREE_FAIL; )
#define DERIV(BASE_NODE, NODE)  PASS$(!derivate_(tree, (BASE_NODE), (NODE)),   return DRVTV_PASS_ERR;  )

static derivative_err derivate_(Tree* tree, Node** base, Node* in)
{
    assert(tree && base && in);

    //tree_dump(tree, "dump");
    //LOG$("LEX: %s", demangle(&in->lex));
    if(tree->size)
    {
        article_note(ARTICLE_RANDOM);
        article_expression(tree);
    }

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

                    (*base)->left  = in->left;
                    (*base)->right = in->right;

                    DERIV(&(*base)->left, in->left);
                    DERIV(&(*base)->right, in->right);
                    break;
                case LEX_MUL:
                    OP(base, ADD);

                    OP(&(*base)->left,  MUL);
                    OP(&(*base)->right, MUL);
                    
                    (*base)->left->left = in->left;
                    COPY(&(*base)->left->right, in->right);
                    COPY(&(*base)->right->left, in->left);
                    (*base)->right->right = in->right;

                    DERIV(&(*base)->left->left, in->left);
                    DERIV(&(*base)->right->right, in->right);
                    break;
                case LEX_DIV:
                    OP(base, DIV);

                    OP(&(*base)->left,  SUB);
                    OP(&(*base)->right, MUL);

                    OP(&(*base)->left->left,  MUL);
                    OP(&(*base)->left->right, MUL);
                    COPY(&(*base)->right->left, in->right);
                    COPY(&(*base)->right->right, in->right);

                    (*base)->left->left->left = in->left;
                    COPY(&(*base)->left->left->right, in->right);
                    COPY(&(*base)->left->right->left, in->left);
                    (*base)->left->right->right = in->right;

                    DERIV(&(*base)->left->left->left, in->left);
                    DERIV(&(*base)->left->right->right, in->right);
                    break;
                case LEX_POW:
                {
                    OP(base, MUL);

                    COPY(&(*base)->left, in);
                    OP(&(*base)->right, ADD);

                    OP(&(*base)->right->left,  MUL);
                    OP(&(*base)->right->right, MUL);

                    OP(&(*base)->right->left->left, DIV);
                    (*base)->right->left->right = in->left;
                    FUNC(&(*base)->right->right->left, ln);
                    (*base)->right->right->right = in->right;
                    
                    COPY(&(*base)->right->left->left->left,  in->right);
                    COPY(&(*base)->right->left->left->right, in->left);
                    COPY(&(*base)->right->right->left->left, in->left);

                    DERIV(&(*base)->right->left->right,  in->left);
                    DERIV(&(*base)->right->right->right, in->right);

                    break;
                }
                default : case LEX_NOCODE :
                    ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
            }
            
            break;
        case LEXT_FUNC:
            switch(in->lex.value.code)
            {
                case LEX_sin:
                    OP(base, MUL);

                    FUNC(&(*base)->left, cos);
                    (*base)->right = in->left;

                    COPY(&(*base)->left->left, in->left);

                    DERIV(&(*base)->right, in->left);
                    break;
                case LEX_cos:
                    OP(base, MUL);

                    OP(&(*base)->left, SUB);
                    (*base)->right = in->left;

                    NUM(&(*base)->left->left, 0);

                    FUNC(&(*base)->left->right, sin);
                    
                    COPY(&(*base)->left->right->left, in->left);

                    DERIV(&(*base)->right, in->left);
                    break;
                case LEX_ln:
                    OP(base, MUL);

                    OP(&(*base)->left, POW);
                    (*base)->right = in->left;

                    COPY(&(*base)->left->left, in->left);

                    NUM(&(*base)->left->right, -1);

                    DERIV(&(*base)->right, in->left);
                    break;
                default:
                    ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
            }

            break;
        default: case LEXT_NOTYPE : case LEXT_PAREN :
            ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
    }

    return DRVTV_NOERR;
}

derivative_err derivate(Tree* tree_out, Tree* tree_in)
{
    ASSERT_RET$(tree_out && tree_in, DRVTV_NULLPTR);
    tree_out->root = tree_in->root;

    PASS$(!derivate_(tree_out, &(tree_out->root), tree_in->root), return DRVTV_PASS_ERR; );

    return DRVTV_NOERR;
}
