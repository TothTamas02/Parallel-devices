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

__kernel void gauss(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    const int k[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };

    if (x >= 1 && y >= 1 && x < width - 1 && y < height - 1) {
        int sumR = 0;
        int sumG = 0;
        int sumB = 0;
        for (int ky = -1; ky <= 1; ky++) {
            for (int kx = -1; kx <= 1; kx++) {
                int idx = ((y + ky) * width + (x + kx)) * 3;
                int w = k[ky + 1][kx + 1];
                sumR += input[idx] * w;
                sumG += input[idx + 1] * w;
                sumB += input[idx + 2] * w;
            }
        }
        int idx = (y * width + x) * 3;
        output[idx] = (uchar)(sumR / 16);
        output[idx + 1] = (uchar)(sumG / 16);
        output[idx + 2] = (uchar)(sumB / 16);
    }
}

__kernel void median(__global uchar *input, __global uchar *output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= 1 && y >= 1 && x < width - 1 && y < height - 1) {
        uchar R[9], G[9], B[9];
        int k = 0;
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                int idx = ((y + j) * width + (x + i)) * 3;
                R[k] = input[idx];
                G[k] = input[idx + 1];
                B[k] = input[idx + 2];
                k++;
            }
        }
        for (int i = 0; i < 9-1; i++) {
            for (int j = 0; j < 9-i-1; j++) {
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
        output[idx] = R[4];
        output[idx + 1] = G[4];
        output[idx + 2] = B[4];
    }
}
