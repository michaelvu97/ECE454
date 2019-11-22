#include "node.h"
#include "util.h"
#include "coolguyhash.h"
#include "stdlib.h"

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

    Node* result_node = lookup_or_insert(level, nw, ne, sw, se);
    ASSERT(result_node);

    return result_node;
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
    ASSERT(parent->level >= 2);

    return NODE(
        parent->nw->se->se,
        parent->ne->sw->sw,
        parent->sw->ne->ne,
        parent->se->nw->nw
    );
}


Node* centered_sub_sub_node_deep(Node* parent)
{
    /*
     * 0000 0000
     * 0000 0000
     * 00XX XX00
     * 00XX XX00
     *
     * 00XX XX00
     * 00XX XX00
     * 0000 0000
     * 0000 0000
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

Node* centered_horizontal_deep(Node* w, Node* e)
{
    /*
     * West East
     * 00XX XX00
     * 00XX XX00
     * 00XX XX00
     * 00XX XX00
     */
    ASSERT(w);
    ASSERT(e);
    ASSERT(w->level >= 2);
    ASSERT(e->level == w->level);

    return NODE(
        w->ne,
        e->nw,
        w->se,
        e->sw
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

Node* centered_vertical_deep(Node* n, Node* s)
{
    /*
     * 0000
     * 0000
     * XXXX
     * XXXX ^^North^^

     * XXXX vvSouthvv
     * XXXX
     * 0000
     * 0000 
     */
    ASSERT(n);
    ASSERT(s);
    ASSERT(n->level >= 2);
    ASSERT(n->level == s->level);

    return NODE(
        n->sw,
        n->se,
        s->nw,
        s->ne
    );
}

Node* next_generation(Node* node)
{
    ASSERT(node->level >= 2);

    if (node->result)
    {
        return node->result;
    }

    if (node->level == 2)
    {
        /*
         * The current node is a 4x4, we have to compute the inside board (2x2)
         * manually.
         * The performance here can also be improved.
         */
        Node* nw = node->nw;
        Node* ne = node->ne;
        Node* sw = node->sw;
        Node* se = node->se;

        char nw_pop = 
            nw->nw->alive + 
            nw->ne->alive + 
            nw->sw->alive +
            ne->nw->alive +
            ne->sw->alive +
            sw->nw->alive +
            sw->ne->alive +
            se->nw->alive;

        char ne_pop =
            nw->ne->alive +
            nw->se->alive +
            ne->nw->alive +
            ne->ne->alive +
            ne->se->alive +
            sw->ne->alive +
            se->nw->alive +
            se->ne->alive;

        char se_pop = 
            nw->se->alive +
            nw->sw->alive +
            ne->sw->alive +
            sw->nw->alive +
            sw->sw->alive +
            sw->se->alive +
            se->nw->alive +
            se->sw->alive;

        char sw_pop =
            nw->se->alive +
            ne->sw->alive +
            ne->se->alive +
            sw->ne->alive +
            sw->se->alive +
            se->ne->alive +
            se->sw->alive +
            se->se->alive;

        Node* result = NODE(
            alivep(nw_pop, nw->se->alive) ? single_alive : single_dead,
            alivep(ne_pop, ne->sw->alive) ? single_alive : single_dead,
            alivep(sw_pop, sw->ne->alive) ? single_alive : single_dead,
            alivep(se_pop, se->nw->alive) ? single_alive : single_dead
        );

        node->result = result;

        return result;
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

        
        Node* A = next_generation(node->nw);
        Node* B = next_generation(centered_horizontal_deep(node->nw, node->ne));
        Node* C = next_generation(node->ne);
        Node* D = next_generation(centered_vertical_deep(node->nw, node->sw));
        Node* E = next_generation(centered_sub_sub_node_deep(node));
        Node* F = next_generation(centered_vertical_deep(node->ne, node->se));
        Node* G = next_generation(node->sw);
        Node* H = next_generation(centered_horizontal_deep(node->sw, node->se));
        Node* I = next_generation(node->se);
        
        /*
        Node* A = centered_subnode(node->nw);
        Node* B = centered_horizontal(node->nw, node->ne);
        Node* C = centered_subnode(node->ne);
        Node* D = centered_vertical(node->nw, node->sw);
        Node* E = centered_sub_sub_node(node);
        Node* F = centered_vertical(node->ne, node->se);
        Node* G = centered_subnode(node->sw);
        Node* H = centered_horizontal(node->sw, node->se);
        Node* I = centered_subnode(node->se);
        */

        ASSERT(A->level == node->level - 2);
        ASSERT(B->level == node->level - 2);
        ASSERT(C->level == node->level - 2);
        ASSERT(D->level == node->level - 2);
        ASSERT(D->level == node->level - 2);
        ASSERT(F->level == node->level - 2);
        ASSERT(G->level == node->level - 2);
        ASSERT(H->level == node->level - 2);
        ASSERT(I->level == node->level - 2);

        ASSERT(A->result);

        Node* inner_node_nw = NODE(A, B, D, E);
        Node* inner_node_ne = NODE(B, C, E, F);
        Node* inner_node_sw = NODE(D, E, G, H);
        Node* inner_node_se = NODE(E, F, H, I);

        /*
        Node* inner_node_nw_result = inner_node_nw->result;
        if (!inner_node_nw_result)
        {
            inner_node_nw_result = next_generation(inner_node_nw);
            inner_node_nw->result = inner_node_nw_result;
        }

        Node* inner_node_sw_result = inner_node_sw->result;
        if (!inner_node_sw_result)
        {
            inner_node_sw_result = next_generation(inner_node_sw);
            inner_node_sw->result = inner_node_sw_result;
        }

        Node* inner_node_ne_result = inner_node_ne->result;
        if (!inner_node_ne_result)
        {
            inner_node_ne_result = next_generation(inner_node_ne);
            inner_node_ne->result = inner_node_ne_result;
        }

        Node* inner_node_se_result = inner_node_se->result;
        if (!inner_node_se_result)
        {
            inner_node_se_result = next_generation(inner_node_se);
            inner_node_se->result = inner_node_se_result;
        }*/

        Node* result = NODE(
            next_generation(inner_node_nw),
            next_generation(inner_node_ne),
            next_generation(inner_node_sw),
            next_generation(inner_node_se)
        );

        ASSERT(result->level == node->level - 1);

        node->result = result;
        return result;
    }
}
