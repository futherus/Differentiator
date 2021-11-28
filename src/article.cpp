#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include "article.h"

static const char TREE_TEXFILE[]  = "tree_tex.pdf";

#define PRINT(format, ...) fprintf(stream, format, ##__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////////////

static FILE* TEX_STREAM = nullptr;

static const char NOTE_BEFORE_DRVTV[]  = "Возьмем для примера несложную производную";
static const char NOTE_BEFORE_CUTTER[] = "Проведя несколько элементарных выкладок над данным выражением";
static const char NOTE_BEFORE_RESULT[] = "Получаем довольно очевидный результат";
static const char NOTE_AFTER_RESULT[]  = "Дальнейшие выкладки оставляем читателю в качестве упражнения";

static const char* NOTE_ARR[]  = {
    "Из школьного курса алгебры известно",
    "Из очевидной симметрии",
    "Как известно",
    "Найдем произвольную функцию такую, что",
    "Дважды применим теорему 3.5",
    "Из геометрических соображений",
    "Тупо скатав из Корявова",
    "Из этого следует",
    "Легко понять",
    "Легко видеть",
    "Не теряя общности можем сказать, что",
    "Как легко убедиться",
    "В самом деле",
    "Действительно",
    "Аналогичное рассуждение показывает, что",
    "С другой стороны",
    "Отметим одно очевидное умозаключение, которое часто будет встречаться в дальнейшем",
    "По третьему закону Кеплера",
    "Приводя подобные",
    "Воспользовавшись алгеброй логики",
    "Применяя 367 метод Султанова",
    "Следующее утверждение сразу вытекает из формул (16) \\textsection 4 гл. 1 т.3",
};

static char TEX_TEMP_FILE[] = "tree_dump/tex_temp.tex";

static const char TEX_INTRO[] = 
R"(
\documentclass[12pt]{article}
\usepackage[utf8]{inputenc}
\usepackage[russian]{babel}
\usepackage{geometry} 
\geometry{a4paper} 
\usepackage{graphicx}

\begin{document}

\begin{titlepage}
   \begin{center}
        \sffamily
        \itshape
        \Huge
        А. Л. Симанкович
        \vspace*{1cm}
        
        \textbf{ОСНОВЫ МАТЕМАТИЧЕСКОГО  КАТАРСИСА}
       
        \vspace{0.5cm}
        
        \textbf{1}
            
        \vspace{1.5cm}
        \large
        Издание девятое, \\
        стереотипное

        \vfill
        \normalfont
        \normalsize
        \begin{minipage}{10cm}
        \begin{center}
        Допущено министерством образования Республики Беларусь в качестве учебника для студентов лучшего высшего учебного заведения, невысыпающихся по направлениям подготовки и специальностям в области сверхЪестественных наук и математики, техники и технологий, образования и педагогики  
        \end{center}
        \end{minipage}
        
        \vspace{2cm}
        \includegraphics[scale = 0.08]{logo.jpg}
        \vspace{0.5cm}
        
        2021
   \end{center}
\end{titlepage}

)";

static const char TEX_OUTRO[] = 
R"(
\end{document}
)";

