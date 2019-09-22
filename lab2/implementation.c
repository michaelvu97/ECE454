#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

unsigned char *processMoveUp(unsigned char *buffer_frame, unsigned width, 
    unsigned height, int offset);

unsigned char *processMoveRight(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processMoveDown(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processMoveLeft(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processRotateCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                               int rotate_iteration);

unsigned char *processRotateCCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                                int rotate_iteration);

unsigned char *processMirrorX(unsigned char *buffer_frame, unsigned int width, unsigned int height);

unsigned char *processMirrorY(unsigned char *buffer_frame, unsigned width, unsigned height);

/*******************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 
 *     24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up. Must be > 0.
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the
 *     image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 ******************************************************************************/
unsigned char *processMoveUp(unsigned char *buffer_frame, unsigned width, 
    unsigned height, int offset) {

    int offsetBytes = offset * 3;

    // Write from left to right, top to bottom
    for (int row = 0; row < height - offset; ++row)
    {
        int rowPixel = row * height;
        for (int column = 0; column < width; ++column)
        {
            int dest = 3 * (rowPixel + column);
            int source = dest + (offsetBytes * width);
            buffer_frame[dest] = buffer_frame[source];
            buffer_frame[dest + 1] = buffer_frame[source + 1];
            buffer_frame[dest + 2] = buffer_frame[source + 2];
        }
    }

    // Fill in white pixels
    for (int row = height - offset; row < height; ++row)
    {
        int rowPixel = row * width;
        for (int column = 0; column < width; ++column)
        {
            int base_byte = 3 * (rowPixel + column);
            buffer_frame[base_byte] = 0xff;
            buffer_frame[base_byte + 1] = 0xff;
            buffer_frame[base_byte + 2] = 0xff;
        }
    }

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image left. Must be > 0.
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveRight(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
    
    int offsetBytes = offset * 3;

    // Write from right to left, bottom to top
    for (int row = height - 1; row >= 0; --row) 
    {
        int rowPixel = row * height;
        for (int column = width - 1 - offset; column >= 0; --column)
        {
            int source = 3 * (rowPixel + column);
            int target = source + offsetBytes;
            buffer_frame[target] = buffer_frame[source];
            buffer_frame[target + 1] = buffer_frame[source + 1];
            buffer_frame[target + 2] = buffer_frame[source + 2];
        }
    }

    // Fill in white pixels
    for (int row = 0; row < height; ++row)
    {
        int rowPixel = row * width;
        for (int column = 0; column < offset; ++column)
        {
            int base = 3 * (rowPixel + column);
            buffer_frame[base] = 0xff;      
            buffer_frame[base + 1] = 0xff;
            buffer_frame[base + 2] = 0xff;
        }
    }

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up. Must be > 0.
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveDown(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {

    int offsetBytes = offset * 3;

    // Write from right to left, bottom to top.
    for (int row = height - 1; row >= offset; --row)
    {
        int rowPixel = row * height;
        for (int column = width - 1; column >= 0; --column)
        {
            int dest = 3 * (rowPixel + column);
            int source = dest - (offsetBytes * width);
            buffer_frame[dest] = buffer_frame[source];
            buffer_frame[dest + 1] = buffer_frame[source + 1];
            buffer_frame[dest + 2] = buffer_frame[source + 2];
        }
    }

    // Fill in white pixels
    for (int row = 0; row < offset; ++row)
    {
        int rowPixel = row * width;
        for (int column = 0; column < width; ++column)
        {
            int base_byte = 3 * (column + rowPixel);
            buffer_frame[base_byte] = 0xff;
            buffer_frame[base_byte + 1] = 0xff;
            buffer_frame[base_byte + 2] = 0xff;
        }
    }

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image right. Must be > 0.
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveLeft(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {

    int offsetBytes = offset * 3;

    // Write from left to right, top to bottom
    for (int row = 0; row < height; ++row)
    {
        int rowPixel = row * height;
        for (int column = 0; column < width - offset; ++column) 
        {
            int dest = 3 * (rowPixel + column);
            int source = dest + offsetBytes;
            buffer_frame[dest] = buffer_frame[source];
            buffer_frame[dest + 1] = buffer_frame[source + 1];
            buffer_frame[dest + 2] = buffer_frame[source + 2];
        }    
    }

    // fill left over pixels with white pixels
    for (int row = 0; row < height; ++row)
    {
        int rowPixel = row * width; 
        for (int column = width - offset; column < width; ++column)
        {
            int base = 3 * (rowPixel + column);
            buffer_frame[base] = 0xff;
            buffer_frame[base + 1] = 0xff;
            buffer_frame[base + 2] = 0xff;
        }
    }
    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
unsigned char *processRotateCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                               int rotate_iteration) {
    rotate_iteration = rotate_iteration % 4;
    if (rotate_iteration < 0)
        rotate_iteration += 4;

    if (rotate_iteration == 0)
        return buffer_frame;

    // allocate memory for temporary image buffer
    unsigned char *rendered_frame = allocateFrame(width, height);

    int widthBytes = 3 * width;

    // store shifted pixels to temporary buffer
    for (int iteration = 0; iteration < rotate_iteration; iteration++)
    {
        int columnByte = widthBytes - 3;

        for (int row = 0; row < width; row++)
        {
            int rowBytes = row * widthBytes;
            int rowByteOffset = columnByte;

            for (int column = 0; column < height; column++)
            {
                int position_frame_buffer = rowBytes + column * 3;

                rendered_frame[rowByteOffset] = buffer_frame[position_frame_buffer];
                rendered_frame[rowByteOffset + 1] = buffer_frame[position_frame_buffer + 1];
                rendered_frame[rowByteOffset + 2] = buffer_frame[position_frame_buffer + 2];

                rowByteOffset += widthBytes;
            }

            columnByte -= 3;
        }

        // copy the temporary buffer back to original frame buffer
        buffer_frame = copyFrame(rendered_frame, buffer_frame, width, height);
    }

    // free temporary image buffer
    deallocateFrame(rendered_frame);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorX(unsigned char *buffer_frame, unsigned int width, unsigned int height) {
    // allocate memory for temporary image buffer
    unsigned char *rendered_frame = allocateFrame(width, height);

    // store shifted pixels to temporary buffer
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            int position_rendered_frame = row * height * 3 + column * 3;
            int position_buffer_frame = (height - row - 1) * height * 3 + column * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, width, height);

    // free temporary image buffer
    deallocateFrame(rendered_frame);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorY(unsigned char *buffer_frame, unsigned width, unsigned height) {
    // allocate memory for temporary image buffer
    unsigned char *rendered_frame = allocateFrame(width, height);

    // store shifted pixels to temporary buffer
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            int position_rendered_frame = row * height * 3 + column * 3;
            int position_buffer_frame = row * height * 3 + (width - column - 1) * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, width, height);

    // free temporary image buffer
    deallocateFrame(rendered_frame);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "why-is-team-name-an-option-if-the-team-size-is-1";

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
void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
    int processed_frames = 0;
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
       // printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
              // sensor_values[sensorValueIdx].value);
        int value = sensor_values[sensorValueIdx].value;
        if (!strcmp(sensor_values[sensorValueIdx].key, "W")) 
        {
            if (value > 0)
                frame_buffer = processMoveUp(frame_buffer, width, height, value);
            else if (value < 0)
                frame_buffer = processMoveDown(frame_buffer, width, height, -1 * value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "A"))
        {
            if (value > 0) 
                frame_buffer = processMoveLeft(frame_buffer, width, height, value);
            else if (value < 0)
                frame_buffer = processMoveRight(frame_buffer, width, height, -1 * value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "S"))
        {
            if (value > 0)  
                frame_buffer = processMoveDown(frame_buffer, width, height, value);
            else
                frame_buffer = processMoveUp(frame_buffer, width, height, -1 * value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "D"))
        {
            if (value > 0)
                frame_buffer = processMoveRight(frame_buffer, width, height, value);
            else if (value < 0)
                frame_buffer = processMoveLeft(frame_buffer, width, height, -1 * value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "CW"))
        {
            frame_buffer = processRotateCW(frame_buffer, width, height, value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "CCW"))
        {
            frame_buffer = processRotateCW(frame_buffer, width, height, -1 * value);
        } 
        else if (!strcmp(sensor_values[sensorValueIdx].key, "MX"))
        {
            frame_buffer = processMirrorX(frame_buffer, width, height);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MY"))
        {
            frame_buffer = processMirrorY(frame_buffer, width, height);
        }
        
        // TODO
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // ULTRA IMPORTANT, CHANGE THIS VALUE BACK TO 25 BEFORE SUBMITTING
        // AS WELL AS IN THE IMPLENTATION REFERENCE
        processed_frames += 1;
        if (processed_frames % 1 == 0) {
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    return;
}
