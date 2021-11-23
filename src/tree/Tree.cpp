#include "Tree.h"
#include "../dumpsystem/dumpsystem.h"

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define ASSERT(CONDITION, ERROR)                \
    do                                          \
    {                                           \
        if(!(CONDITION))                        \
        {                                       \
            tree_dump(tree, #ERROR, (ERROR));   \
            return (ERROR);                     \
        }                                       \
    } while(0)                                  \

static tree_err ptr_arr_resize_(Tree* tree)
{
    assert(tree);
    
    ptrdiff_t new_cap = tree->ptr_arr_cap;
    if(new_cap == 0)
        new_cap = TREE_PTR_ARR_MIN_CAP;

    Node** temp = (Node**) realloc(tree->ptr_arr, new_cap * sizeof(Node*));
    ASSERT(temp, TREE_BAD_ALLOC);
    
    memset(temp + tree->ptr_arr_cap, 0, new_cap - tree->ptr_arr_cap);

    tree->ptr_arr     = temp;
    tree->ptr_arr_cap = new_cap;
    
    return TREE_NOERR;
}

static tree_err add_chunk_(Tree* tree)
{
    assert(tree);

    if(tree->cap / TREE_CHUNK_SIZE == tree->ptr_arr_cap)
        if(ptr_arr_resize_(tree))
            return TREE_BAD_ALLOC;
    
    Node* temp = (Node*) calloc(TREE_CHUNK_SIZE, sizeof(Node));
    ASSERT(temp, TREE_BAD_ALLOC);

    tree->ptr_arr[tree->cap / TREE_CHUNK_SIZE] = temp;
    tree->cap += TREE_CHUNK_SIZE;
    
    return TREE_NOERR;
}

tree_err tree_init(Tree* tree, Lexem root_data)
{
    ASSERT(tree, TREE_NULLPTR);
    ASSERT(!tree->ptr_arr, TREE_REINIT);

    if(add_chunk_(tree))
        return TREE_BAD_ALLOC;

    tree->ptr_arr[0][0] = {root_data, nullptr, nullptr};
    tree->size = 1;
    tree->root = &tree->ptr_arr[0][0];

    return TREE_NOERR;
}

tree_err tree_dstr(Tree* tree)
{
    ASSERT(tree, TREE_NULLPTR);
    ASSERT(tree->ptr_arr, TREE_NOTINIT);

    for(ptrdiff_t iter = 0; iter < tree->cap / TREE_CHUNK_SIZE; iter++)
        free(tree->ptr_arr[iter]);
    
    return TREE_NOERR;
}

tree_err tree_add(Tree* tree, Node** base_ptr, Lexem data)
{
    ASSERT(tree && base_ptr, TREE_NULLPTR);
    ASSERT(tree->ptr_arr, TREE_NOTINIT);

    if(tree->cap == tree->size)
        if(add_chunk_(tree))
            return TREE_BAD_ALLOC;

    Node* new_node = &tree->ptr_arr[tree->size / TREE_CHUNK_SIZE][tree->size % TREE_CHUNK_SIZE];
    *new_node  = {data, nullptr, nullptr};
    *base_ptr  = new_node;

    tree->size++;

    return TREE_NOERR;
}

static void (*VISITOR_FUNCTION_)(Node*, size_t depth) = nullptr;
static size_t VISITOR_DEPTH_ = -1;

static void tree_visitor_(Node* node)
{
    VISITOR_DEPTH_++;    

    VISITOR_FUNCTION_(node, VISITOR_DEPTH_);

    if(node->right)
        tree_visitor_(node->right);

    if(node->left)
        tree_visitor_(node->left);

    VISITOR_DEPTH_--;
}

tree_err tree_visitor(Tree* tree, void (*function)(Node* node, size_t depth))
{
    ASSERT(tree && function, TREE_NULLPTR);
    ASSERT(tree->ptr_arr, TREE_NOTINIT);

    VISITOR_FUNCTION_ = function;
    VISITOR_DEPTH_    = -1;

    tree_visitor_(tree->root);

    return TREE_NOERR;
}

/*
static Stack* PATH_STACK = nullptr;
static bool tree_find_(Node* node, const char data[])
{
    stack_push(PATH_STACK, node);

    if(strcmp(node->data, data) == 0)
        return true;
    
    if(node->left)
        if(tree_find_(node->left, data))
            return true;
    
    if(node->right)
        if(tree_find_(node->right, data))
            return true;

    void* temp = nullptr;
    stack_pop(PATH_STACK, &temp);

    return false;
}

tree_err tree_find(Tree* tree, const char data[], Stack* path_stk)
{
    ASSERT(tree && data && path_stk, TREE_NULLPTR);

    PATH_STACK = path_stk;

    tree_find_(tree->root, data);

    return TREE_NOERR;
}
*/