#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

typedef enum
{
    none,
    translateXY,
    rotateCW,
    mirrorX,
    mirrorY
} instruction_type;

typedef struct {
    instruction_type type;
    int argument_a; // Used by translate X and rotateCW
    int argument_b; // Only used by translate Y
} instruction;

typedef struct transformation {
    int a,b,c,d,e,f;
    /*
     *  |a c e|
     *  |b d f|
     *  |0 0 1|
     */
} transformation;


static inline transformation get_transformation(int a, int b, int c, int d, int e, int f)
{
    transformation t;
    t.a = a;
    t.b = b;
    t.c = c;
    t.d = d;
    t.e = e;
    t.f = f;
    return t;
}

// Inline?
static inline transformation get_identity()
{
    return get_transformation(1, 0, 0, 1, 0, 0);
}

// TODO: this will probably be a big source of optimization opportunity
// Multiplies A and B and stores in A.
static inline void compose_transformation(transformation* a, transformation* b)
{
    int a_prime = a->a*b->a + a->c*b->b;
    int b_prime = a->b*b->a + a->d*b->b;
    int c_prime = a->a*b->c + a->c*b->d;
    int d_prime = a->b*b->c + a->d*b->d;
    int e_prime = a->a*b->e + a->c*b->f + a->e;
    int f_prime = a->b*b->e + a->d*b->f + a->f;
    a->a = a_prime;
    a->b = b_prime;
    a->c = c_prime;
    a->d = d_prime;
    a->e = e_prime;
    a->f = f_prime;
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "0.1xer";

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

transformation rotation_to_transformation(int rotation, int image_side_length)
{
    rotation = rotation % 4;
    if (rotation < 0)
        rotation += 4;
    switch (rotation)
    {
        case 0:
            return get_identity();
        case 1:
            return get_transformation(0, 1, -1, 0, image_side_length - 1, 0);
        case 2:
            // Note for later: rotating by 180 is the same as mirroring across both axes
            return get_transformation(-1, 0, 0, -1, image_side_length - 1, image_side_length - 1);
        case 3:
            return get_transformation(0, -1, 1, 0, 0, image_side_length - 1);
    }
}

transformation instruction_to_transformation(instruction instr, int image_side_length)
{
    // Assumes that instr is not none.
    switch (instr.type)
    {
        case rotateCW:
           return rotation_to_transformation(instr.argument_a, image_side_length);
        case translateXY:
            return get_transformation(1, 0, 0, 1, instr.argument_a, instr.argument_b);
        case mirrorX:
            return get_transformation(-1, 0, 0, 1, image_side_length - 1, 0);
        case mirrorY:
            return get_transformation(1, 0, 0, -1, 0, image_side_length - 1);
    }

    printf("oops");
}

instruction parse_sensor_value(struct kv sensor_value)
{
    instruction instr;
    instr.type = none;
    instr.argument_a = 0;
    instr.argument_b = 0;

    char* key = sensor_value.key;
    int value = sensor_value.value;

    // TODO: optimize instead of using strcmp.  
    if (!strcmp(key, "W"))
    {
        if (value != 0)
        {
            instr.type = translateXY;
            instr.argument_b = -1 * value;
        }
    } else if (!strcmp(key, "S"))
    {
        if (value != 0)
        {
            instr.type = translateXY;
            instr.argument_b = value;
        }
    } else if (!strcmp(key, "D"))
    {
        if (value != 0)
        {
            instr.type = translateXY;
            instr.argument_a = value;
        }
    } else if (!strcmp(key, "A"))
    {
        if (value != 0)
        {
            instr.type = translateXY;
            instr.argument_a = -1 * value;
        }
    } else if (!strcmp(key, "CW"))
    {
        if (value != 0)
        {
            instr.type = rotateCW;
            instr.argument_a = value;
        }
    } else if (!strcmp(key, "CCW"))
    {
        if (value != 0)
        {
            instr.type = rotateCW;
            instr.argument_a = -1 * value;
        }
    } else if (!strcmp(key, "MX"))
    {
        instr.type = mirrorX;
    } else if (!strcmp(key, "MY"))
    {
        instr.type = mirrorY;
    }

    return instr;
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

    // TODO create custom int buffers instead of char*.
    int buffer_b_is_dest = 1;
    unsigned char* frame_buffer_b = (unsigned char*) malloc(sizeof(unsigned char*) * width * height);

    instruction instructionQueue[25];

    for (int frameIdx = 0; frameIdx < frames_to_process; frameIdx++)
    {   
        // Wipe the previous instruction queue
        for (int i = 0; i < 25; i++)
            instructionQueue[i].type = none;

        // Read the first instruction
        instructionQueue[0] = parse_sensor_value(sensor_values[frameIdx * 25]);

        int previousInstructionIndex = 0;

        for (int sensorIdx = 1; sensorIdx < 25; sensorIdx++)
        {
            instruction instr = parse_sensor_value(
                sensor_values[frameIdx * 25 + sensorIdx]
            );

            // Compare to the previous instruction.
            if (instr.type == instructionQueue[previousInstructionIndex].type)
            {
                // Update the previous instruction, skip this one.
                switch (instr.type)
                {
                    case translateXY:
                        instructionQueue[previousInstructionIndex].argument_b += instr.argument_b;
                    case rotateCW:
                        instructionQueue[previousInstructionIndex].argument_a += instr.argument_a;
                        break;
                    case mirrorX:
                    case mirrorY:
                        instructionQueue[previousInstructionIndex].type = none;
                        while (previousInstructionIndex > 0 && instructionQueue[previousInstructionIndex].type == none)
                            previousInstructionIndex--;
                        break;
                    default:
                        break;
                }
            } else if (instr.type != none)
            {
                instructionQueue[sensorIdx] = instr;
                previousInstructionIndex = sensorIdx;
            }
        }

        // For debugging
        // for (int i = 0; i < 25; i++)
        // {
        //     instruction instr = instructionQueue[i];
        //     switch (instr.type)
        //     {
        //         case none:
        //             printf("None,");
        //             break;
        //         case translateXY:
        //             printf("T(%d,%d),", instr.argument_a, instr.argument_b);
        //             break;
        //         case rotateCW:
        //             printf("R(%d),", instr.argument_a);
        //             break;
        //         case mirrorX:
        //             printf("M(X)");
        //             break;
        //         case mirrorY:
        //             printf("M(Y)");
        //             break;
        //         default:
        //             break;
        //     }
        // }
        // printf("\n");

        // Aggregate a transformation matrix for all the instructions, from left to right.
        transformation cumulative_transformation = get_identity();
        for (int i = 24; i >= 0; --i)
        {
            instruction instr = instructionQueue[i];
            if (instr.type == none)
                continue;

            transformation t = instruction_to_transformation(instr, width);

            compose_transformation(&cumulative_transformation, &t);
        }

        // For debugging
        printf("[%d %d %d]\n[%d %d %d]\n", cumulative_transformation.a,
            cumulative_transformation.c,
            cumulative_transformation.e,
            cumulative_transformation.b,
            cumulative_transformation.d,
            cumulative_transformation.f
        );

        unsigned char* dest_buffer = buffer_b_is_dest ? frame_buffer_b : frame_buffer;
        unsigned char* src_buffer = buffer_b_is_dest ? frame_buffer : frame_buffer_b;

        // Write white space. TODO: do optimized mask
        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                int base = 3 * (row * width + col);
                dest_buffer[base] = 0xff;
                dest_buffer[base + 1] = 0xff;
                dest_buffer[base + 2] = 0xff;
            }
        }

        // Push onto register
        int a = cumulative_transformation.a;
        int b = cumulative_transformation.b;
        int c = cumulative_transformation.c;
        int d = cumulative_transformation.d;
        int e = cumulative_transformation.e;
        int f = cumulative_transformation.f;

        // Transform buffer
        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                // possible todo: decompose into regions which are analyzed?
                int col_prime = a * col + c * row + e;
                int row_prime = b * col + d * row + f;

                if (col_prime < 0 || col_prime >= width || row_prime < 0 || row_prime >= height)
                    continue;

                int src = 3 * (row * width + col);
                int dest = 3 * (row_prime * width + col_prime);
                // Copy
                dest_buffer[dest] = src_buffer[src];
                dest_buffer[dest + 1] = src_buffer[src + 1];
                dest_buffer[dest + 2] = src_buffer[src + 2];
            }
        }
        
        verifyFrame(dest_buffer, width, height, grading_mode);
        buffer_b_is_dest = !buffer_b_is_dest;
    }

    return;
}
