#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

void print_usage() {
	printf("Usage: ./image_filter --input <input_file.jpg|png|...> --output <output_file.png> --filter sobel|gauss|median|luma --device cpu|gpu\n");
}

int main(int argc, char** argv) {
	char *input_file = NULL;
	char *output_file = NULL;
	char *filter = NULL;
	char *device = NULL;

	for(int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
			input_file = argv[++i];
		} else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
			output_file = argv[++i];
		} else if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc) {
			filter = argv[++i];
		} else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
			device = argv[++i];
		}
	}

	if (!input_file || !output_file || !filter || !device) {
		print_usage();
		return 1;
	}

	int width, height, channels;
	unsigned char *input_image = stbi_load(input_file, &width, &height, &channels, 0);
	if (!input_image) {
		fprintf(stderr, "Error loading image %s\n", input_file);
		return 1;
	}

	printf("Loaded image %s with dimensions %dx%d and %d channels\n", input_file, width, height, channels);

	unsigned char *output_image = (unsigned char *)malloc(width * height * channels);
	if (!output_image) {
		fprintf(stderr, "Error allocating memory for output image\n");
		stbi_image_free(input_image);
		return 1;
	}

	run_filter(input_image, output_image, width, height, channels, filter, device, argv[2]);

	if (!stbi_write_png(output_file, width, height, 3, output_image, width * 3)) {
		fprintf(stderr, "Error writing image %s\n", output_file);
	} else {
		printf("Saved filtered image to %s\n", output_file);
	}

	stbi_image_free(input_image);
	free(output_image);
	return 0;
}
