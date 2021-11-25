#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include "article.h"

static const char TREE_TEXFILE[]  = "tree_tex.pdf";

#define PRINT(format, ...) fprintf(stream, format, ##__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////////////

static FILE* TEX_STREAM = nullptr;

static char TEX_TEMP_FILE[] = "tree_dump/tex_temp.tex";

static const char TEX_INTRO[] = 
R"(
\documentclass[11pt]{article}
\usepackage[utf8]{inputenc}
\usepackage{geometry} 
\geometry{a4paper} 
\usepackage{graphicx}

\title{Brief Article}
\author{The Author}

\begin{document}

\maketitle
)";

static const char TEX_OUTRO[] = 
R"(
\end{document}
)";

static int get_priority_(Node* check)
{
    if(!check)
        return 0;

    switch(check->lex.value.code)
    {
        case LEX_ADD : case LEX_SUB :
            return 1;
        case LEX_MUL : case LEX_DIV :
            return 2;
        case LEX_POW:
            return 3;
        default:
        { /*fallthrough*/ }
    }
    
    return 0;
}

static void tree_tex_node_(Node* node, Node* parent)
{
    FILE* stream = TEX_STREAM;

    switch(node->lex.type)
    {
        case LEXT_OP:
            if(get_priority_(node) <= get_priority_(parent))
                PRINT("(");

            switch(node->lex.value.code)
            {
                case LEX_DIV:
                    PRINT(" \\frac{");
                    break;
                default:
                { /*fallthrough*/ }
            }
            
            tree_tex_node_(node->left, node);

            switch(node->lex.value.code)
            {
                case LEX_DIV:
                    PRINT("}{");
                    break;
                case LEX_MUL:
                    PRINT(" \\cdot ");
                    break;
                case LEX_POW:
                    PRINT(" ^{");
                    break;
                default:
                    PRINT(" %c ", node->lex.value.code);
                    break;
            }

            tree_tex_node_(node->right, node);

            switch(node->lex.value.code)
            {
                case LEX_DIV : case LEX_POW :
                    PRINT("} ");
                    break;
                default:
                { /*fallthrough*/ }
            }

            if(get_priority_(node) <= get_priority_(parent))
                PRINT(")");

            break;
        case LEXT_FUNC:
            PRINT("%s(", demangle(&node->lex));

            tree_tex_node_(node->left, node);
            
            PRINT(")");
            break;
        default:
            PRINT("%s", demangle(&node->lex));
    }
}

void tree_tex_dump(Tree* tree)
{
    FILE* stream = TEX_STREAM;
    if(stream == nullptr)
        return;

    PRINT("$$");

    tree_tex_node_(tree->root, nullptr);

    PRINT("$$");
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void close_texfile_()
{
    fprintf(TEX_STREAM, "%s", TEX_OUTRO);

    if(fclose(TEX_STREAM) != 0)
        perror("Tree tex file can't be succesfully closed");

    char sys_cmd[FILENAME_MAX] = "pdflatex -quiet -aux-directory=tree_dump -job-name=tex_dump ";
    strcat(sys_cmd, TEX_TEMP_FILE);

    system(sys_cmd);
}

void tree_tex_init()
{
    if(TREE_TEXFILE[0] != 0)
    {
        TEX_STREAM = fopen(TEX_TEMP_FILE, "w");

        if(TEX_STREAM)
        {
            atexit(&close_texfile_);
            fprintf(TEX_STREAM, "%s", TEX_INTRO);
            
            return;
        }

    }

    perror("Can't open tex file");

    return;
}
