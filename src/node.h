#ifndef NODE_H
#define NODE_H

#include "util.h"

#define NODE(a,b,c,d) make_node(a,b,c,d)

typedef struct node {
    long population;
    int level;
    char alive;
    struct node* nw;
    struct node* ne;
    struct node* sw;
    struct node* se;

    struct node* result;
} Node;

Node* make_node(Node* nw, Node* ne, Node* sw, Node* se);
// Node* make_empty_node(int level);
Node* make_leaf(char alive);

Node* centered_subnode(Node* parent);
Node* centered_sub_sub_node(Node* parent);
Node* centered_horizontal(Node* w, Node* e);
Node* centered_vertical(Node* n, Node* s);

Node* next_generation(Node* node);

#endif