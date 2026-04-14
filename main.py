from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import vec_mat_ZZ_interface as zz_vm
import time
import gc
import PRF.aes_prf as prf

def main():
    
    zz_vm.benchmark_ntl_setup(512, 51200, 100, 50, 10)

if __name__ == "__main__":
    main()