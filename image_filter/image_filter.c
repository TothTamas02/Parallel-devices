#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

void print_usage()
{
	printf("Usage: ./image_filter --input <input_file.jpg|png|...> --output <output_file.png> --filter sobel|gauss|median|luma --platform <platform_id>\n");
}

int main(int argc, char **argv)
{
	char *input_file = NULL;
	char *output_file = NULL;
	char *filter = NULL;
	int platform_id = -1;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--input") == 0 && i + 1 < argc)
		{
			input_file = argv[++i];
		}
		else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
		{
			output_file = argv[++i];
		}
		else if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc)
		{
			filter = argv[++i];
		}
		else if (strcmp(argv[i], "--platform") == 0 && i + 1 < argc)
		{
			platform_id = atoi(argv[++i]);
		}
	}

	if (!input_file || !output_file || !filter || platform_id == -1)
	{
		print_usage();
		return 1;
	}

	int width, height, channels;
	unsigned char *input_image = stbi_load(input_file, &width, &height, &channels, 0);
	if (!input_image)
	{
		fprintf(stderr, "Error loading image %s\n", input_file);
		return 1;
	}

	printf("Loaded image %s with dimensions %dx%d and %d channels\n", input_file, width, height, channels);

	unsigned char *output_image = (unsigned char *)malloc(width * height * channels);
	if (!output_image)
	{
		fprintf(stderr, "Error allocating memory for output image\n");
		stbi_image_free(input_image);
		return 1;
	}

	if (platform_id == -2)
	{
		// CPU-only mode
		if (strcmp(filter, "sobel") == 0 && channels == 3)
		{
			unsigned char *gray_image = (unsigned char *)malloc(width * height);
			if (!gray_image)
			{
				fprintf(stderr, "Error allocating memory for grayscale image\n");
				stbi_image_free(input_image);
				free(output_image);
				return 1;
			}
			for (int i = 0; i < width * height; i++)
			{
				int r = input_image[i * 3];
				int g = input_image[i * 3 + 1];
				int b = input_image[i * 3 + 2];
				gray_image[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
			}
			run_filter_cpu(filter, gray_image, output_image, width, height, 1, input_file);
			free(gray_image);
		}
		else
		{
			run_filter_cpu(filter, input_image, output_image, width, height, channels, input_file);
		}
	}
	else
	{
		run_filter(input_image, output_image, width, height, channels, filter, platform_id, input_file);
	}

	if (!stbi_write_png(output_file, width, height, 3, output_image, width * 3))
	{
		fprintf(stderr, "Error writing image %s\n", output_file);
	}
	else
	{
		printf("Saved filtered image to %s\n", output_file);
	}

	stbi_image_free(input_image);
	free(output_image);
	return 0;
}
