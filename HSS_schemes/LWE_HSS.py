import Encryptions.LWE as LWE
from NTL_interfaces.vec_mat_ZZ_interface import (
    vec_random,
    vec_sub,
    OKDM,
    DDEC,
    mat_add,
    mat_add_scalar,
    vec_add_scalar,
    vec_add
)
from NTL_interfaces.ZZ_interface import (
    zz_random,
    zz_add
)
import PRF.aes_prf as prf
import NTL_interfaces.vec_mat_ZZ_interface as zz_vm

PATH_TO_PARAMETERS = "../hss_parameters/lwe_parameters.txt"
state = {}

def read_parameters(n, Bmax):
    with open(PATH_TO_PARAMETERS, 'r') as f:
        for line in f:
            if line.startswith('rop'):
                continue
            params = line.strip().split(',')
            if int(params[0]) >= n and int(params[5]) >= Bmax:
                state['lambda'] = n
                state['Bmax'] = Bmax
                state['q_len'] = int(params[2])
                state['p_len'] = int(params[3])
                state['n'] = int(params[1])
                state['m'] = state['n'] * state['q_len']
                return
    raise ValueError(f"Parameters for lambda={n} and Bmax={Bmax} not found in {PATH_TO_PARAMETERS}")

def benchmark_Setup(n, Bmax=1, iterations=1000):
    read_parameters(n, Bmax)
    zz_vm.benchmark_ntl_setup(state['lambda'], state['n'], state['m'], state['q_len'], state['p_len'], iterations)

def benchmark_input(iterations=1000):
    zz_vm.run_benchmark_OKDM(state['n'], state['m'], state['q_len'], state['p_len'], iterations)
    
def benchmark_load_or_mul(iterations=1000):
    zz_vm.run_benchmark_DDEC(state['n'], state['m'], state['q_len'], state['p_len'], iterations)
    
def benchmark_last_mul(iterations=1000):
    zz_vm.run_benchmark_last_mul(state['n'], state['m'], state['q_len'], state['p_len'], iterations)

def benchmark_last_mem_add(iterations=1000):
    b = 0
    zz_vm.run_benchmark_last_mem_add(b, state['n'], state['q_len'], state['p_len'], iterations)
