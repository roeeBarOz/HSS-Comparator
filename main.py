from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import vec_mat_ZZ_interface as zz_vm
import time
import gc

def main():
    
    p = zz.zz_random_prime(553, 100)
    zz_vm.benchmark_ntl_mul(500, 100, str(p))

if __name__ == "__main__":
    main()