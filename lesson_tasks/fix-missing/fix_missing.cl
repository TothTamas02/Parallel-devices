__kernel void fix_missing(
	__global const int* input,
	__global int* output,
	int n
) {
	int i = get_global_id(0);
	if (i >= n) return;

	if(input[i] == 0) {
		output[i] = (input[i-1] + input[i+1]) / 2;
	} else {
		output[i] = input[i];
	}
}
