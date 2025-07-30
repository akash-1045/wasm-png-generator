#include <stdlib.h>
#include <string.h>
#include <png.h>

// Struct to hold PNG data in memory
typedef struct {
    unsigned char *data;  // Pointer to PNG byte buffer
    size_t size;          // Current size of data written
    size_t capacity;      // Total allocated capacity
} PngBuffer;

// Custom write callback for libpng to write data into memory
void png_memory_write(png_structp png_ptr, png_bytep data, png_size_t length) {
    PngBuffer *p = (PngBuffer *)png_get_io_ptr(png_ptr);

    // Resize buffer if needed
    if (p->size + length > p->capacity) {
        size_t new_capacity = p->capacity * 2 + length;
        p->data = realloc(p->data, new_capacity);
        p->capacity = new_capacity;
    }

    // Copy new PNG chunk into buffer
    memcpy(p->data + p->size, data, length);
    p->size += length;
}


// generate PNG from raw RGB data
unsigned char* generate_png(uint8_t *rgb, int width, int height, int *out_size) {

    // Initialize libpng write structure
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) return NULL;

    // Create PNG info structure (metadata)
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return NULL;
    }

    // Set error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NULL;
    }

    // Set up memory buffer to collect PNG output
    PngBuffer buffer = {0};
    png_set_write_fn(png_ptr, &buffer, png_memory_write, NULL);

    // Set PNG header info (image dimensions, color type, etc.)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr); // Write header

    // Prepare row pointers to access RGB data line by line
    png_bytep *row_pointers = malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = rgb + y * width * 3;
    }

    png_write_image(png_ptr, row_pointers); // Write pixel data
    png_write_end(png_ptr, NULL);

    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr); // Cleanup libpng structs

    *out_size = buffer.size; // Return size of PNG buffer
    return buffer.data;     // Return pointer to PNG data in memory
}
