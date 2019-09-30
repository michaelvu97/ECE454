#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

// #define DEBUGGING

typedef enum
{
    none,
    translateX,
    translateY,
    rotateCW,
    mirrorX,
    mirrorY
} instruction_type;

enum 
{
    BR_XY = 0,
    BR_YX = 1,
    TR_XY = 2,
    TR_YX = 3,
    TL_XY = 4,
    TL_YX = 5,
    BL_XY = 6,
    BL_YX = 7
};

typedef struct {
    instruction_type type;
    int argument; // Used by translateX,Y and rotateCW
} instruction;

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "😉";

    // Please fill in your information
    char student_first_name[] = "Michael";
    char student_last_name[] = "Vu";
    char student_student_number[] = "1002473272";

    // Printing out team information
    printf("*******************************************************************************************************\n");
    printf("Team Information:\n");
    printf("\tteam_name: %s\n", team_name);
    printf("\tstudent_first_name: %s\n", student_first_name);
    printf("\tstudent_last_name: %s\n", student_last_name);
    printf("\tstudent_student_number: %s\n", student_student_number);
}

instruction parse_sensor_value(struct kv sensor_value)
{
    instruction instr;
    instr.type = none;
    instr.argument = 0;

    char* key = sensor_value.key;

    char first = *key;
    if (first == 'M'){
        if (key[1] == 'X')
            instr.type = mirrorX;
        else 
            instr.type = mirrorY;

        return instr;
    }

    int value = sensor_value.value;

    if (first == 'C')
    {
        instr.type = rotateCW;

        // Rotation
        if (key[1] == 'C')
            // Counter-clockwise
            instr.argument = -1 * value;
        else
            // Clockwise
            instr.argument = value;

        return instr;
    }

    instr.type = translateX;

    switch (first)
    {
        case 'W':
            instr.type = translateY;
            instr.argument = -1 * value;
            break;
        case 'S':
            instr.type = translateY;
            instr.argument = value;
            break;
        case 'D':
            instr.argument = value;
            break;
        case 'A':
            instr.argument = -1 * value;
            break;
    }

    return instr;
}

static inline int max(int a, int b)
{
    return a > b ? a : b;
}

static inline int min(int a, int b)
{
    return a > b ? b : a;
}

