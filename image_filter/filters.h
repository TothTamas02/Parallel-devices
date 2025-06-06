#ifndef FILTERS_H
#define FILTERS_H

void run_filter(unsigned char *input, unsigned char *output, int width, int height, int channels, const char *filter, const int platform_id, const char *input_file);
void run_filter_cpu(const char *filter, const unsigned char *input, unsigned char *output, int width, int height, int channels, const char *input_file);

#endif
