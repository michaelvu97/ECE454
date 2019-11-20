#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#include "string.h"

static void update_custom_borders(char* custom_board, int dim)
{
    int outer_dim = dim + 2;
    char* inner_start = custom_board + outer_dim + 1;
    
    // TODO optimize

    /*
     * Corners
     */

    // TL
    custom_board[0] = inner_start[outer_dim * (dim - 1) + dim - 1];

    // TR
    custom_board[outer_dim - 1] = inner_start[outer_dim * (dim - 1)];

    // BL
    custom_board[outer_dim * (dim + 1)] = inner_start[dim - 1];

    // BR
    custom_board[outer_dim * outer_dim - 1] = inner_start[0];

    /*
     * Edges
     */

    // Top edge
    memcpy(custom_board + 1, inner_start + (outer_dim * (dim - 1)), dim);

    // Bottom edge
    memcpy(custom_board + 1 + (outer_dim * (outer_dim - 1)), inner_start, dim);

    // Left edge
    char* src = inner_start + (dim - 1);
    char* dest = inner_start - 1;
    while (src != inner_start + (outer_dim * dim) + (dim - 1))
    {
        *dest = *src;
        src += outer_dim;
        dest += outer_dim;
    }

    // Right edge
    src = inner_start;
    dest = inner_start + dim;
    while (src != inner_start + (outer_dim * dim))
    {
        *dest = *src;
        src += outer_dim;
        dest += outer_dim;
    }
}

static void convert_to_custom(char* source_board, char* custom_board, const int dim)
{
    int outer_dim = dim + 2;

    for (int row = 0; row < dim; ++row)
    {
        for (int col = 0; col < dim; ++col)
        {
            custom_board[(row + 1) * outer_dim + col + 1] = source_board[row * dim + col];
        }
    }

    update_custom_borders(custom_board, dim);
}

static void convert_from_custom(char* custom_board, char* target_board, const int dim)
{
    int outer_dim = dim + 2;
    char* start = custom_board + 1 + outer_dim;

    for (int row = 0; row < dim; ++row)
    {
        for (int col = 0; col < dim; ++col)
        {
            target_board[row * dim + col] = start[row * outer_dim + col];
        }
    }
}

#endif