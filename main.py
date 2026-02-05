from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import vec_mat_ZZ_p_interface as zzp_vm

def main():
    print("--- 1. Setup ---")
    # Define a large prime modulus q (e.g., a 60-bit prime)
    q_str = zz.zz_random_prime(10, 100)
    print(f"Modulus q: {q_str}")
    
    # Initialize the global context for ZZ_p
    zzp.zz_p_init(q_str)

    print("\n--- 2. Vector Operations ---")
    # Generate two random vectors of length 3
    v1_str = zzp_vm.vec_random(3)
    v2_str = zzp_vm.vec_random(3)
    
    print(f"v1: {v1_str}")
    print(f"v2: {v2_str}")
    
    # Vector Addition: v3 = v1 + v2
    v3_str = zzp_vm.vec_add(v1_str, v2_str)
    print(f"v1 + v2: {v3_str}")
    
    # Inner Product: dot = <v1, v2>
    dot_str = zzp_vm.vec_inner_product(v1_str, v2_str)
    print(f"<v1, v2>: {dot_str}")
    
    # Scalar Multiplication: v4 = v1 * 5
    scalar = "5"
    v4_str = zzp_vm.vec_mul_scalar(v1_str, scalar)
    print(f"v1 * 5: {v4_str}")

    print("\n--- 3. Matrix Operations ---")
    # Generate a random 3x3 matrix A
    A_str = zzp_vm.mat_random(3, 3)
    # Generate a random 3x3 matrix B
    B_str = zzp_vm.mat_random(3, 3)
    
    print(f"Matrix A:\n{A_str}")
    print(f"Matrix B:\n{B_str}")
    
    # Matrix Addition: C = A + B
    C_str = zzp_vm.mat_add(A_str, B_str)
    print(f"A + B:\n{C_str}")
    
    # Matrix Multiplication: D = A * B
    D_str = zzp_vm.mat_mul(A_str, B_str)
    print(f"A * B:\n{D_str}")
    
    # Matrix-Vector Multiplication: u = A * v1
    # Note: v1 is treated as a column vector
    u_str = zzp_vm.mat_mul_vec(A_str, v1_str)
    print(f"A * v1:\n{u_str}")

    print("\n--- 4. Determinant & Inverse ---")
    # Calculate determinant of A
    det_str = zzp_vm.mat_det(A_str)
    print(f"det(A): {det_str}")
    
    # Calculate inverse of A (if det != 0)
    # Note: For random large matrices mod q, it's almost certainly invertible
    if det_str != "0":
        inv_A_str = zzp_vm.mat_inv(A_str)
        print(f"inv(A):\n{inv_A_str}")
        
        # Verify: A * inv(A) should be Identity
        id_check = zzp_vm.mat_mul(A_str, inv_A_str)
        print(f"A * inv(A) [Should be Identity]:\n{id_check}")
    else:
        print("Matrix A is singular, cannot invert.")

if __name__ == "__main__":
    main()