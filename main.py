from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import ZZ_p_interface as zzp
from NTL_interfaces import ZZ_interface as zz
from NTL_interfaces import vec_mat_ZZ_interface as zz_vm
import time
import gc
import PRF.aes_prf as prf

def main():
    
    k = int(zz.zz_random(128)).to_bytes(16, 'big')
    # print(type(k))
    start = time.time()
    for i in range(50000):
        a = prf.apply(k, i.to_bytes(16, 'big'))
    end = time.time()
    print("Time taken: ", end - start)

if __name__ == "__main__":
    main()