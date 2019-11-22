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

int bin_capacities[] = {
    0,
    16,
    1 << 4,
    1 << 16,
    1 << 8,
    1 << 8,
    1 << 8,
    1 << 8,
    1 << 8,
    1 << 8,
    10000,
    10000,
};

void hash_init()
{
    for (int i = 0; i < 12; i++)
    {
        bins[i].capacity = bin_capacities[i];
        bins[i].contents = (struct hash_node**) calloc(bin_capacities[i], sizeof(struct hash_node*));
    }

    single_alive = make_leaf(1);
    single_dead = make_leaf(0);
}

unsigned long hashcode(unsigned long nw, unsigned long ne, unsigned long sw, unsigned long se)
{
    // TODO fix this
    return (nw >> 4) + ne + (sw >> 2) + se;
}

Node* lookup_or_insert(int level, Node* nw, Node* ne, Node* sw, Node* se)
{
    ASSERT(level > 0);
    ASSERT(level < 12);

    long hash_code = hashcode((unsigned long)nw, (unsigned long)ne, (unsigned long)sw, (unsigned long)se);

    CoolGuyHash bin = bins[level];
    long index = hash_code % bin.capacity;
    ASSERT(index >= 0);
    ASSERT(index < bin.capacity);

    // Search
    struct hash_node* iter = bin.contents[index];
    struct hash_node* prev = 0x0;
    while (iter)
    {
        Node* n = iter->val;
        if (n->nw == nw && n->ne == ne && n->sw == sw && n->se == se)
   
        {
            return n;
        }
        prev = iter;
        iter = iter->next;
    }

    Node* n = malloc(sizeof(Node));
    n->level = level;
    n->nw = nw;
    n->ne = ne;
    n->sw = sw;
    n->se = se;
    n->result = 0x0; // Not computed yet.
    n->population = nw->population +
                    ne->population +
                    sw->population +
                    se->population;

    n->alive = n->population > 0;

    struct hash_node* new_hash_node = malloc(sizeof(struct hash_node));
    new_hash_node->val = n;
    new_hash_node->next = 0x0;

    if (prev == 0x0)
    {
        // Insert to list head.
        bin.contents[index] = new_hash_node;
    } else {
        // Insert to end of list
        prev->next = new_hash_node;
    }

    return n;
}
