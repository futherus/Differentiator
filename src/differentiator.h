#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include "tree/Tree.h"

enum diff_err
{
    DIFF_NOERR           = 0,
    DIFF_PASS_ERR        = 1,
    DIFF_NULLPTR         = 2,
    DIFF_READ_FAIL       = 3,
    DIFF_TREE_FAIL       = 5,
    DIFF_INDEFINITE      = 6,
    DIFF_UNEXPTD_LEX     = 7,
    DIFF_AUTOVAR_OVRFLW  = 8,
    DIFF_BAD_ALLOC       = 9,
};

enum article_enum
{
    ARTICLE_NO_NOTE       = 0,
    ARTICLE_BEFORE_DRVTV  = 1,
    ARTICLE_BEFORE_CUTTER = 2,
    ARTICLE_RESULT        = 3,
    ARTICLE_RANDOM        = 666,
};

const double EPSILON = 1.0e-128;

void article_init(const char diff_var[]);
diff_err article_record(Tree* tree, article_enum note, const char* prefix);

diff_err differentiator(char* equation);

diff_err derivate(Tree* expr_out, Tree* expr_in, const char diff_var[]);

diff_err cutter(Tree* tree);

#endif // DIFFERENTIATOR_H