void write_to_buffer_BR_x_y(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = -3 * min(0, origin_x);
    int source_y_min = -3 * min(0, origin_y);

    int source_x_max = 3 * (dim - max(0, origin_x));
    int source_y_max = 3 * (dim - max(0, origin_y));

    #ifdef DEBUGGING
    printf("br_x_y\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_min = 3 * max(0, origin_x);
    int dest_y_min = 3 * max(0, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_min; src_y < source_y_max; src_y += 3, dest_y += 3)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_min; src_x < source_x_max; src_x += 3, dest_x += 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y_offset + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_BR_y_x(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = min(0, origin_y) * -3;
    int source_y_min = min(0, origin_x) * -3;

    int source_x_max = 3 * (dim - max(0, origin_y));
    int source_y_max = 3 * (dim - max(0, origin_x));

    #ifdef DEBUGGING
    printf("br_y_x\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_min = 3 * max(0, origin_x);
    int dest_y_min = 3 * max(0, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_min; src_y < source_y_max; src_y += 3, dest_x += 3)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_min; src_x < source_x_max; src_x += 3, dest_y += 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dim * dest_y + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_BL_y_x(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;

    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = min(0, origin_y) * -3;
    int source_y_min = 3 * max(0, origin_x - dim_inclusive);

    int source_x_max = 3 * (dim - max(0, origin_y));
    int source_y_max = 3 * (dim - max(0, dim - origin_x));

    #ifdef DEBUGGING
    printf("bl_y_x\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = 3 * min(dim_inclusive, origin_x);
    int dest_y_start = 3 * max(0, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; src_y += 3, dest_x -= 3)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; src_x += 3, dest_y += 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dim * dest_y + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_BL_x_y(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;

    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = max(0, 3 * (origin_x - dim_inclusive));
    int source_y_min = max(0, -3 * origin_y);

    int source_x_max = 3 * (dim - max(0, dim - origin_x));
    int source_y_max = 3 * (dim - max(0, origin_y));

    #ifdef DEBUGGING
    printf("bl_x_y\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = 3 * min(dim_inclusive, origin_x);
    int dest_y_start = max(0, 3 * origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; src_y += 3, dest_y += 3)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; src_x += 3, dest_x -= 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y_offset + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_TR_y_x(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;

    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = 3 * max((origin_y - dim_inclusive), 0);
    int source_y_min = -3 * min(0, origin_x);

    int source_x_max = 3 * min(origin_y, dim);
    int source_y_max = 3 * (dim - max(0, origin_x));

    #ifdef DEBUGGING
    printf("tr_y_x\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = max(0, 3 * origin_x);
    int dest_y_start = 3 * min(origin_y, dim_inclusive);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; src_y += 3, dest_x += 3)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; src_x += 3, dest_y -= 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y * dim + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_TR_x_y(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;
    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = -3 * min(0, origin_x);
    int source_y_min = max(3 * (origin_y - dim_inclusive), 0);

    int source_x_max = 3 * (dim - max(0, origin_x));
    int source_y_max = 3 * min(origin_y, dim);

    #ifdef DEBUGGING
    printf("tr_x_y\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = max(0, 3 * origin_x);
    int dest_y_start = 3 * min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; src_y += 3, dest_y -= 3)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; src_x += 3, dest_x += 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y_offset + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

void write_to_buffer_TL_y_x(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;

    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = max(0, 3 * (origin_y - dim_inclusive));
    int source_y_min = max(0, 3 * (origin_x - dim_inclusive));

    int source_x_max = 3 * min(dim, origin_y + 1);
    int source_y_max = 3 * min(dim, origin_x + 1);

    #ifdef DEBUGGING
    printf("tl_y_x\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = 3 * min(dim_inclusive, origin_x);
    int dest_y_start = 3 * min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; src_y += 3, dest_x -= 3)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; src_x += 3, dest_y -= 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y * dim + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }

}

void write_to_buffer_TL_x_y(
    unsigned char* src_buffer,
    unsigned char* dest_buffer,
    int dim,
    int origin_x, int origin_y)
{
    int dim_inclusive = dim - 1;
    /*
     * Calculate the read bounds of the source image.
     */
    int source_x_min = max(0, 3 * (origin_x - dim_inclusive));
    int source_y_min = max(0, 3 * (origin_y - dim_inclusive));

    int source_x_max = 3 * min(dim, origin_x + 1);
    int source_y_max = 3 * min(dim, origin_y + 1);

    #ifdef DEBUGGING
    printf("tl_x_y\n");
    #endif

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = 3 * min(dim_inclusive, origin_x);
    int dest_y_start = 3 * min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; src_y += 3, dest_y -= 3)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; src_x += 3, dest_x -= 3)
        {
            int src_offset = src_y_offset + src_x;
            int dest_offset = dest_y_offset + dest_x;

            dest_buffer[dest_offset] = src_buffer[src_offset];
            dest_buffer[dest_offset + 1] = src_buffer[src_offset + 1];
            dest_buffer[dest_offset + 2] = src_buffer[src_offset + 2];
        }
    }
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          You can modify anything else in this file
 ***********************************************************************************************************************
 * @param sensor_values - structure stores parsed key value pairs of program instructions
 * @param sensor_values_count - number of valid sensor values parsed from sensor log file or commandline console
 * @param frame_buffer - pointer pointing to a buffer storing the imported  24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param grading_mode - turns off verification and turn on instrumentation
 ***********************************************************************************************************************
 *
 **********************************************************************************************************************/
void implementation_driver(
        struct kv *sensor_values,
        int sensor_values_count, 
        unsigned char *frame_buffer,
        unsigned int width,
        unsigned int height, 
        bool grading_mode)
{
    int processed_frames = 0;
    int frames_to_process = sensor_values_count / 25;

    int LAMBDA = width - 1;

    unsigned char* src_buffers[8];
    for (int i = 0; i < 8; i++)
    {
        src_buffers[i] = (unsigned char*) malloc(sizeof(unsigned char*) * width * height);
    }

    /*
     * Set up src buffers
     */

    // BR_XY (no transformation)
    unsigned char* temp_src_buffer = src_buffers[BR_XY];
    for (int row = 0; row < width; ++row)
    {
        int row_offset = row * width;
        for (int col = 0; col < width; ++col)
        {
            int base = 3 * (row_offset + col);
            temp_src_buffer[base] = frame_buffer[base];
            temp_src_buffer[base + 1] = frame_buffer[base + 1];
            temp_src_buffer[base + 2] = frame_buffer[base + 2];
        }
    }

    // BR_YX
    temp_src_buffer = src_buffers[BR_YX];
    for (int src_y = 0, dest_x = 0; src_y < width; ++src_y, ++dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = 0; src_x < width; ++src_x, ++dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // BL_XY
    temp_src_buffer = src_buffers[BL_XY];
    for (int src_y = 0, dest_y = 0; src_y < width; ++src_y, ++dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = LAMBDA; src_x < width; ++src_x, --dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // BL_YX
    temp_src_buffer = src_buffers[BL_YX];
    for (int src_y = 0, dest_x = LAMBDA; src_y < width; ++src_y, --dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = 0; src_x < width; ++src_x, ++dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // TR_XY
    temp_src_buffer = src_buffers[TR_XY];
    for (int src_y = 0, dest_y = LAMBDA; src_y < width; ++src_y, --dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = 0; src_x < width; ++src_x, ++dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // TR_YX
    temp_src_buffer = src_buffers[TR_YX];
    for (int src_y = 0, dest_x = 0; src_y < width; ++src_y, ++dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = LAMBDA; src_x < width; ++src_x, --dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // TL_XY
    temp_src_buffer = src_buffers[TL_XY];
    for (int src_y = 0, dest_y = LAMBDA; src_y < width; ++src_y, --dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = LAMBDA; src_x < width; ++src_x, --dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    // TL_XY
    temp_src_buffer = src_buffers[TL_YX];
    for (int src_y = 0, dest_x = LAMBDA; src_y < width; ++src_y, --dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = LAMBDA; src_x < width; ++src_x, --dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_src_buffer[dest_base] = frame_buffer[src_base];
            temp_src_buffer[dest_base + 1] = frame_buffer[src_base + 1];
            temp_src_buffer[dest_base + 2] = frame_buffer[src_base + 2];
        }
    }

    int origin_x = 0, origin_y = 0;
    int unit_x_x = 1, unit_x_y = 0;
    int unit_y_x = 0, unit_y_y = 1;

    for (int frameIdx = 0; frameIdx < frames_to_process; frameIdx++)
    {   
        // These can be reset every frame, since we use the last transformation
        // as the source.
        for (int sensorIdx = 0; sensorIdx < 25; sensorIdx++)
        {
            instruction instr = parse_sensor_value(
                sensor_values[frameIdx * 25 + sensorIdx]
            );

            int val = instr.argument;

            // TODO opt
            switch (instr.type)
            {
                case translateX:
                    origin_x += val;
                    unit_x_x += val;
                    unit_y_x += val;
                    break;
                case translateY:
                    origin_y += val;
                    unit_x_y += val;
                    unit_y_y += val;
                    break;
                case rotateCW:
                    // Constraint val to [0,3].
                    val = val % 4;
                    if (val < 0)
                        val += 4;
                    
                    if (val == 2)
                    {
                        origin_x = LAMBDA - origin_x;
                        origin_y = LAMBDA - origin_y;
                        unit_x_x = LAMBDA - unit_x_x;
                        unit_x_y = LAMBDA - unit_x_y;
                        unit_y_x = LAMBDA - unit_y_x;
                        unit_y_y = LAMBDA - unit_y_y;
                    }
                    else 
                    {
                        int temp_origin_x = origin_x, 
                        temp_unit_x_x = unit_x_x, 
                        temp_unit_y_x = unit_y_x;

                        if (val == 1)
                        {
                            origin_x = LAMBDA - origin_y;
                            origin_y = temp_origin_x;
                            unit_x_x = LAMBDA - unit_x_y;
                            unit_x_y = temp_unit_x_x;
                            unit_y_x = LAMBDA - unit_y_y;
                            unit_y_y = temp_unit_y_x;
                        } else if (val == 3)
                        {
                            origin_x = origin_y;
                            origin_y = LAMBDA - temp_origin_x;
                            unit_x_x = unit_x_y;
                            unit_x_y = LAMBDA - temp_unit_x_x;
                            unit_y_x = unit_y_y;
                            unit_y_y = LAMBDA - temp_unit_y_x;
                        }
                    }
                    break;
                case mirrorX:
                    origin_y = LAMBDA - origin_y;
                    unit_x_y = LAMBDA - unit_x_y;
                    unit_y_y = LAMBDA - unit_y_y;
                    break;
                case mirrorY:
                    origin_x = LAMBDA - origin_x;
                    unit_x_x = LAMBDA - unit_x_x;
                    unit_y_x = LAMBDA - unit_y_x;
                    break;
                default:
                    break;
            }
        }

        // Write white space. TODO: do optimized mask
        for (int row = 0; row < height; row++)
        {
            int row_start = row * width;
            for (int col = 0; col < width; col++)
            {
                int base = 3 * (row_start + col);
                frame_buffer[base] = 0xff;
                frame_buffer[base + 1] = 0xff;
                frame_buffer[base + 2] = 0xff;
            }
        }

        int unit_x_x_dir = unit_x_x - origin_x;
        int unit_x_y_dir = unit_x_y - origin_y;
        int unit_y_x_dir = unit_y_x - origin_x;
        int unit_y_y_dir = unit_y_y - origin_y;

        // TODO determine which to run in a more optimized way
        int buffer_type;
        int src_buffer_offset_x;
        int src_buffer_offset_y;
        unsigned char* current_src_buffer;

        if (unit_x_x_dir > 0)
        {
            if (unit_y_y_dir > 0)
            {
                buffer_type = BR_XY;
                src_buffer_offset_y = origin_y;
            }
            else
            {
                buffer_type = TR_XY;
                src_buffer_offset_y = origin_y - LAMBDA;
            }

            src_buffer_offset_x = origin_x;
        }
        else if (unit_x_x_dir < 0)
        {
            if (unit_y_y_dir > 0)
            {
                buffer_type = BL_XY;
                src_buffer_offset_y = origin_y;
            }
            else
            {
                buffer_type = TL_XY;
                src_buffer_offset_y = origin_y - LAMBDA;
            }

            src_buffer_offset_x = origin_x - LAMBDA;
        }
        else if (unit_x_y_dir > 0)
        {
            if (unit_y_x_dir > 0)
            {
                buffer_type = BR_YX;
                src_buffer_offset_x = origin_x;
            }
            else
            {
                buffer_type = BL_YX;
                src_buffer_offset_x = origin_x - LAMBDA;
            }

            src_buffer_offset_y = origin_y;                
        }
        else
        {
            if (unit_y_x_dir > 0)
            {
                buffer_type = TR_YX;
                src_buffer_offset_x = origin_x;
            }
            else
            {
                buffer_type = TL_YX;
                src_buffer_offset_x = origin_x - LAMBDA;
            }

            src_buffer_offset_y = origin_y - LAMBDA;
        }

        current_src_buffer = src_buffers[buffer_type];

        #ifdef DEBUGGING
            printf("buffer_type:%d, origin: (%d, %d), dest: (%d, %d)\n", buffer_type, origin_x, origin_y, src_buffer_offset_x, src_buffer_offset_y);
        #endif

        // TODO opt bounds
        for (int dest_y = src_buffer_offset_y, src_y = 0; src_y < width; ++src_y, ++dest_y)
        {
            if (dest_y < 0 || dest_y >= width)
                continue;

            int dest_y_offset = dest_y * width;
            int src_y_offset = src_y * width;

            for (int dest_x = src_buffer_offset_x, src_x = 0; src_x < width; ++src_x, ++dest_x)
            {
                if (dest_x < 0 || dest_x >= width)
                    continue;

                int dest_offset = 3 * (dest_y_offset + dest_x);
                int src_offset = 3 * (src_y_offset + src_x);
                frame_buffer[dest_offset] = current_src_buffer[src_offset];
                frame_buffer[dest_offset + 1] = current_src_buffer[src_offset + 1];
                frame_buffer[dest_offset + 2] = current_src_buffer[src_offset + 2];
            }
        }
        verifyFrame(frame_buffer, width, height, grading_mode);
    }

    for (int i = 0; i < 8; i++) 
    {
        free(src_buffers[i]);
    }

    return;
}
