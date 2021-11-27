#include <assert.h>

#include "derivative.h"
#include "article.h"
#include "dumpsystem/dumpsystem.h"

#define NUM(BASE_NODE, VAL) PASS$(!tree_add_num(tree, (BASE_NODE), (VAL)), return DRVTV_TREE_FAIL; )

inline tree_err tree_add_num(Tree* tree, Node** base_node, double val)
{
    Lexem tmp = {};
    tmp.code      = LEX_IMMCONST;
    tmp.value.num = val;

    return tree_add(tree, base_node, &tmp);
}

#define RSRVD(BASE_NODE, NAME) PASS$(!tree_add_code(tree, (BASE_NODE), LEX_##NAME), return DRVTV_TREE_FAIL; )

inline tree_err tree_add_code(Tree* tree, Node** base_node, Lexem_code code)
{
    Lexem tmp = {};
    tmp.code = code;

    return tree_add(tree, base_node, &tmp);
}

#define MKNODE(BASE_NODE, LEX)  PASS$(!tree_add(tree,  (BASE_NODE), (LEX)),    return DRVTV_TREE_FAIL; )
#define COPY(BASE_NODE, ORIGIN) PASS$(!tree_copy(tree, (BASE_NODE), (ORIGIN)), return DRVTV_TREE_FAIL; )
#define DERIV(BASE_NODE, NODE)  PASS$(!derivate_(tree, (BASE_NODE), (NODE)),   return DRVTV_PASS_ERR;  )

#define DEF_OP(SYMB, CODE)            \
    case (LEX_##CODE):                \

#define DEF_PAREN(SYMB, CODE)         \
    case (LEX_##CODE):                \

#define DEF_FUNC(HASH, NAME)          \
    case(LEX_##NAME):                 \

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

    switch(in->lex.code)
    {
        case LEX_IMMCONST:
            NUM(base, 0);
            break;

        case LEX_VAR:
            NUM(base, 1);
            break;

        case LEX_ADD : case LEX_SUB :
            MKNODE(base, &in->lex);

            (*base)->left  = in->left;
            (*base)->right = in->right;

            DERIV(&(*base)->left, in->left);
            DERIV(&(*base)->right, in->right);
            break;

        case LEX_MUL:
            RSRVD(base, ADD);

            RSRVD(&(*base)->left,  MUL);
            RSRVD(&(*base)->right, MUL);
            
            (*base)->left->left = in->left;
            COPY(&(*base)->left->right, in->right);
            COPY(&(*base)->right->left, in->left);
            (*base)->right->right = in->right;

            DERIV(&(*base)->left->left, in->left);
            DERIV(&(*base)->right->right, in->right);
            break;

        case LEX_DIV:
            RSRVD(base, DIV);

            RSRVD(&(*base)->left,  SUB);
            RSRVD(&(*base)->right, MUL);

            RSRVD(&(*base)->left->left,  MUL);
            RSRVD(&(*base)->left->right, MUL);
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
            RSRVD(base, MUL);

            COPY(&(*base)->left, in);
            RSRVD(&(*base)->right, ADD);

            RSRVD(&(*base)->right->left,  MUL);
            RSRVD(&(*base)->right->right, MUL);

            RSRVD(&(*base)->right->left->left, DIV);
            (*base)->right->left->right = in->left;
            RSRVD(&(*base)->right->right->left, ln);
            (*base)->right->right->right = in->right;
            
            COPY(&(*base)->right->left->left->left,  in->right);
            COPY(&(*base)->right->left->left->right, in->left);
            COPY(&(*base)->right->right->left->left, in->left);

            DERIV(&(*base)->right->left->right,  in->left);
            DERIV(&(*base)->right->right->right, in->right);
            break;

        case LEX_sin:
            RSRVD(base, MUL);

            RSRVD(&(*base)->left, cos);
            (*base)->right = in->left;

            COPY(&(*base)->left->left, in->left);

            DERIV(&(*base)->right, in->left);
            break;

        case LEX_cos:
            RSRVD(base, MUL);

            RSRVD(&(*base)->left, SUB);
            (*base)->right = in->left;

            NUM(&(*base)->left->left, 0);

            RSRVD(&(*base)->left->right, sin);
            
            COPY(&(*base)->left->right->left, in->left);

            DERIV(&(*base)->right, in->left);
            break;

        case LEX_ln:
            RSRVD(base, MUL);

            RSRVD(&(*base)->left, POW);
            (*base)->right = in->left;

            COPY(&(*base)->left->left, in->left);

            NUM(&(*base)->left->right, -1);

            DERIV(&(*base)->right, in->left);
            break;
            
        default: case LEX_NOCODE :
                                    #include "reserved_parentheses.inc"
        {   
            ASSERT$(0, DRVTV_NOTFOUND, assert(0); );
        }
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