#define DEF_PAREN(SYMB, CODE)         \
    case (LEX_##CODE):                \

#define DEF_FUNC(HASH, NAME)          \
    case(LEX_##NAME):                 \

static int get_priority_(Node* check)
{
    if(!check)
        return 0;

    switch(check->lex.code)
    {
        case LEX_ADD : case LEX_SUB :
            return 1;
            
        case LEX_MUL : case LEX_DIV :
            return 2;

        case LEX_POW:
            return 3;

        #include "reserved_functions.inc"
            return 4;
            
        case LEX_VAR : case LEX_IMMCONST :
            return 5;

        case LEX_NOCODE: 
                        #include "reserved_parentheses.inc"
        {
            assert(0);
        }

        default: 
        { /*fallthrough*/ }
    }
    
    return 0;
}

static void tex_node_(Node* node, Node* parent)
{
    assert(node);
    
    FILE* stream = TEX_STREAM;

    bool is_parentheses = false;
    bool is_derivative  = false;

    if(get_priority_(node) <= get_priority_(parent))
        is_parentheses = true;
    else
        is_parentheses = false;

    if(parent)
        if(node->lex.location.head != parent->lex.location.head)
        {
            is_derivative  = true;
            is_parentheses = false;
        }

    if(is_derivative)
        PRINT("(");

    switch(node->lex.code)
    {
        case LEX_DIV:
            PRINT("\\frac{");
            tex_node_(node->left, node);
            PRINT("}{");
            tex_node_(node->right, node);
            PRINT("}");
            break;

        case LEX_ADD:
            tex_node_(node->left, node);
            PRINT(" + ");
            tex_node_(node->right, node);
            break;

        case LEX_SUB:
            tex_node_(node->left, node);
            PRINT(" - ");
            tex_node_(node->right, node);
            break;

        case LEX_MUL:
            tex_node_(node->left, node);
            PRINT(" \\cdot ");
            tex_node_(node->right, node);
            break;

        case LEX_POW:
            tex_node_(node->left, node);
            PRINT("^{");
            tex_node_(node->right, node);
            PRINT("}");
            break;

        #include "reserved_functions.inc"
            PRINT("%s(", demangle(&node->lex));
            tex_node_(node->left, node);
            PRINT(")");
            break;

        case LEX_IMMCONST : case LEX_VAR :
            PRINT("%s", demangle(&node->lex));
            break;

        default : case LEX_NOCODE :
                #include "reserved_parentheses.inc"
        {
            assert(0);
        }
    }

    if(is_derivative)
        PRINT(")'");
}

void article_expression(Tree* tree, article_enum num)
{
    assert(tree);
    assert(tree->root);

    FILE* stream = TEX_STREAM;
    if(stream == nullptr)
        return;

    switch(num)
    {
        case ARTICLE_BEFORE_DRVTV:
            PRINT("%s\n"
                  "\\begin{equation}\n"
                  "(",
                   NOTE_BEFORE_DRVTV);

            tex_node_(tree->root, nullptr);

            PRINT(")'\n"
                  "\\end{equation}\n");

            return;

        case ARTICLE_BEFORE_CUTTER:
            PRINT("%s\n"
                  "\\begin{equation}\n",
                  NOTE_BEFORE_CUTTER);

            tex_node_(tree->root, nullptr);

            PRINT("\n\\end{equation}\n");

            return;

        case ARTICLE_RESULT:
            PRINT("%s\n"
                  "\\begin{equation}\n",
                  NOTE_BEFORE_RESULT);

            tex_node_(tree->root, nullptr);


            PRINT("\n\\end{equation}\n"
                  "%s\n",
                  NOTE_AFTER_RESULT);

            return;
            
        case ARTICLE_RANDOM:
        {
            num = (article_enum) (rand() % (sizeof(NOTE_ARR) / sizeof(char*)));

            PRINT("%s\n", NOTE_ARR[num]);

            PRINT("\\begin{equation}\n");

            int coin = rand() % 7;
            if(coin == 0)
                PRINT("\\rotatebox{180}{$\n");

            tex_node_(tree->root, nullptr);

            if(coin == 0)
                PRINT("\n$}\n");

            PRINT("\\end{equation}\n");

            return;
        }
        default:
            assert(0);
    }
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

void article_init()
{
    if(TREE_TEXFILE[0] != 0)
    {
        TEX_STREAM = fopen(TEX_TEMP_FILE, "w");

        if(TEX_STREAM)
        {
            atexit(&close_texfile_);
            fprintf(TEX_STREAM, "%s", TEX_INTRO);
            srand((unsigned int) time(nullptr));

            return;
        }

    }

    perror("Can't open tex file");

    return;
}
