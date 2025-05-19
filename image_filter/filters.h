#ifndef FILTERS_H
#define FILTERS_H

void run_filter(unsigned char *input, unsigned char *output, int width, int height, int channels, const char *filter, const char *device_type_str, const char *input_file);

#endif
