
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include "Tree.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static const char TREE_DUMPFILE[] = "tree_dump.html";
static const char TREE_TEXFILE[]  = "tree_tex.pdf";

#define PRINT(format, ...) fprintf(stream, format, ##__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////////////

static long DUMP_ITERATION = 0;
static FILE* TEMP_GRAPH_STREAM = nullptr;

static const char GRAPHVIZ_PNG_NAME[]  = "tree_dump/graphviz_dump";
static const char GRAPHVIZ_TEMP_FILE[] = "tree_dump/graphviz_temp.txt";

static const char GRAPHVIZ_INTRO[] =
R"(
digraph G{
    graph [dpi = 100];
    bgcolor = "#2F353B";
    ranksep = 1;
    edge[minlen = 3, arrowsize = 2, penwidth = 1.5, color = green];
    node[shape = rectangle, style = "filled, rounded", fillcolor = "#C5D0E6", fontsize = 30,
         color = "#C5D0E6", penwidth = 3];
)";

static const char GRAPHVIZ_OUTRO[] = "}\n";

static char* graphviz_png_()
{
    static char filename[FILENAME_MAX] = "";
    sprintf(filename, "%s_%ld.png", GRAPHVIZ_PNG_NAME, DUMP_ITERATION);

    return filename;
}

static void tree_print_node_(Node* node, size_t depth)
{
    FILE* stream = TEMP_GRAPH_STREAM;

    switch(node->lex.type)
    {
        case LEXT_OP:
            PRINT("node%p[label = \"%c\"]", node, node->lex.value.code);
            break;
        case LEXT_IMMCONST:
            PRINT("node%p[label = \"%lg\"]", node, node->lex.value.num);
            break;
        case LEXT_VAR:
            PRINT("node%p[label = \"%s\"]", node, node->lex.value.name);
            break;
        default:
            assert(0);
    }

    if(node->left  != nullptr)
        PRINT("node%p -> node%p;\n", node, node->left);
    if(node->right != nullptr)
        PRINT("node%p -> node%p;\n", node, node->right);
}

static void tree_graph_dump_(Tree* tree)
{
    FILE* stream = fopen(GRAPHVIZ_TEMP_FILE, "w");
    if(!stream)
    {
        perror("Can't open temporary dump file");
        return;
    }
    TEMP_GRAPH_STREAM = stream;

    PRINT("%s", GRAPHVIZ_INTRO);

    tree_visitor(tree, &tree_print_node_);

    PRINT("%s", GRAPHVIZ_OUTRO);

    fclose(stream);

    char sys_cmd[FILENAME_MAX] = "dot tree_dump/graphviz_temp.txt -q -Tpng -o ";
    strcat(sys_cmd, graphviz_png_());

    system(sys_cmd);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static FILE* DUMP_STREAM = nullptr;

static const char HTML_INTRO[] =
R"(
<html>
    <head>
        <title>
            Tree log
        </title>
        <style>
            .ok {color: springgreen;font-weight: bold;}
            .error{color: red;font-weight: bold;}
            .log{color: #C5D0E6;}
            .title{color: #E59E1F;text-align: center;font-weight: bold;}
        </style>
    </head>
    <body bgcolor = "#2F353B">
        <pre class = "log">
)";

static const char HTML_OUTRO[] =
R"(
        </pre>
    </body>
</html>
)";

static const char DATA_IS_NULL_MSG[] = "\n                                                                                            "
                                       "\n  DDDDDD      A    TTTTTTTTT    A         IIIII   SSSSS     N    N  U     U  L      L       "
                                       "\n  D     D    A A       T       A A          I    S          NN   N  U     U  L      L       "
                                       "\n  D     D   A   A      T      A   A         I     SSSS      N N  N  U     U  L      L       "
                                       "\n  D     D  AAAAAAA     T     AAAAAAA        I         S     N  N N  U     U  L      L       "
                                       "\n  DDDDDD  A       A    T    A       A     IIIII  SSSSS      N   NN   UUUUU   LLLLLL LLLLLL  "
                                       "\n                                                                                            ";

static const char ERROR_MSG[]        = "\n                                          "
                                       "\n  EEEEEE  RRRR    RRRR     OOOO   RRRR    "
                                       "\n  E       R   R   R   R   O    O  R   R   "
                                       "\n  EEEE    RRRR    RRRR    O    O  RRRR    "
                                       "\n  E       R   R   R   R   O    O  R   R   "
                                       "\n  EEEEEE  R    R  R    R   OOOO   R    R  "
                                       "\n                                          ";

void tree_dump(Tree* tree, const char msg[], tree_err errcode)
{
    DUMP_ITERATION++;

    FILE* stream = DUMP_STREAM;
    if(stream == nullptr)
        return;
    
    PRINT("<span class = \"title\">\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------</span>\n");

    if(errcode)
    {
        PRINT("<span class = \"error\"> %s\n", ERROR_MSG);
        PRINT("%s (%d)\n", msg, errcode);
    }
    else    
        PRINT("<span class = \"title\"> %s\n", msg);

    PRINT("</span>\n");
    
    if(!tree)
    {
        PRINT("<span class = \"error\"> %s </span>\n", DATA_IS_NULL_MSG);
        PRINT("<span class = \"title\">\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------</span>\n");
        return;
    }

    PRINT("size: %lld\n" "capacity: %lld\n", tree->size, tree->cap);

    if(!tree->ptr_arr)
    {
        PRINT("<span class = \"error\"> %s </span>\n", DATA_IS_NULL_MSG);
        PRINT("<span class = \"title\">\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n</span>");
        return;
    }

    tree_graph_dump_(tree);

    PRINT(R"(<img src = ")" "%s" R"(" alt = "Graphical dump" width = 1080>)", graphviz_png_());

    PRINT("<span class = \"title\">\n\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n</span>");
}

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
        case LEX_POWER:
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
                case LEX_POWER:
                    PRINT(" ^{");
                    break;
                default:
                    PRINT(" %c ", node->lex.value.code);
                    break;
            }

            tree_tex_node_(node->right, node);

            switch(node->lex.value.code)
            {
                case LEX_DIV : case LEX_POWER :
                    PRINT("} ");
                    break;
                default:
                { /*fallthrough*/ }
            }

            if(get_priority_(node) <= get_priority_(parent))
                PRINT(")");

            break;
        case LEXT_IMMCONST:
            PRINT(" %lg ", node->lex.value.num);
            break;
        case LEXT_VAR:
            PRINT(" %s ", node->lex.value.name);
            break;
        default:
            assert(0);
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

#undef PRINT

static void close_dumpfile_()
{
    fprintf(DUMP_STREAM, "%s", HTML_OUTRO);

    if(fclose(DUMP_STREAM) != 0)
        perror("Tree dump file can't be succesfully closed");
}

void tree_dump_init(FILE* dumpstream)
{
    if(dumpstream)
    {
        DUMP_STREAM = dumpstream;
        return;
    }

    if(TREE_DUMPFILE[0] != 0)
    {
        DUMP_STREAM = fopen(TREE_DUMPFILE, "w");


        if(DUMP_STREAM)
        {
            atexit(&close_dumpfile_);
            fprintf(DUMP_STREAM, "%s", HTML_INTRO);
            
            return;
        }
    }

    perror("Can't open dump file");
    DUMP_STREAM = stderr;

    return;
}

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
