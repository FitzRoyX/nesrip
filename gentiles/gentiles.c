#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"   
#include <stdint.h>

//#include "lodepng.h" // Ensure you have lodepng.h and lodepng.c in your project

#define REPEAT_COUNT 16
#define TILE_SIZE 8
#define IMAGE_WIDTH (TILE_SIZE * REPEAT_COUNT)
#define IMAGE_HEIGHT TILE_SIZE

void generateTile(uint8_t* image, uint8_t color1[4], uint8_t color2[4]) {
    for (int y = 0; y < TILE_SIZE; y++) {
        for (int x = 0; x < TILE_SIZE; x++) {
            int index = 4 * (y * TILE_SIZE + x);
            if (x == y || x == TILE_SIZE - y - 1) {
                // Set color for the "X" pattern
                image[index + 0] = color1[0]; // Red
                image[index + 1] = color1[1]; // Green
                image[index + 2] = color1[2]; // Blue
                image[index + 3] = color1[3]; // Alpha
            }
            else {
                // Set background color
                image[index + 0] = color2[0];
                image[index + 1] = color2[1];
                image[index + 2] = color2[2];
                image[index + 3] = color2[3];
            }
        }
    }
}

void generate_image(uint8_t *image) {
    uint8_t tile[TILE_SIZE * TILE_SIZE * 4]; // RGBA format

    // Define two colors: Red for "X" and White for background
    uint8_t color1[4] = { 255, 0, 0, 255 };   // Red
    uint8_t color2[4] = { 255, 255, 255, 255 }; // White

    // Generate the tile
    generateTile(tile, color1, color2);

    for (int y = 0; y < TILE_SIZE; y++) {
        for (int repeat = 0; repeat < REPEAT_COUNT; repeat++) {
            for (int x = 0; x < TILE_SIZE; x++) {
                int src_idx = (y * TILE_SIZE + x) * 4;
                int dest_idx = (y * IMAGE_WIDTH + repeat * TILE_SIZE + x) * 4;

                image[dest_idx] = tile[src_idx];
                image[dest_idx + 1] = tile[src_idx + 1];
                image[dest_idx + 2] = tile[src_idx + 2];
                image[dest_idx + 3] = tile[src_idx + 3];
            }
        }
    }
}

int main() {
    uint8_t image[IMAGE_WIDTH * IMAGE_HEIGHT * 4];
    generate_image(image);

    // Save the tile as a PNG file
	if (!stbi_write_png("tile_pattern.png", IMAGE_WIDTH, IMAGE_HEIGHT, 4, image, IMAGE_WIDTH * 4)) {
        printf("Failed to write image\n");
        return 1;
	}
    //unsigned error = lodepng_encode32_file("tile_pattern.png", image, IMAGE_WIDTH, IMAGE_HEIGHT);
    //if (error) {
    //    printf("Error %u: %s\n", error, lodepng_error_text(error));
    //    return 1;
    //}

    printf("Tiles saved as tile_pattern.png\n");
    return 0;
}
