#include "hashlife.h"
#include "util.h"

/*
 * Implementation of Gosper's Hashlife algorithm (1984).
 */

#define TODO(s) \
    printf(s); \
    ASSERT(0)

#define NODE(a,b,c,d) make_node(a,b,c,d)

typedef struct node {
    int level;
    struct node* nw;
    struct node* ne;
    struct node* sw;
    struct node* se;
} Node;

static Node* make_node(Node* nw, Node* ne, Node* sw, Node* se)
{
    TODO("make_node");
}

static Node* centered_subnode(Node* parent)
{
    /*
     * Returns the centered subnode.
     *
     * 0000
     * 0XX0
     * 0XX0
     * 0000
     *
     */
    
    ASSERT(parent);
    ASSERT(parent->level >= 2);

    return NODE(
        parent->nw->se,
        parent->ne->sw,
        parent->sw->ne,
        parent->se->nw
    );
}

static Node* centered_sub_sub_node(Node* parent)
{
    /*
     * 0000 0000
     * 0000 0000
     * 0000 0000
     * 000X X000
     *
     * 000X X000
     * 0000 0000
     * 0000 0000
     * 0000 0000
     */

    ASSERT(parent);
    ASSERT(parent->level >= 3);

    return NODE(
        parent->nw->se->se,
        parent->ne->sw->sw,
        parent->sw->ne->ne,
        parent->se->nw->nw
    );
}

static Node* centered_horizontal(Node* w, Node* e)
{
    /*
     * West East
     * 0000 0000
     * 000X X000
     * 000X X000
     * 0000 0000
     */
    ASSERT(w);
    ASSERT(e);
    ASSERT(w->level >= 2);
    ASSERT(e->level == w->level);

    return NODE(
        w->ne->se,
        e->nw->sw,
        w->se->ne,
        e->sw->nw
    );
}

static Node* centered_vertical(Node* n, Node* s)
{
    /*
     * 0000
     * 0000
     * 0000
     * 0XX0 ^^North^^

     * 0XX0 vvSouthvv
     * 0000
     * 0000
     * 0000 
     */
    ASSERT(n);
    ASSERT(s);
    ASSERT(n->level >= 2);
    ASSERT(n->level == s->level);

    return NODE(
        n->sw->se,
        n->se->sw,
        s->nw->ne,
        s->ne->nw
    );
}

Node* next_generation(Node* node)
{
    ASSERT(node->level > 1);
    if (node->level == 2)
    {
        TODO("node base case");
    } else 
    {
        /*
         * Layout:
         *
         * 0000 0000
         * 0AAB BCC0
         * 0AAB BCC0
         * 0DDE EFF0
         *
         * 0DDE EFF0
         * 0GGH HII0
         * 0GGH HII0
         * 0000 0000
         *
         */
        Node* A = centered_subnode(node->nw);
        Node* B = centered_horizontal(node->nw, node->ne);
        Node* C = centered_subnode(node->ne);
        Node* D = centered_vertical(node->nw, node->sw);
        Node* E = centered_sub_sub_node(node);
        Node* F = centered_vertical(node->ne, node->se);
        Node* G = centered_subnode(node->sw);
        Node* H = centered_horizontal(node->sw, node->se);
        Node* I = centered_subnode(node->se);

        return NODE(
            next_generation(NODE(A, B, D, E)),
            next_generation(NODE(B, C, E, F)),
            next_generation(NODE(D, E, G, H)),
            next_generation(NODE(E, F, H, I))
        );
    }
}

char* hashlife(char* outboard, char* inboard, const int dim, const int gens_max)
{
    return 0;
}