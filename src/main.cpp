#include <stdlib.h>
#include <sys/stat.h>

#include "differentiator.h"
#include "parser.h"
#include "dumpsystem/dumpsystem.h"
#include "jumps.h"
#include "args.h"

static int get_file_sz_(const char filename[], size_t* sz)
{
    struct stat buff = {};
    if(stat(filename, &buff) == -1)
        return -1;
    
    *sz = buff.st_size;
    
    return 0;
}

int main(int argc, char* argv[])
{
    char infile_name[FILENAME_MAX]   = "";
    char diff_variable[FILENAME_MAX] = "";

    args_msg msg = process_args(argc, argv, infile_name, diff_variable);
    if(msg)
    {
        response_args(msg);
        return DIFF_NOERR;
    }

    Tree tree = {};
    Tree diff_tree = {};

    size_t file_sz = 0;
    FILE* instream = nullptr;
    char* buffer   = nullptr;

    tree_dump_init(dumpsystem_get_stream(log));
    article_init(diff_variable);

TRY__
    ASSERT$(!get_file_sz_(infile_name, &file_sz),     DIFF_READ_FAIL, FAIL__);

    buffer = (char*) calloc(file_sz, sizeof(char));
    ASSERT$(buffer,                                   DIFF_BAD_ALLOC, FAIL__);

    instream = fopen(infile_name, "r");
    ASSERT$(instream,                                 DIFF_READ_FAIL, FAIL__);

    file_sz = fread(buffer, sizeof(char), file_sz, instream);
    ASSERT$(!ferror(instream),                        DIFF_READ_FAIL, FAIL__);
    
    fclose(instream);

    buffer = (char*) realloc(buffer, file_sz * sizeof(char));
    ASSERT$(buffer,                                   DIFF_READ_FAIL, FAIL__);

    PASS$(!lexer_init(buffer, file_sz), FAIL__);

    PASS$(!parse(&tree),                FAIL__);
    tree_dump(&tree, "INITIAL_TREE");

    PASS$(!article_record(&tree, ARTICLE_BEFORE_CUTTER, "f"), FAIL__);
    PASS$(!cutter(&tree),                                     FAIL__);
    tree_dump(&tree, "CUTTED_TREE");
    
    PASS$(!article_record(&tree, ARTICLE_BEFORE_DRVTV, "f"),  FAIL__);
    PASS$(!derivate(&diff_tree, &tree, diff_variable),        FAIL__);
    tree_dump(&diff_tree, "DERIVED_TREE");

    PASS$(!article_record(&diff_tree, ARTICLE_BEFORE_CUTTER, "f'"), FAIL__);
    PASS$(!cutter(&diff_tree),                                      FAIL__);
    tree_dump(&diff_tree, "CUTTED_TREE");

    PASS$(!article_record(&diff_tree, ARTICLE_RESULT, "f'"),        FAIL__);

CATCH__
    free(buffer);
    tree_dstr(&tree);
    tree_dstr(&diff_tree);
    lexer_dstr();

FINALLY__
    return DIFF_NOERR;

ENDTRY__
}