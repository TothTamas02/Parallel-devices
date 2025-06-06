__kernel void rank(__global const int *input, __global int *output, const int n) {
	int i = get_global_id(0);
	int current = input[i];
	int count = 0;
	for (int j = 0; j < n; ++j) {
		if (input[j] < current) {
			count++;
		}
	}
	output[i] = count;
}

