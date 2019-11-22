#include "util.h"
#include "node.h"
#include "coolguyhash.h"
#include "stdlib.h"

struct hash_node {
    Node* val;
    struct hash_node* next;
};

typedef struct coolguyhash {
    int capacity;
    struct hash_node** contents;
} CoolGuyHash;

/*
 * Hash bins:
 * Level = 0-11
 */
CoolGuyHash bins[12];

Node* single_alive = 0x0;
Node* single_dead = 0x0;

void hash_init()
{
    for (int i = 0; i < 12; i++)
    {
        bins[i].capacity = 1 << i;
        bins[i].contents = (struct hash_node**) calloc(1 << i, sizeof(struct hash_node*));
        for (int j = (1 << i) - 1; j >= 0; j--)
        {   
            bins[i].contents[j] = 0x0;
        }
    }

    single_alive = make_leaf(1);
    single_dead = make_leaf(0);
}

unsigned long hashcode(Node* nw, Node* ne, Node* sw, Node* se)
{
    // TODO fix this
    return
        (unsigned long) nw +
        (unsigned long) ne +
        (unsigned long) sw +
        (unsigned long) se;
}

Node* lookup(int level, Node* nw, Node* ne, Node* sw, Node* se)
{
    ASSERT(level >= 0);
    ASSERT(level < 12);

    long hash_code = hashcode(nw, ne, sw, se);

    CoolGuyHash bin = bins[level];
    long index = hash_code % bin.capacity;

    // Search
    struct hash_node* iter = bin.contents[index];
    while (iter)
    {
        Node* n = iter->val;
        if (n->nw == nw && n->ne == ne && n->sw == sw && n->se == se)
        {
            return n;
        }
        iter = iter->next;
    }

    return 0x0;
}