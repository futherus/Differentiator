#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include "differentiator.h"
#include "dumpsystem/dumpsystem.h"

static const char TREE_TEXFILE[] = "tree_tex.pdf";

static const size_t NOMINATOR_SUBTREE_SIZE = 30;

#define PRINT(format, ...) fprintf(stream, format, ##__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////////////

static FILE* TEX_STREAM = nullptr;
static char  DIFF_VARIABLE[FILENAME_MAX] = "";

static const char NOTE_BEFORE_DRVTV[]  = "Возьмем от него производную";
static const char NOTE_BEFORE_CUTTER[] = "Проведя несколько элементарных выкладок над данным выражением";
static const char NOTE_BEFORE_RESULT[] = "Получаем довольно очевидный результат";
static const char NOTE_AFTER_RESULT[]  = "Дальнейшие выкладки оставляем читателю в качестве упражнения";

struct Substitution
{
    Node* node = nullptr;
    Lexem lex  = {};
};

#define DEF_SUBST(HASH, NAME) LEX_##NAME,
static const Lexem_code SUBST_CODES[] = 
{
    #include "reserved_substitutions.inc"
};
#undef DEF_SUBST

static const int SUBST_CODES_SZ = sizeof(SUBST_CODES) / sizeof(Lexem_code);

static const char* NOTE_ARR[] =
{
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
\usepackage{amsmath}
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

#define DEF_SUBST(HASH, NAME)         \
    case(LEX_##NAME):                 \

static int get_priority_(Node* check)
{
    if(!check)
        return 0;

    switch(check->lex.code)
    {
        case LEX_DIV : case LEX_POW :
            return 1;

        case LEX_ADD:
            return 2;
            
        case LEX_SUB:
            return 3;
        
        case LEX_MUL:
            return 4;
        
        #include "reserved_functions.inc"
            return 5;
        
        case LEX_VAR : case LEX_IMMCONST :
                #include "reserved_substitutions.inc"
        {
            return 6;
        }

        case LEX_NOCODE: case LEX_EOF :
                        #include "reserved_parentheses.inc"
        {
            assert(0);
        }

        default: 
        { /*fallthrough*/ }
    }
    
    return 0;
}

static Substitution* SUBST_ARR = nullptr;
static int SUBST_ARR_SZ = 0;

static int substitution_indx(Node* node)
{
    assert(SUBST_ARR && node);

    for(int iter = 0; iter < SUBST_ARR_SZ; iter++)
        if(SUBST_ARR[iter].node == node)
            return iter;

    return -1;
}

static void tex_node_(Node* node, Node* parent)
{
    assert(node);
    
    FILE* stream = TEX_STREAM;

    int indx = substitution_indx(node);
    if(indx != -1 && parent)
    {
        PRINT("%s", demangle(&SUBST_ARR[indx].lex));
        return;
    }

    bool is_derivative  = false;
    bool is_paren = false;

    if(parent)
    {
        if(node->lex.location.head != parent->lex.location.head)
            is_derivative  = true;

        is_paren = (parent->lex.code == LEX_POW && parent->left == node);
    }

    if(is_derivative)
        PRINT("(");

    switch(node->lex.code)
    {
        case LEX_DIV:
        {
            if(is_paren)
                PRINT("(");

            PRINT("\\frac{");
            tex_node_(node->left, node);
            PRINT("}{");
            tex_node_(node->right, node);
            PRINT("}");

            if(is_paren)
                PRINT(")");

            break;
        }
        case LEX_ADD:
        {
            is_paren |= (get_priority_(node) < get_priority_(parent));

            if(is_paren)
                PRINT("(");

            tex_node_(node->left, node);
            PRINT(" + ");
            tex_node_(node->right, node);

            if(is_paren)
                PRINT(")");

            break;
        }
        case LEX_SUB:
        {
            is_paren |= (get_priority_(node) <= get_priority_(parent));

            if(is_paren)
                PRINT("(");

            tex_node_(node->left, node);
            PRINT(" - ");
            tex_node_(node->right, node);

            if(is_paren)
                PRINT(")");

            break;
        }
        case LEX_MUL:
        {
            is_paren |= (get_priority_(node) < get_priority_(parent));

            if(is_paren)
                PRINT("(");

            tex_node_(node->left, node);
            PRINT(" \\cdot ");
            tex_node_(node->right, node);

            if(is_paren)
                PRINT(")");

            break;
        }
        case LEX_POW:
        {
            is_paren = (get_priority_(parent) >= 5); 

            if(is_paren)
                PRINT("(");

            tex_node_(node->left, node);
            PRINT("^{");
            tex_node_(node->right, node);
            PRINT("}");
            
            if(is_paren)
                PRINT(")");

            break;
        }
        #include "reserved_functions.inc"
        {
            if(is_paren)
                PRINT("(");

            PRINT("%s{", demangle(&node->lex));
            tex_node_(node->left, node);
            PRINT("}");

            if(is_paren)
                PRINT(")");

            break;
        }
        case LEX_IMMCONST : case LEX_VAR :
            PRINT("%s", demangle(&node->lex));
            break;

        default : case LEX_NOCODE : case LEX_EOF :
                #include "reserved_parentheses.inc"
                #include "reserved_substitutions.inc"
        {
            assert(0);
        }
    }
    
    if(is_derivative)
        PRINT(")'_{%s}", DIFF_VARIABLE);
}

static void article_expression_(Tree* tree, Node* subroot, const char* prefix)
{
    assert(tree && subroot && TEX_STREAM);

    FILE* stream = TEX_STREAM;

    PRINT("\\begin{equation}\n");

    int coin = rand() % 7;
    if(coin == 0)
        PRINT("\\rotatebox{180}{$\n");

    if(prefix)
        PRINT("%s =", prefix);

    tex_node_(subroot, nullptr);

    if(coin == 0)
        PRINT("\n$}\n");

    PRINT("\\end{equation}\n");

    return;
}

static diff_err nominator_(Tree* tree, Node* node, size_t* subtree_sz)
{
    assert(tree && node && subtree_sz);
    assert(SUBST_ARR); 

    size_t left_sz  = 0;
    size_t right_sz = 0;

    if(node->left)
        PASS$(!nominator_(tree, node->left, &left_sz), return DIFF_PASS_ERR; );
    
    if(node->right)
        PASS$(!nominator_(tree, node->right, &right_sz), return DIFF_PASS_ERR; );
 
    if(right_sz + left_sz > NOMINATOR_SUBTREE_SIZE && node != tree->root)
    {
        ASSERT_RET$(SUBST_ARR_SZ <= SUBST_CODES_SZ, DIFF_AUTOVAR_OVRFLW);

        SUBST_ARR[SUBST_ARR_SZ].node      = node;
        SUBST_ARR[SUBST_ARR_SZ].lex       = {node->lex};
        SUBST_ARR[SUBST_ARR_SZ].lex.code  = SUBST_CODES[SUBST_ARR_SZ];

        *subtree_sz = 0;
        (SUBST_ARR_SZ)++;
    }
    else
    {
        (*subtree_sz) += (left_sz + right_sz);
    }

    (*subtree_sz)++;

    return DIFF_NOERR;
}

static diff_err nominator(Tree* tree)
{
    assert(tree && SUBST_ARR);

    size_t tree_sz = 0;

    PASS$(!nominator_(tree, tree->root, &tree_sz), return DIFF_PASS_ERR; );

    return DIFF_NOERR;
}

diff_err article_record(Tree* tree, article_enum note, const char* prefix)
{
    ASSERT_RET$(tree && TEX_STREAM, DIFF_NULLPTR);
    
    FILE* stream = TEX_STREAM;

    SUBST_ARR = (Substitution*) calloc(SUBST_CODES_SZ, sizeof(Substitution));
    ASSERT_RET$(SUBST_ARR, DIFF_BAD_ALLOC);
    SUBST_ARR_SZ = 0;

    nominator(tree);

    switch(note)
    {
        case ARTICLE_BEFORE_DRVTV:
            PRINT("%s\n", NOTE_BEFORE_DRVTV);
            break;

        case ARTICLE_BEFORE_CUTTER:
            PRINT("%s\n", NOTE_BEFORE_CUTTER);
            break;

        case ARTICLE_RANDOM:
        {
            int coin = rand() % (sizeof(NOTE_ARR) / sizeof(char*));

            PRINT("%s\n", NOTE_ARR[coin]);

            break;
        }
        case ARTICLE_RESULT:
            PRINT("%s\n", NOTE_BEFORE_RESULT);
            break;

        case ARTICLE_NO_NOTE:
            break;

        default:
            assert(0);
    }

    article_expression_(tree, tree->root, prefix);

    if(SUBST_ARR_SZ)
    {
        PRINT("где\n");

        for(int iter = 0; iter < SUBST_ARR_SZ; iter++)
            article_expression_(tree, SUBST_ARR[iter].node, demangle(&SUBST_ARR[iter].lex));
    }

    PRINT("\\vskip 1cm\n");

    if(note == ARTICLE_RESULT)
        PRINT("%s\n", NOTE_AFTER_RESULT);

    free(SUBST_ARR);

    return DIFF_NOERR;
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

void article_init(const char diff_var[])
{
    if(TREE_TEXFILE[0] != 0)
    {
        TEX_STREAM = fopen(TEX_TEMP_FILE, "w");

        if(TEX_STREAM)
        {
            atexit(&close_texfile_);
            fprintf(TEX_STREAM, "%s", TEX_INTRO);
            memcpy(DIFF_VARIABLE, diff_var, strlen(diff_var));

            srand((unsigned int) time(nullptr));

            return;
        }
    }

    perror("Can't open tex file");

    return;
}
