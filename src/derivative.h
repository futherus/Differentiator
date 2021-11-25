#ifndef DERIVATIVE_H
#define DERIVATIVE_H

#include "tree/Tree.h"

enum derivative_err
{
    DRVTV_NOERR = 0,
    DRVTV_NULLPTR = 1,
};

derivative_err derivate(Tree* expr_out, Tree* expr_in);

#endif // DERIVATIVE_H