#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

// #define DEBUGGING

#define INSTR_translateX 1
#define INSTR_translateY 2
#define INSTR_rotateCW 3
#define INSTR_mirrorX 4
#define INSTR_mirrorY 5

typedef struct 
{
    int offset; // x_bytes + y_bytes.
    unsigned char r;
    unsigned char g;
    unsigned char b;
} segment_t;

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

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "jackson";

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
    int total_buffer_size = 3 * width * width;
    int LAMBDA = width - 1;
    int triple_width = width * 3;
    int TRIPLE_LAMBDA = 3 * LAMBDA;
    int TRIPLE_WIDTH_LAMBDA = width * TRIPLE_LAMBDA;


    int num_pixels = 0;
    for (int i = 0; i < total_buffer_size; i += 3)
    {

        // TODO opt
        if (frame_buffer[i] == 0xff
            && frame_buffer[i + 1] == 0xff
            && frame_buffer[i + 2] == 0xff)
            continue;
        num_pixels++;
    }

    #ifdef DEBUGGING
        printf("num_pixels: %d\n", num_pixels);
    #endif

    segment_t* src_buffers[8];
    for (int i = 0; i < 8; i++)
    {
        src_buffers[i] = (segment_t*) malloc(sizeof(segment_t) * num_pixels);
    }

    int write_index = 0;

    for (
        int src_row = 0,
            row_width = 0,
            inverse_row = TRIPLE_LAMBDA,
            inverse_row_width = TRIPLE_WIDTH_LAMBDA; 

            src_row < triple_width;

            src_row += 3,
            row_width += triple_width,
            inverse_row -= 3,
            inverse_row_width -= triple_width)
    {
        for (
            int src_col = 0, 
                col_width = 0,
                inverse_col = TRIPLE_LAMBDA,
                inverse_col_width = TRIPLE_WIDTH_LAMBDA; 

                src_col < triple_width; 

                src_col += 3, 
                col_width += triple_width,
                inverse_col -= 3,
                inverse_col_width -= triple_width)
        {
            int src_base = row_width + src_col;
            register unsigned char r = frame_buffer[src_base];
            register unsigned char g = frame_buffer[src_base + 1];
            register unsigned char b = frame_buffer[src_base + 2];

            if (r == 0xff && g == 0xff && b == 0xff)
                continue;

            // TODO reduce multiplication

            // BRXY
            src_buffers[BR_XY][write_index].offset = src_base;
            src_buffers[BR_XY][write_index].r = r;
            src_buffers[BR_XY][write_index].g = g;
            src_buffers[BR_XY][write_index].b = b;

            // BRYX
            src_buffers[BR_YX][write_index].offset = col_width + src_row;
            src_buffers[BR_YX][write_index].r = r;
            src_buffers[BR_YX][write_index].g = g;
            src_buffers[BR_YX][write_index].b = b;

            // TRXY
            src_buffers[TR_XY][write_index].offset = inverse_row_width + src_col;
            src_buffers[TR_XY][write_index].r = r; 
            src_buffers[TR_XY][write_index].g = g;
            src_buffers[TR_XY][write_index].b = b;

            // TRYX
            src_buffers[TR_YX][write_index].offset = src_row + inverse_col_width;
            src_buffers[TR_YX][write_index].r = r;
            src_buffers[TR_YX][write_index].g = g;
            src_buffers[TR_YX][write_index].b = b;

            // TLXY
            src_buffers[TL_XY][write_index].offset = inverse_col + inverse_row_width;
            src_buffers[TL_XY][write_index].r = r;
            src_buffers[TL_XY][write_index].g = g;
            src_buffers[TL_XY][write_index].b = b;

            // TLYX
            src_buffers[TL_YX][write_index].offset = inverse_row + inverse_col_width;
            src_buffers[TL_YX][write_index].r = r;
            src_buffers[TL_YX][write_index].g = g;
            src_buffers[TL_YX][write_index].b = b;

            // BLXY
            src_buffers[BL_XY][write_index].offset = inverse_col + row_width;
            src_buffers[BL_XY][write_index].r = r;
            src_buffers[BL_XY][write_index].g = g;
            src_buffers[BL_XY][write_index].b = b;

            // BLYX
            src_buffers[BL_YX][write_index].offset = inverse_row + col_width;
            src_buffers[BL_YX][write_index].r = r;
            src_buffers[BL_YX][write_index].g = g;
            src_buffers[BL_YX][write_index].b = b;

            write_index++;
        }
    }
    // Fill with white (can probably be optimized)
    int ws_len = total_buffer_size;
    while (ws_len-- > 0)
        frame_buffer[ws_len] = 0xff;

    int origin_x = 0, origin_y = 0;
    int unit_x_x = 1, unit_x_y = 0;
    int unit_y_x = 0, unit_y_y = 1;

    int frame_end = frames_to_process * 25;

    for (int frameIdx = 0; frameIdx < frame_end; frameIdx += 25)
    {
        int sensor_index_end = frameIdx + 25;   
        for (int sensorIdx = frameIdx; sensorIdx < sensor_index_end; sensorIdx++)
        {
            unsigned char type;
            int argument;

            // TODO: unroll further?
            char* key = sensor_values[sensorIdx].key;

            char first = *key;
            if (first == 'M'){
                if (key[1] == 'X')
                    type = INSTR_mirrorX;
                else 
                    type = INSTR_mirrorY;
            } else {
                argument = sensor_values[sensorIdx].value;
                if (first == 'C')
                {
                    type = INSTR_rotateCW;

                    // Rotation
                    if (key[1] == 'C')
                        // Counter-clockwise
                        argument = -1 * argument;
                } else {
                    switch (first)
                    {
                        case 'W':
                            argument = -1 * argument;
                        case 'S':
                            type = INSTR_translateY;
                            break;
                        case 'A':
                            argument = -1 * argument;
                        case 'D':
                            type = INSTR_translateX;
                            break;
                    }
                }
            }

            switch (type)
            {
                case INSTR_translateX:
                    origin_x += argument;
                    unit_x_x += argument;
                    unit_y_x += argument;
                    break;
                case INSTR_translateY:
                    origin_y += argument;
                    unit_x_y += argument;
                    unit_y_y += argument;
                    break;
                case INSTR_rotateCW:
                    // Constrain argument to [0,3].
                    argument = argument % 4;
                    if (argument < 0)
                        argument += 4;
                    
                    if (argument == 2)
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

                        if (argument == 1)
                        {
                            origin_x = LAMBDA - origin_y;
                            origin_y = temp_origin_x;
                            unit_x_x = LAMBDA - unit_x_y;
                            unit_x_y = temp_unit_x_x;
                            unit_y_x = LAMBDA - unit_y_y;
                            unit_y_y = temp_unit_y_x;
                        } else if (argument == 3)
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
                case INSTR_mirrorX:
                    origin_y = LAMBDA - origin_y;
                    unit_x_y = LAMBDA - unit_x_y;
                    unit_y_y = LAMBDA - unit_y_y;
                    break;
                case INSTR_mirrorY:
                    origin_x = LAMBDA - origin_x;
                    unit_x_x = LAMBDA - unit_x_x;
                    unit_y_x = LAMBDA - unit_y_x;
                    break;
                default:
                    break;
            }
        }

        int unit_x_x_dir = unit_x_x - origin_x;
        int unit_x_y_dir = unit_x_y - origin_y;
        int unit_y_x_dir = unit_y_x - origin_x;
        int unit_y_y_dir = unit_y_y - origin_y;

        int origin_x_bytes = 3 * origin_x;
        int origin_y_bytes = triple_width * origin_y;

        // TODO determine which to run in a more optimized way
        int src_buffer_offset_x_bytes;
        int src_buffer_offset_y_bytes;
        segment_t* current_src_buffer;

        #ifdef DEBUGGING
            printf("%d %d %d %d\n", unit_x_x_dir, unit_x_y_dir, unit_y_x_dir, unit_y_y_dir);
        #endif

        if (unit_x_x_dir > 0)
        {
            if (unit_y_y_dir > 0)
            {
                current_src_buffer = src_buffers[BR_XY];                
                src_buffer_offset_y_bytes = origin_y_bytes;
                #ifdef DEBUGGING
                    printf("BR_XY\n");
                #endif
            }
            else
            {
                current_src_buffer = src_buffers[TR_XY];
                src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
                #ifdef DEBUGGING
                    printf("TR_XY\n");
                #endif
            }

            src_buffer_offset_x_bytes = origin_x_bytes;
        }
        else if (unit_x_x_dir < 0)
        {
            if (unit_y_y_dir > 0)
            {
                #ifdef DEBUGGING
                    printf("BL_XY\n");
                #endif
                current_src_buffer = src_buffers[BL_XY];
                src_buffer_offset_y_bytes = origin_y_bytes;
            }
            else
            {
                current_src_buffer = src_buffers[TL_XY];
                src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
                #ifdef DEBUGGING
                    printf("TL_XY\n");
                #endif
            }

            src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
        }
        else if (unit_x_y_dir > 0)
        {
            if (unit_y_x_dir > 0)
            {
                current_src_buffer = src_buffers[BR_YX];
                src_buffer_offset_x_bytes = origin_x_bytes;
                #ifdef DEBUGGING
                    printf("BR_YX\n");
                #endif
            }
            else
            {
                current_src_buffer = src_buffers[BL_YX];
                src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
                #ifdef DEBUGGING
                    printf("BL_YX\n");
                #endif
            }

            src_buffer_offset_y_bytes = origin_y_bytes;
        }
        else
        {
            if (unit_y_x_dir > 0)
            {
                current_src_buffer = src_buffers[TR_YX];
                src_buffer_offset_x_bytes = origin_x_bytes;
                #ifdef DEBUGGING
                    printf("TR_YX\n");
                #endif
            }
            else
            {
                current_src_buffer = src_buffers[TL_YX];                
                src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
                #ifdef DEBUGGING
                    printf("TL_YX\n");
                #endif
            }

            src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
        }

        #ifdef DEBUGGING
            printf("origin: (%d, %d), dest: (%d, %d)\n", origin_x, origin_y, src_buffer_offset_x_bytes, src_buffer_offset_y_bytes);
        #endif

        int base_offset = src_buffer_offset_x_bytes + src_buffer_offset_y_bytes;

        // Copy the transformed dense structure with offset
        segment_t* current_src_buffer_iter = current_src_buffer;
        segment_t* current_src_buffer_iter_end = current_src_buffer_iter + num_pixels;
        while (current_src_buffer_iter < current_src_buffer_iter_end)
        {
            segment_t current_segment = *current_src_buffer_iter;
            register int start = current_segment.offset + base_offset;
        
            frame_buffer[start] = current_segment.r;
            frame_buffer[start + 1] = current_segment.g;
            frame_buffer[start + 2] = current_segment.b;
            current_src_buffer_iter++;
        }

        verifyFrame(frame_buffer, width, height, grading_mode);

        if (frameIdx != frames_to_process - 1)
        {
            current_src_buffer_iter = current_src_buffer;
            while (current_src_buffer_iter < current_src_buffer_iter_end)
            {
                register int start = current_src_buffer_iter->offset + base_offset;

                frame_buffer[start] = 0xff;
                frame_buffer[start + 1] = 0xff;
                frame_buffer[start + 2] = 0xff;

                current_src_buffer_iter++;
            }
        }
    }

    #ifdef DEBUGGING
        printf("Done!\n");
    #endif

    for (int i = 0; i < 8; i++) 
    {
        if (src_buffers[i])
            free(src_buffers[i]);
    }

    return;
}
 
