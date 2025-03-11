__kernel void vector_add(
                         __global const float *A,
                          __global const float *B,
                          __global float *C,
                          const unsigned int N
                         )
{
int id = get_global_id(0);

if (id < N) {
C[id] = A[id] + B[id];
}
}
