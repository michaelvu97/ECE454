#include "hashlife.h"
#include "stdlib.h"
#include "util.h"
#include "coolguyhash.h"
#include "node.h"

#define DIM 1024

/*
 * Implementation of Gosper's Hashlife algorithm (1984).
 */

Node* build_board(char* board, int depth, int upper_x, int upper_y)
{
    if (depth == 1)
    {
        return NODE(
            board[upper_y * DIM + upper_x] ? single_alive : single_dead,
            board[upper_y * DIM + upper_x + 1] ? single_alive : single_dead,
            board[(upper_y + 1) * DIM + upper_x] ? single_alive : single_dead,
            board[(upper_y + 1) * DIM + upper_x + 1] ? single_alive : single_dead
        );
    }

    int width_at_this_depth = 1 << (depth - 1);
    return NODE(
        build_board(board, depth - 1, upper_x, upper_y),
        build_board(board, depth - 1, upper_x + width_at_this_depth, upper_y),
        build_board(board, depth - 1, upper_x, upper_y + width_at_this_depth),
        build_board(board, depth - 1, upper_x + width_at_this_depth, upper_y + width_at_this_depth)
    );
}

void restore_board(Node* root, char* board, int depth, int upper_x, int upper_y)
{
    ASSERT(root);
    ASSERT(root->level == depth);
    if (depth == 1)
    {
        ASSERT(root->nw);
        ASSERT(root->nw->alive == 1 || root->nw->alive == 0);
        ASSERT(root->ne->alive == 1 || root->ne->alive == 0);
        ASSERT(root->sw->alive == 1 || root->sw->alive == 0);
        ASSERT(root->se->alive == 1 || root->se->alive == 0);

        board[DIM * upper_y + upper_x] = root->nw->alive;
        board[DIM * upper_y + upper_x + 1] = root->ne->alive;
        board[DIM * (upper_y + 1) + upper_x] = root->sw->alive;
        board[DIM * (upper_y + 1) + upper_x + 1] = root->se->alive;
        return;
    }

    int half_width_at_this_depth = 1 << (depth - 1);

    restore_board(root->nw, board, depth - 1, upper_x, upper_y);
    restore_board(root->ne, board, depth - 1, upper_x + half_width_at_this_depth, upper_y);
    restore_board(root->sw, board, depth - 1, upper_x, upper_y + half_width_at_this_depth);
    restore_board(root->se, board, depth - 1, upper_x + half_width_at_this_depth, upper_y + half_width_at_this_depth);
}

char* hashlife(char* outboard, char* inboard, const int gens_max)
{
    hash_init();
    /*
     * Build our board
     */
    Node* root = build_board(inboard, 10, 0, 0);

    restore_board(root, outboard, 10, 0, 0);

    // TODO
    return outboard;
}
