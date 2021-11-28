#include <stdlib.h>
#include <sys/stat.h>

#include "tree/Tree.h"
#include "lexer/lexer.h"
#include "derivative.h"
#include "parser.h"
#include "dumpsystem/dumpsystem.h"
#include "cutter.h"
#include "article.h"

static int file_sz_(const char filename[], size_t* sz)
{
    struct stat buff = {};
    if(stat(filename, &buff) == -1)
        return -1;
    
    *sz = buff.st_size;
    
    return 0;
}

enum differentiator_err
{
    DIFFTOR_NOERR = 0,
    DIFFTOR_READ_FAIL = 1,
};

int main()
{
    size_t file_size = 0;
    ASSERT_RET$(!file_sz_("input.txt", &file_size), DIFFTOR_READ_FAIL);

    char* buffer = (char*) calloc(file_size, sizeof(char));

    FILE* instream = fopen("input.txt", "r");
    ASSERT_RET$(instream, DIFFTOR_READ_FAIL);

    fread(buffer, sizeof(char), file_size, instream);
    ASSERT_RET$(!ferror(instream), DIFFTOR_READ_FAIL);
    
    tree_dump_init(dumpsystem_get_stream(log));
    article_init();

    Tree tree = {};
    Tree derived_tree = {};

    parse(&tree, buffer);
    tree_dump(&tree, "ORIGINAL_TREE");

    article_expression(&tree, ARTICLE_BEFORE_DRVTV);
    derivate(&derived_tree, &tree);
    tree_dump(&derived_tree, "DERIVED_TREE");

    article_expression(&derived_tree, ARTICLE_BEFORE_CUTTER);
    cutter(&derived_tree);
    tree_dump(&derived_tree, "CUTTED_TREE");

    article_expression(&derived_tree, ARTICLE_RESULT);

    tree_dstr(&tree);
    tree_dstr(&derived_tree);

    return 0;
}