#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "filters.h"
#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(_WIN32)
#include <windows.h>
#include <tchar.h>
#endif

void sobel_filter_cpu(const unsigned char *input, unsigned char *output, int width, int height) {
    int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    int pixel = input[(y + j) * width + (x + i)];
                    sumX += gx[j + 1][i + 1] * pixel;
                    sumY += gy[j + 1][i + 1] * pixel;
                }
            }
            int magnitude = (int)sqrt(sumX * sumX + sumY * sumY);
            if (magnitude > 255) magnitude = 255;
            int out_idx = (y * width + x) * 3;
            output[out_idx] = magnitude;
            output[out_idx + 1] = magnitude;
            output[out_idx + 2] = magnitude;
        }
    }
}

void gauss_filter_cpu(const unsigned char *input, unsigned char *output, int width, int height) {
    const int kernel[5][5] = {
        {1, 4, 6, 4, 1},
        {4, 16, 24, 16, 4},
        {6, 24, 36, 24, 6},
        {4, 16, 24, 16, 4},
        {1, 4, 6, 4, 1}
    };
    const int norm = 256;

    for (int y = 2; y < height - 2; y++) {
        for (int x = 2; x < width - 2; x++) {
            int sumR = 0, sumG = 0, sumB = 0;
            for (int j = -2; j <= 2; j++) {
                for (int i = -2; i <= 2; i++) {
                    int idx = ((y + j) * width + (x + i)) * 3;
                    int w = kernel[j + 2][i + 2];
                    sumR += input[idx] * w;
                    sumG += input[idx + 1] * w;
                    sumB += input[idx + 2] * w;
                }
            }
            int out_idx = (y * width + x) * 3;
            output[out_idx] = (unsigned char)(sumR / norm);
            output[out_idx + 1] = (unsigned char)(sumG / norm);
            output[out_idx + 2] = (unsigned char)(sumB / norm);
        }
    }
}

void median_filter_cpu(const unsigned char *input, unsigned char *output, int width, int height) {
    for (int y = 2; y < height - 2; y++) {
        for (int x = 2; x < width - 2; x++) {
            unsigned char R[25], G[25], B[25];
            int k = 0;
            for (int j = -2; j <= 2; j++) {
                for (int i = -2; i <= 2; i++) {
                    int idx = ((y + j) * width + (x + i)) * 3;
                    R[k] = input[idx];
                    G[k] = input[idx + 1];
                    B[k] = input[idx + 2];
                    k++;
                }
            }
            for (int i = 0; i < 24; i++) {
                for (int j = 0; j < 24 - i; j++) {
                    if (R[j] > R[j + 1]) { unsigned char t = R[j]; R[j] = R[j + 1]; R[j + 1] = t; }
                    if (G[j] > G[j + 1]) { unsigned char t = G[j]; G[j] = G[j + 1]; G[j + 1] = t; }
                    if (B[j] > B[j + 1]) { unsigned char t = B[j]; B[j] = B[j + 1]; B[j + 1] = t; }
                }
            }
            int idx = (y * width + x) * 3;
            output[idx] = R[12];
            output[idx + 1] = G[12];
            output[idx + 2] = B[12];
        }
    }
}

void luma_filter_cpu(const unsigned char *input, unsigned char *output, int width, int height) {
    for (int y = 2; y < height - 2; y++) {
        for (int x = 2; x < width - 2; x++) {
            float luma[25];
            unsigned char R[25], G[25], B[25];
            int k = 0;
            for (int j = -2; j <= 2; j++) {
                for (int i = -2; i <= 2; i++) {
                    int idx = ((y + j) * width + (x + i)) * 3;
                    R[k] = input[idx];
                    G[k] = input[idx + 1];
                    B[k] = input[idx + 2];
                    luma[k] = 0.299f * R[k] + 0.587f * G[k] + 0.114f * B[k];
                    k++;
                }
            }
            for (int i = 0; i < 24; i++) {
                for (int j = 0; j < 24 - i; j++) {
                    if (luma[j] > luma[j + 1]) {
                        float tl = luma[j]; luma[j] = luma[j + 1]; luma[j + 1] = tl;
                        unsigned char tr = R[j]; R[j] = R[j + 1]; R[j + 1] = tr;
                        unsigned char tg = G[j]; G[j] = G[j + 1]; G[j + 1] = tg;
                        unsigned char tb = B[j]; B[j] = B[j + 1]; B[j + 1] = tb;
                    }
                }
            }
            int idx = (y * width + x) * 3;
            output[idx] = R[12];
            output[idx + 1] = G[12];
            output[idx + 2] = B[12];
        }
    }
}

void run_filter_cpu(const char *filter, const unsigned char *input, unsigned char *output, int width, int height, int channels, const char *input_file) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (strcmp(filter, "sobel") == 0 && channels == 1) {
        sobel_filter_cpu(input, output, width, height);
    } else if (strcmp(filter, "gauss") == 0 && channels == 3) {
        gauss_filter_cpu(input, output, width, height);
    } else if (strcmp(filter, "median") == 0 && channels == 3) {
        median_filter_cpu(input, output, width, height);
    } else if (strcmp(filter, "luma") == 0 && channels == 3) {
        luma_filter_cpu(input, output, width, height);
    } else {
        fprintf(stderr, "Unsupported filter or channel count for CPU run: %s (%d channels)\n", filter, channels);
        return;
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    char cpu_name[256] = "cpu_seq";
#if defined(__APPLE__)
    size_t size = sizeof(cpu_name);
    if (sysctlbyname("machdep.cpu.brand_string", &cpu_name, &size, NULL, 0) != 0) {
        strncpy(cpu_name, "cpu_unknown", sizeof(cpu_name));
        cpu_name[sizeof(cpu_name) - 1] = '\0';
    }
#elif defined(__linux__)
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    strncpy(cpu_name, colon + 2, sizeof(cpu_name)); // skip ": "
                    cpu_name[sizeof(cpu_name) - 1] = '\0';
                    cpu_name[strcspn(cpu_name, "\n")] = 0; // remove newline
                }
                break;
            }
        }
        fclose(cpuinfo);
    } else {
        strncpy(cpu_name, "cpu_unknown", sizeof(cpu_name));
        cpu_name[sizeof(cpu_name) - 1] = '\0';
    }
#elif defined(_WIN32)
    HKEY hKey;
    LONG lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) {
        DWORD size = sizeof(cpu_name);
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpu_name, &size);
        RegCloseKey(hKey);
    } else {
        strncpy(cpu_name, "cpu_unknown", sizeof(cpu_name));
        cpu_name[sizeof(cpu_name) - 1] = '\0';
    }
#endif

#ifdef __linux__
#define delimeter '/'
#elif __APPLE__
#define delimeter '/'
#else
#define delimeter '\\'
#endif

    const char *filename_only = strrchr(input_file, delimeter);
    if (filename_only)
    {
        filename_only++;
    }
    else
    {
        filename_only = input_file;
    }

		char sequentialCpuName[512];
    snprintf(sequentialCpuName, sizeof(sequentialCpuName), "%s (sequential)", cpu_name);

    FILE *log = fopen("measurements.csv", "a");
    if (log) {
        fprintf(log, "%s,%s,%s,%d,%d,%.2f\n", filename_only, filter, sequentialCpuName, width, height, elapsed);
        fclose(log);
    } else {
        fprintf(stderr, "Failed to open measurements.csv for writing\n");
    }
}
