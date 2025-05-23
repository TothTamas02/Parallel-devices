__kernel void sobel(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    const int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const int gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    if (x >= 1 && y >= 1 && x < width - 1 && y < height - 1) {
        int sumX = 0, sumY = 0;

        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                int pixel = input[(y + j) * width + (x + i)];
                sumX += pixel * gx[j + 1][i + 1];
                sumY += pixel * gy[j + 1][i + 1];
            }
        }

        int magnitude = clamp((int)sqrt((float)(sumX * sumX + sumY * sumY)), 0, 255);
        output[y * width + x] = (uchar)magnitude;
    }
}

#define RADIUS 2
#define KSIZE (2 * RADIUS + 1)

__constant int gaussian_kernel[KSIZE * KSIZE] = {
     1,  4,  6,  4, 1,
     4, 16, 24, 16, 4,
     6, 24, 36, 24, 6,
     4, 16, 24, 16, 4,
     1,  4,  6,  4, 1
};

#define NORMALIZATION_FACTOR 256

__kernel void gauss(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= RADIUS && y >= RADIUS && x < width - RADIUS && y < height - RADIUS) {
        int sumR = 0, sumG = 0, sumB = 0;

        for (int ky = -RADIUS; ky <= RADIUS; ky++) {
            for (int kx = -RADIUS; kx <= RADIUS; kx++) {
                int idx = ((y + ky) * width + (x + kx)) * 3;
                int w = gaussian_kernel[(ky + RADIUS) * KSIZE + (kx + RADIUS)];
                sumR += input[idx] * w;
                sumG += input[idx + 1] * w;
                sumB += input[idx + 2] * w;
            }
        }

        int out_idx = (y * width + x) * 3;
        output[out_idx] = (uchar)(sumR / NORMALIZATION_FACTOR);
        output[out_idx + 1] = (uchar)(sumG / NORMALIZATION_FACTOR);
        output[out_idx + 2] = (uchar)(sumB / NORMALIZATION_FACTOR);
    }
}

__kernel void median(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= 2 && y >= 2 && x < width - 2 && y < height - 2) {
        uchar R[25], G[25], B[25];
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
                if (R[j] > R[j + 1]) {
                    uchar temp = R[j];
                    R[j] = R[j + 1];
                    R[j + 1] = temp;
                }
                if (G[j] > G[j + 1]) {
                    uchar temp = G[j];
                    G[j] = G[j + 1];
                    G[j + 1] = temp;
                }
                if (B[j] > B[j + 1]) {
                    uchar temp = B[j];
                    B[j] = B[j + 1];
                    B[j + 1] = temp;
                }
            }
        }
        int idx = (y * width + x) * 3;
        output[idx] = R[12];
        output[idx + 1] = G[12];
        output[idx + 2] = B[12];
    }
}

__kernel void luma(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= 2 && y >= 2 && x < width - 2 && y < height - 2) {
        float luma[25];
        uchar R[25], G[25], B[25];
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

        // Bubble sort by luma
        for (int i = 0; i < 24; i++) {
            for (int j = 0; j < 24 - i; j++) {
                if (luma[j] > luma[j + 1]) {
                    float temp_l = luma[j];
                    luma[j] = luma[j + 1];
                    luma[j + 1] = temp_l;

                    uchar temp_r = R[j];
                    R[j] = R[j + 1];
                    R[j + 1] = temp_r;

                    uchar temp_g = G[j];
                    G[j] = G[j + 1];
                    G[j + 1] = temp_g;

                    uchar temp_b = B[j];
                    B[j] = B[j + 1];
                    B[j + 1] = temp_b;
                }
            }
        }

        int idx = (y * width + x) * 3;
        output[idx] = R[12];
        output[idx + 1] = G[12];
        output[idx + 2] = B[12];
    }
}
