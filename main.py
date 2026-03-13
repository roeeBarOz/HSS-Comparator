from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import vec_mat_ZZ_p_interface as zzp_vm
import time

def main():
    print("--- 1. Setup ---")
    # Define a large prime modulus q (e.g., a 60-bit prime)
    q_str = zz.zz_random_prime(500, 100)
    
    # Initialize the global context for ZZ_p
    zzp.zz_p_init(q_str)
    print(f"Initialized ZZ_p with modulus q = {q_str}")

    print("\n--- 2. Vector Operations ---")
    # Generate two random vectors of length 3
    v1_str = zzp_vm.vec_random(1000)
    v2_str = zzp_vm.vec_random(1000)
    
    # Vector Addition: v3 = v1 + v2
    start = time.time()
    for _ in range(1000):
        zzp_vm.vec_add(v1_str, v2_str)
    end = time.time()
    print(f"Time for 1000 vector additions: {end - start:.4f} seconds")
    
    # Inner Product: dot = <v1, v2>
    start = time.time()
    for _ in range(1000):
        zzp_vm.vec_inner_product(v1_str, v2_str)
    end = time.time()
    dot_str = zzp_vm.vec_inner_product(v1_str, v2_str)
    print(f"Time for 1000 inner products: {end - start:.4f} seconds")
    
    # Scalar Multiplication: v4 = v1 * 5
    scalar = "5"
    start = time.time()
    for _ in range(1000):
        zzp_vm.vec_mul_scalar(v1_str, scalar)
    end = time.time()
    v4_str = zzp_vm.vec_mul_scalar(v1_str, scalar)
    print(f"Time for 1000 scalar multiplications: {end - start:.4f} seconds")

    print("\n--- 3. Matrix Operations ---")
    # Generate a random 3x3 matrix A
    A_str = zzp_vm.mat_random(1000, 1000)
    # Generate a random 3x3 matrix B
    B_str = zzp_vm.mat_random(1000, 1000)
    
    # Matrix Addition: C = A + B
    start = time.time()
    for _ in range(100):
        zzp_vm.mat_add(A_str, B_str)
    end = time.time()
    C_str = zzp_vm.mat_add(A_str, B_str)
    print(f"Time for 100 matrix additions: {end - start:.4f} seconds")
    
    # # Matrix Multiplication: D = A * B
    # start = time.time()
    # for _ in range(100):
    #     zzp_vm.mat_mul(A_str, B_str)
    # end = time.time()
    # D_str = zzp_vm.mat_mul(A_str, B_str)
    # print(f"Time for 100 matrix multiplications: {end - start:.4f} seconds")
    
    # Matrix-Vector Multiplication: u = A * v1
    # Note: v1 is treated as a column vector
    start = time.time()
    for _ in range(100):        zzp_vm.mat_mul_vec(A_str, v1_str)
    end = time.time()
    u_str = zzp_vm.mat_mul_vec(A_str, v1_str)
    print(f"Time for 100 matrix-vector multiplications: {end - start:.4f} seconds")

    print("\n--- 4. Determinant & Inverse ---")
    # Calculate determinant of A
    start = time.time()
    for _ in range(100):        zzp_vm.mat_transpose(A_str)
    end = time.time()
    print(f"Time for 100 matrix transpositions: {end - start:.4f} seconds")
    
    # # Calculate inverse of A (if det != 0)
    # # Note: For random large matrices mod q, it's almost certainly invertible
    # if det_str != "0":
    #     inv_A_str = zzp_vm.mat_inv(A_str)
    #     print(f"inv(A):\n{inv_A_str}")
        
    #     # Verify: A * inv(A) should be Identity
    #     id_check = zzp_vm.mat_mul(A_str, inv_A_str)
    #     print(f"A * inv(A) [Should be Identity]:\n{id_check}")
    # else:
    #     print("Matrix A is singular, cannot invert.")

if __name__ == "__main__":
    main()