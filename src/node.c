#include "node.h"
#include "util.h"
#include "coolguyhash.h"

static inline Node* new_node()
{
    return (Node*) malloc(sizeof(Node));
}

Node* make_node(Node* nw, Node* ne, Node* sw, Node* se)
{
    ASSERT(nw);
    ASSERT(ne);
    ASSERT(sw);
    ASSERT(se);

    int level = nw->level + 1; 

    Node* existing = lookup(level, nw, ne, sw, se);
    if (existing)
        return existing;

    Node* n = new_node();
    n->level = level;
    n->nw = nw;
    n->ne = ne;
    n->sw = sw;
    n->se = se;
    n->population = nw->population +
                    ne->population +
                    sw->population +
                    se->population;

    n->alive = n->population > 0;

    return n;
}

Node* make_leaf(char alive)
{
    Node* node = new_node();
    node->level = 0;
    node->nw = 0x0;
    node->ne = 0x0;
    node->sw = 0x0;
    node->se = 0x0;
    node->alive = node->population = alive;
    return node;
}

Node* centered_subnode(Node* parent)
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

Node* centered_sub_sub_node(Node* parent)
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

Node* centered_horizontal(Node* w, Node* e)
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

Node* centered_vertical(Node* n, Node* s)
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
    ASSERT(node->level >= 2);
    if (node->level == 2)
    {
        /*
         * The current node is a 4x4, we have to compute the inside board (2x2)
         * manually.
         * TODO memoization?
         * The performance here can also be improved.
         */
        char nw_pop = 
            node->nw->nw->alive + 
            node->nw->ne->alive + 
            node->nw->sw->alive +
            node->ne->nw->alive +
            node->ne->sw->alive +
            node->sw->nw->alive +
            node->sw->ne->alive +
            node->se->nw->alive;

        char ne_pop =
            node->nw->ne->alive +
            node->nw->se->alive +
            node->ne->nw->alive +
            node->ne->ne->alive +
            node->ne->se->alive +
            node->sw->ne->alive +
            node->se->nw->alive +
            node->se->ne->alive;

        char se_pop = 
            node->nw->se->alive +
            node->nw->sw->alive +
            node->ne->sw->alive +
            node->sw->nw->alive +
            node->sw->sw->alive +
            node->sw->se->alive +
            node->se->nw->alive +
            node->se->sw->alive;

        char sw_pop =
            node->nw->se->alive +
            node->ne->sw->alive +
            node->ne->se->alive +
            node->sw->ne->alive +
            node->sw->se->alive +
            node->se->ne->alive +
            node->se->sw->alive +
            node->se->se->alive;

        return NODE(
            alivep(nw_pop, node->nw->se->alive) ? single_alive : single_dead,
            alivep(ne_pop, node->ne->sw->alive) ? single_alive : single_dead,
            alivep(sw_pop, node->sw->ne->alive) ? single_alive : single_dead,
            alivep(se_pop, node->se->nw->alive) ? single_alive : single_dead
        );
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

        ASSERT(node->nw->level == node->level - 1);
        ASSERT(node->ne->level == node->level - 1);
        ASSERT(node->sw->level == node->level - 1);
        ASSERT(node->se->level == node->level - 1);

        Node* A = centered_subnode(node->nw);
        Node* B = centered_horizontal(node->nw, node->ne);
        Node* C = centered_subnode(node->ne);
        Node* D = centered_vertical(node->nw, node->sw);
        Node* E = centered_sub_sub_node(node);
        Node* F = centered_vertical(node->ne, node->se);
        Node* G = centered_subnode(node->sw);
        Node* H = centered_horizontal(node->sw, node->se);
        Node* I = centered_subnode(node->se);

        ASSERT(A->level == node->level - 2);
        ASSERT(B->level == node->level - 2);
        ASSERT(C->level == node->level - 2);
        ASSERT(D->level == node->level - 2);
        ASSERT(D->level == node->level - 2);
        ASSERT(F->level == node->level - 2);
        ASSERT(G->level == node->level - 2);
        ASSERT(H->level == node->level - 2);
        ASSERT(I->level == node->level - 2);

        return NODE(
            next_generation(NODE(A, B, D, E)),
            next_generation(NODE(B, C, E, F)),
            next_generation(NODE(D, E, G, H)),
            next_generation(NODE(E, F, H, I))
        );
    }
}
