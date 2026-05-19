from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import vec_mat_ZZ_interface as zz_vm
import time
import gc
import PRF.aes_prf as prf

def main():
    
    # Cryptographic parameters
    N = 100
    M = 2000

    # Moduli must be passed as raw byte strings (b"...")
    # Using a dummy massive integer to simulate the 276-bit prime q
    Q_STR = b"123456789101112131415161718192021222324252627282930" 
    P_STR = b"65537"

    # Iteration counts
    OKDM_ITERATIONS = 0   # Keep this low, it allocates ~3.6 GB per run
    DDEC_ITERATIONS = 50  # Very fast matrix math, safe to run many times
    
    print("\n" + "="*50)
    print("   HOMOMORPHIC SECRET SHARING (HSS) BENCHMARKS")
    print("="*50 + "\n")

    print(f"Parameters: N={N}, M={M}")
    print("-" * 50)

    # Run OKDM Benchmark
    print("\n>>> Launching OKDM Generation Benchmark...")
    zz_vm.run_benchmark_OKDM(N, M, Q_STR, OKDM_ITERATIONS)

    print("-" * 50)

    # Run DDEC Benchmark
    print("\n>>> Launching DDEC Server Evaluation Benchmark...")
    zz_vm.run_benchmark_DDEC(N, M, Q_STR, P_STR, DDEC_ITERATIONS)

    print("\nAll benchmarks completed successfully.")
if __name__ == "__main__":
    main()