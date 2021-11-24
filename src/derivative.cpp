#include "derivative.h"
#include "dumpsystem/dumpsystem.h"

derivative_err derivate_(Tree* expr_out, Node** base_node, Node* node_in)
{
    Lexem tmp = {};

    switch(node_in->lex.type)
    {
        case LEXT_IMMCONST:
            tmp.type = LEXT_IMMCONST;
            tmp.value.num = 0;
            tree_add(expr_out, base_node, tmp);

            break;
        case LEXT_VAR:
            tmp.type = LEXT_IMMCONST;
            tmp.value.num = 1;
            tree_add(expr_out, base_node, tmp);
            
            break;
        case LEXT_OP:
            switch(node_in->lex.value.code)
            {
                case LEX_ADD : case LEX_SUB :
                    tree_add(expr_out, base_node, node_in->lex);

                    derivate_(expr_out, &(*base_node)->left,  node_in->left);
                    derivate_(expr_out, &(*base_node)->right, node_in->right);

                    break;
                case LEX_MUL :
                    tmp.type = LEXT_OP;
                    tmp.value.code = LEX_ADD;
                    tree_add(expr_out, base_node, tmp);

                    tree_add(expr_out, (*base_node)->left, node_in->lex);
                    tree_add(expr_out, (*base_node)->right, node_in->lex);

                    derivate_(expr_out, (*base_node)->left->left),   node_in->left);
                    (*base_node)->left->right) = node_in->right;
                    derivate_(expr_out, (*base_node)->right->left),  node_in->right);
                    (*base_node)->right->right = node_in->right;

                    break;
                default:
                    ASSERT$(0, op_error, assert(0));
            }
            
            break;
        default:
            ASSERT$(0, type_error, assert(0));
    }

    return DRVTV_NOERR;
}
/*
derivative_err derivate(Tree* expr_out, Tree* expr_in)
{
    ASSERT_RET$(expr_out, expr_in, DRVTV_NULLPTR);

    derivate_
}
*/