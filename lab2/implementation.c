#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

typedef enum
{
    none,
    translateX,
    translateY,
    rotateCW,
    mirrorX,
    mirrorY
} instruction_type;

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
    int source_x_min = min(0, origin_x) * -1;
    int source_y_min = min(0, origin_y) * -1;

    int source_x_max = dim - max(0, origin_x);
    int source_y_max = dim - max(0, origin_y);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_min = max(0, origin_x);
    int dest_y_min = max(0, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_min; src_y < source_y_max; ++src_y, ++dest_y)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_min; src_x < source_x_max; ++src_x, ++dest_x)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y_offset + dest_x);

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
    int source_x_min = min(0, origin_y) * -1;
    int source_y_min = min(0, origin_x) * -1;

    int source_x_max = dim - max(0, origin_y);
    int source_y_max = dim - max(0, origin_x);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_min = max(0, origin_x);
    int dest_y_min = max(0, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_min; src_y < source_y_max; ++src_y, ++dest_x)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_min; src_x < source_x_max; ++src_x, ++dest_y)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dim * dest_y + dest_x);

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
    int source_x_min = min(0, origin_y) * -1;
    int source_y_min = max(0, origin_x - dim_inclusive);

    int source_x_max = dim - max(0, origin_y);
    int source_y_max = dim - max(0, dim - origin_x);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = min(dim_inclusive, origin_x);
    int dest_y_start = max(0, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; ++src_y, --dest_x)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; ++src_x, ++dest_y)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dim * dest_y + dest_x);

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
    int source_x_min = max(0, origin_x - dim_inclusive);
    int source_y_min = max(0, -1 * origin_y);

    int source_x_max = dim - max(0, dim - origin_x);
    int source_y_max = dim - max(0, origin_y);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = max(0, min(dim_inclusive, origin_x));
    int dest_y_start = max(0, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; ++src_y, ++dest_y)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; ++src_x, --dest_x)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y_offset + dest_x);

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
    int source_x_min = max(origin_y - dim_inclusive, 0);
    int source_y_min = min(0, origin_x) * -1;

    int source_x_max = min(origin_y, dim);
    int source_y_max = dim - max(0, origin_x);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = max(0, origin_x);
    int dest_y_start = min(origin_y, dim_inclusive);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; ++src_y, ++dest_x)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; ++src_x, --dest_y)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y * dim + dest_x);

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
    int source_x_min = min(0, origin_x) * -1;
    int source_y_min = max(origin_y - dim_inclusive, 0);

    int source_x_max = dim - max(0, origin_x);
    int source_y_max = min(origin_y, dim); // ?

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = max(0, origin_x);
    int dest_y_start = min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; ++src_y, --dest_y)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; ++src_x, ++dest_x)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y_offset + dest_x);

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
    int source_x_min = max(0, origin_y - dim_inclusive);
    int source_y_min = max(0, origin_x - dim_inclusive);

    int source_x_max = min(dim, origin_y + 1);
    int source_y_max = min(dim, origin_x + 1);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = min(dim_inclusive, origin_x);
    int dest_y_start = min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_x = dest_x_start; src_y < source_y_max; ++src_y, --dest_x)
    {
        int src_y_offset = src_y * dim;
        for (int src_x = source_x_min, dest_y = dest_y_start; src_x < source_x_max; ++src_x, --dest_y)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y * dim + dest_x);

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
    int source_x_min = max(0, origin_x - dim_inclusive);
    int source_y_min = max(0, origin_y - dim_inclusive);

    int source_x_max = min(dim, origin_x + 1);
    int source_y_max = min(dim, origin_y + 1);

    /*
     * Calculate the write bounds for the dest image
     */
    int dest_x_start = min(dim_inclusive, origin_x);
    int dest_y_start = min(dim_inclusive, origin_y);

    for (int src_y = source_y_min, dest_y = dest_y_start; src_y < source_y_max; ++src_y, --dest_y)
    {
        int src_y_offset = src_y * dim;
        int dest_y_offset = dest_y * dim;
        for (int src_x = source_x_min, dest_x = dest_x_start; src_x < source_x_max; ++src_x, --dest_x)
        {
            int src_offset = 3 * (src_y_offset + src_x);
            int dest_offset = 3 * (dest_y_offset + dest_x);

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

    int buffer_b_is_dest = 1;
    // this is temporary, for debugging the different transformation
    unsigned char* frame_buffer_a = (unsigned char*) malloc(sizeof(unsigned char*) * width * height);
    unsigned char* frame_buffer_b = (unsigned char*) malloc(sizeof(unsigned char*) * width * height);

    // TODO remove
    // copy frame buffer into_a
    int limit = 3 * width * height;
    for (int i = 0; i < limit; i++)
    {
        frame_buffer_a[i] = frame_buffer[i];
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

        unsigned char* dest_buffer = buffer_b_is_dest ? frame_buffer_b : frame_buffer_a;

        // TODO switch back
        unsigned char* src_buffer = frame_buffer;

        // Write white space. TODO: do optimized mask
        for (int row = 0; row < height; row++)
        {
            int row_start = row * width;
            for (int col = 0; col < width; col++)
            {
                int base = 3 * (row_start + col);
                dest_buffer[base] = 0xff;
                dest_buffer[base + 1] = 0xff;
                dest_buffer[base + 2] = 0xff;
            }
        }

        int unit_x_x_dir = unit_x_x - origin_x;
        int unit_x_y_dir = unit_x_y - origin_y;
        int unit_y_x_dir = unit_y_x - origin_x;
        int unit_y_y_dir = unit_y_y - origin_y;

        // TODO determine which to run in a more optimized way
        if (unit_x_x_dir > 0)
        {
            if (unit_y_y_dir > 0)
                write_to_buffer_BR_x_y(src_buffer, dest_buffer, width, origin_x, origin_y);
            else
                write_to_buffer_TR_x_y(src_buffer, dest_buffer, width, origin_x, origin_y);
        }
        else if (unit_x_x_dir < 0)
        {
            if (unit_y_y_dir > 0)
                write_to_buffer_BL_x_y(src_buffer, dest_buffer, width, origin_x, origin_y);
            else
                write_to_buffer_TL_x_y(src_buffer, dest_buffer, width, origin_x, origin_y);
        }
        else if (unit_x_y_dir > 0)
        {
            if (unit_y_x_dir > 0)
                write_to_buffer_BR_y_x(src_buffer, dest_buffer, width, origin_x, origin_y);
            else
                write_to_buffer_BL_y_x(src_buffer, dest_buffer, width, origin_x, origin_y);
        }
        else
        {
            if (unit_y_x_dir > 0)
                write_to_buffer_TR_y_x(src_buffer, dest_buffer, width, origin_x, origin_y);
            else
                write_to_buffer_TL_y_x(src_buffer, dest_buffer, width, origin_x, origin_y);
        }
        
        verifyFrame(dest_buffer, width, height, grading_mode);
        buffer_b_is_dest = !buffer_b_is_dest;
    }

    free(frame_buffer_a);
    free(frame_buffer_b);

    return;
}
