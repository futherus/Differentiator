#ifndef DERIVATIVE_H
#define DERIVATIVE_H

#include "tree/Tree.h"

enum derivative_err
{
    DRVTV_NOERR = 0,
    DRVTV_PASS_ERR  = 1,
    DRVTV_NULLPTR = 2,
    DRVTV_NOTFOUND = 3,
    DRVTV_TREE_FAIL = 4,

};

derivative_err derivate(Tree* expr_out, Tree* expr_in);

#endif // DERIVATIVE_H