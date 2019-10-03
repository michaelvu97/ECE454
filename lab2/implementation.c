#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

//#define DEBUGGING

typedef enum
{
    none,
    translateX,
    translateY,
    rotateCW,
    mirrorX,
    mirrorY
} instruction_type;

typedef struct 
{
    int x_bytes;
    int y_bytes;
    int length_bytes;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} segment_t;

typedef struct
{
    segment_t* segments;
    int length;
} dense_buffer_t;

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
    char team_name[] = ";SET speedup = 390 WHERE utorid='vumicha2';--";

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

static inline instruction parse_sensor_value(struct kv sensor_value)
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

/*
 *
 * Cache setters
 *
 */
static void compress_buffer(unsigned char* src_buffer, dense_buffer_t** dest_buffer, int width)
{
    #ifdef DEBUGGING
    printf("Compressing buffer\n");
    #endif

    int num_segments = 0;
    int triple_width = 3 * width;
    int byte_max = triple_width * width;

    // Count the exact number of segments
    for (int row_offset_bytes = 0; row_offset_bytes < byte_max; row_offset_bytes += triple_width)
    {
        // find row-continuous segments of the same colour. Map to the same struct.
        int curr_col = 0;

        while (curr_col < triple_width)
        {
            int src_offset = row_offset_bytes + curr_col;

            if (src_buffer[src_offset] == 0xff && src_buffer[src_offset + 1] == 0xff && src_buffer[src_offset + 2] == 0xff)
            {
                curr_col += 3;
                continue;
            }

            unsigned char last_r = src_buffer[src_offset];
            unsigned char last_g = src_buffer[src_offset + 1];
            unsigned char last_b = src_buffer[src_offset + 2];

            // Seek until the we reach the end of the row or a different coloured pixel
            int seek_offset = 3;
            int seek_offset_bytes = 3 + src_offset;
            while (curr_col + seek_offset < triple_width 
                && src_buffer[seek_offset_bytes] == last_r 
                && src_buffer[seek_offset_bytes + 1] == last_g
                && src_buffer[seek_offset_bytes + 2] == last_b)
            {
                seek_offset += 3;
                seek_offset_bytes += 3;
            }

            curr_col += seek_offset;
            num_segments++;
        }
    }

    dense_buffer_t* dest = *dest_buffer = malloc(sizeof(dense_buffer_t));
    dest->length = num_segments;
    segment_t* temp_dest_buffer = dest->segments = (segment_t*) malloc(sizeof(segment_t) * num_segments);

    int write_index = 0;

    for (int row_offset_bytes = 0; row_offset_bytes < byte_max; row_offset_bytes += triple_width)
    {
        // find row-continuous segments of the same colour. Map to the same struct.
        int curr_col_byte = 0;

        while (curr_col_byte < triple_width)
        {
            int src_offset = row_offset_bytes + curr_col_byte;

            // Ignore white pixels
            if (src_buffer[src_offset] == 0xff && src_buffer[src_offset + 1] == 0xff && src_buffer[src_offset + 2] == 0xff)
            {
                curr_col_byte += 3;
                continue;
            }

            // We are at the beginning of a non-white pixel.
            temp_dest_buffer[write_index].x_bytes = curr_col_byte;
            temp_dest_buffer[write_index].y_bytes = row_offset_bytes;
            unsigned char curr_r = temp_dest_buffer[write_index].r = src_buffer[src_offset];
            unsigned char curr_g = temp_dest_buffer[write_index].g = src_buffer[src_offset + 1];
            unsigned char curr_b = temp_dest_buffer[write_index].b = src_buffer[src_offset + 2];

            // Seek until the we reach the end of the row or a different coloured pixel
            int seek_offset_byte = 3;
            int seek_offset_bytes_total = 3 + src_offset; // TODO rm
            while (curr_col_byte + seek_offset_byte < triple_width 
                && src_buffer[seek_offset_bytes_total] == curr_r 
                && src_buffer[seek_offset_bytes_total + 1] == curr_g
                && src_buffer[seek_offset_bytes_total + 2] == curr_b)
            {
                seek_offset_byte += 3;
                seek_offset_bytes_total += 3;
            }
            temp_dest_buffer[write_index].length_bytes = seek_offset_byte;
            curr_col_byte += seek_offset_byte;
            write_index++;
        }
    }
#ifdef DEBUGGING
    printf("num segments: %d\n", num_segments);
#endif
}

static inline void setupBufferBRXY(unsigned char* src_buffer, dense_buffer_t** dest_buffer, int width)
{
    compress_buffer(src_buffer, dest_buffer, width);
}

