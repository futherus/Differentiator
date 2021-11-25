#include "tree/Tree.h"
#include "lexer/lexer.h"
#include "derivative.h"
#include "parser.h"
#include "dumpsystem/dumpsystem.h"
#include "cutter.h"

#include <stdlib.h>
#include <sys/stat.h>

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
    
    Tree tree = {};
    tree_dump_init(dumpsystem_get_stream(log));
    tree_tex_init();

    parse(&tree, buffer);
    tree_tex_dump(&tree);
    Tree derived_tree = {};

    derivate(&derived_tree, &tree);

    tree_dump(&derived_tree, "dump");
    
    cutter(&derived_tree);

    tree_dump(&derived_tree, "dump");
    tree_tex_dump(&derived_tree);

    return 0;
}