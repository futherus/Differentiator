#ifndef ARTICLE_H
#define ARTICLE_H

#include "tree/Tree.h"

enum article_enum
{
    ARTICLE_BEFORE_DRVTV  = 0,
    ARTICLE_BEFORE_CUTTER = 1,
    ARTICLE_RESULT        = 2,
    ARTICLE_RANDOM        = 666,
};

void article_init();

void article_expression(Tree* tree, article_enum num);

#endif // ARTICLE_H