static void setupBufferBRYX(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("BRYX\n");
#endif
    int triple_width = 3 * width;
    int y_end = triple_width * width;
    for (int src_y_byte = 0, dest_x = 0; src_y_byte < y_end; src_y_byte += triple_width, dest_x += 3)
    {
        int end = src_y_byte + triple_width;
        for (int src_x = src_y_byte, dest_y = dest_x; src_x < end; src_x += 3, dest_y += triple_width)
        {
            temp_dest_buffer[dest_y] = src_buffer[src_x];
            temp_dest_buffer[dest_y + 1] = src_buffer[src_x + 1];
            temp_dest_buffer[dest_y + 2] = src_buffer[src_x + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferBLXY(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("BLXY\n");
#endif
    int LAMBDA = width - 1;
    for (int src_y = 0, dest_y = 0; src_y < width; ++src_y, ++dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = LAMBDA; src_x < width; ++src_x, --dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_base];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_base + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_base + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferBLYX(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("BLYX\n");
#endif
    for (int src_y = 0, dest_x = width - 1; src_y < width; ++src_y, --dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = 0; src_x < width; ++src_x, ++dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_base];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_base + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_base + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferTRXY(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("TRXY\n");
#endif
    for (int src_y = 0, dest_y = width - 1; src_y < width; ++src_y, --dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = 0; src_x < width; ++src_x, ++dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_base];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_base + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_base + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferTRYX(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("TRYX\n");
#endif
    int LAMBDA = width - 1;
    for (int src_y = 0, dest_x = 0; src_y < width; ++src_y, ++dest_x)
    {
        int src_row_offset = src_y * width;
        for (int src_x = 0, dest_y = LAMBDA; src_x < width; ++src_x, --dest_y)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_base];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_base + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_base + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferTLXY(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("TLXY\n");
#endif
    int LAMBDA = width - 1;
    for (int src_y = 0, dest_y = LAMBDA; src_y < width; ++src_y, --dest_y)
    {
        int src_row_offset = src_y * width;
        int dest_row_offset = dest_y * width;
        for (int src_x = 0, dest_x = LAMBDA; src_x < width; ++src_x, --dest_x)
        {
            int src_base = 3 * (src_row_offset + src_x);
            int dest_base = 3 * (dest_row_offset + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_base];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_base + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_base + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
}

static void setupBufferTLYX(unsigned char* src_buffer, unsigned char* temp_dest_buffer, dense_buffer_t** dest_buffer, int width)
{
#ifdef DEBUGGING
    printf("TLYX\n");
#endif
    int LAMBDA = width - 1;
    int triple_width = 3 * width;
    for (int src_y = 0, dest_x = LAMBDA; src_y < width; ++src_y, --dest_x)
    {
        int src_row_offset = src_y * triple_width;
        int end = src_row_offset + triple_width;
        for (int src_x = src_row_offset, dest_y = LAMBDA; src_x < end; src_x += 3, --dest_y)
        {
            int dest_base = 3 * (dest_y * width + dest_x);
            temp_dest_buffer[dest_base] = src_buffer[src_x];
            temp_dest_buffer[dest_base + 1] = src_buffer[src_x + 1];
            temp_dest_buffer[dest_base + 2] = src_buffer[src_x + 2];
        }
    }
    compress_buffer(temp_dest_buffer, dest_buffer, width);
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

    dense_buffer_t* src_buffers[8];
    for (int i = 0; i < 8; i++)
    {
        // Buffers are null until they are initialized
        src_buffers[i] = 0x0;
    }

    unsigned char* dest_buffer = (unsigned char*) malloc(total_buffer_size);

    // Fill with white (can probably be optimized)
    int ws_len = total_buffer_size;
    while (ws_len-- > 0)
        dest_buffer[ws_len] = 0xff;

    // Used for building the rotated buffers.
    unsigned char* temp_buffer = (unsigned char*) malloc(total_buffer_size);

    int origin_x = 0, origin_y = 0;
    int unit_x_x = 1, unit_x_y = 0;
    int unit_y_x = 0, unit_y_y = 1;

    for (int frameIdx = 0; frameIdx < frames_to_process; frameIdx++)
    {   
        for (int sensorIdx = 0; sensorIdx < 25; sensorIdx++)
        {
            instruction instr = parse_sensor_value(
                sensor_values[frameIdx * 25 + sensorIdx]
            );

            int val = instr.argument;

            // TODO opt?
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

        int unit_x_x_dir = unit_x_x - origin_x;
        int unit_x_y_dir = unit_x_y - origin_y;
        int unit_y_x_dir = unit_y_x - origin_x;
        int unit_y_y_dir = unit_y_y - origin_y;

        int origin_x_bytes = 3 * origin_x;
        int origin_y_bytes = triple_width * origin_y;

        // TODO determine which to run in a more optimized way
        int src_buffer_offset_x_bytes;
        int src_buffer_offset_y_bytes;
        dense_buffer_t* current_src_buffer;

        #ifdef DEBUGGING
            printf("%d %d %d %d\n", unit_x_x_dir, unit_x_y_dir, unit_y_x_dir, unit_y_y_dir);
        #endif

        if (unit_x_x_dir > 0)
        {
            if (unit_y_y_dir > 0)
            {
                if (!src_buffers[BR_XY])
                    setupBufferBRXY(frame_buffer, &src_buffers[BR_XY], width);
                
                current_src_buffer = src_buffers[BR_XY];                
                src_buffer_offset_y_bytes = origin_y_bytes;
            }
            else
            {
                if (!src_buffers[TR_XY])
                    setupBufferTRXY(frame_buffer, temp_buffer, &src_buffers[TR_XY], width);

                current_src_buffer = src_buffers[TR_XY];
                src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
            }

            src_buffer_offset_x_bytes = origin_x_bytes;
        }
        else if (unit_x_x_dir < 0)
        {
            if (unit_y_y_dir > 0)
            {
                if (!src_buffers[BL_XY])
                    setupBufferBLXY(frame_buffer, temp_buffer, &src_buffers[BL_XY], width);

                current_src_buffer = src_buffers[BL_XY];
                src_buffer_offset_y_bytes = origin_y_bytes;
            }
            else
            {
                if (!src_buffers[TL_XY])
                    setupBufferTLXY(frame_buffer, temp_buffer, &src_buffers[TL_XY], width);

                current_src_buffer = src_buffers[TL_XY];
                src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
            }

            src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
        }
        else if (unit_x_y_dir > 0)
        {
            if (unit_y_x_dir > 0)
            {
                if (!src_buffers[BR_YX])
                    setupBufferBRYX(frame_buffer, temp_buffer, &src_buffers[BR_YX], width);

                current_src_buffer = src_buffers[BR_YX];
                src_buffer_offset_x_bytes = origin_x_bytes;
            }
            else
            {
                if (!src_buffers[BL_YX])
                    setupBufferBLYX(frame_buffer, temp_buffer, &src_buffers[BL_YX], width);

                current_src_buffer = src_buffers[BL_YX];
                src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
            }

            src_buffer_offset_y_bytes = origin_y_bytes;
        }
        else
        {
            if (unit_y_x_dir > 0)
            {
                if (!src_buffers[TR_YX])
                    setupBufferTRYX(frame_buffer, temp_buffer, &src_buffers[TR_YX], width);

                current_src_buffer = src_buffers[TR_YX];
                src_buffer_offset_x_bytes = origin_x_bytes;
            }
            else
            {
                if (!src_buffers[TL_YX])
                    setupBufferTLYX(frame_buffer, temp_buffer, &src_buffers[TL_YX], width);

                current_src_buffer = src_buffers[TL_YX];                
                src_buffer_offset_x_bytes = origin_x_bytes - TRIPLE_LAMBDA;
            }

            src_buffer_offset_y_bytes = origin_y_bytes - TRIPLE_WIDTH_LAMBDA;
        }

        #ifdef DEBUGGING
            printf("origin: (%d, %d), dest: (%d, %d)\n", origin_x, origin_y, src_buffer_offset_x_bytes, src_buffer_offset_y_bytes);
        #endif

        // Copy the transformed dense structure with offset
        int num_segments = current_src_buffer->length;
        segment_t* segments = current_src_buffer->segments;
        for (int i = 0; i < num_segments; ++i)
        {
            segment_t current_segment = segments[i];

            int start = (current_segment.x_bytes + src_buffer_offset_x_bytes) + (src_buffer_offset_y_bytes + current_segment.y_bytes);
            int end = current_segment.length_bytes + start;
    
            register unsigned char r = current_segment.r;
            register unsigned char g = current_segment.g;
            register unsigned char b = current_segment.b;   

            // Can optimize even further by calculating better bound
            for (; start < end; start += 3)
            {
                dest_buffer[start] = r;
                dest_buffer[start + 1] = g;
                dest_buffer[start + 2] = b;
            }
        }
        
        verifyFrame(dest_buffer, width, height, grading_mode);

        if (frameIdx != frames_to_process - 1)
        {
            for (int i = 0; i < num_segments; ++i)
            {
                segment_t current_segment = segments[i];

                int start = (current_segment.x_bytes + src_buffer_offset_x_bytes) + (src_buffer_offset_y_bytes + current_segment.y_bytes);
                int end = current_segment.length_bytes + start;

                for (; start < end; start += 3)
                {
                    dest_buffer[start] = 0xff;
                    dest_buffer[start + 1] = 0xff;
                    dest_buffer[start + 2] = 0xff;
                }
            }
        }
    }

    #ifdef DEBUGGING
        printf("Done!\n");
    #endif

    for (int i = 0; i < 8; i++) 
    {
        if (src_buffers[i])
            free(src_buffers[i]->segments);
        free(src_buffers[i]);
    }
    free(dest_buffer);
    free(temp_buffer);

    return;
}
 
