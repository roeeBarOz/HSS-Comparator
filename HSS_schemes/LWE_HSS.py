from cmath import log
from math import ceil

import NTL_interfaces.vec_mat_ZZ_interface as zz_vm

PATH_TO_PARAMETERS = "/home/roee/HSS-Comparator/hss_parameters/lwe_parameters.txt"
state = {}

def read_parameters(n, Bmax, P_size):
    log_n = ceil(log(n, 2).real)
    p_length = P_size + log_n + Bmax + 42
    q_length = 32
    with open(PATH_TO_PARAMETERS, 'r') as f:
        for line in f:
            if line.startswith('rop'):
                continue
            params = line.strip().split(',')
            if float(params[0]) >= n and float(params[2]) >= q_length:
                state['lambda'] = n
                state['Bmax'] = Bmax
                state['q_len'] = int(params[2])
                state['p_len'] = int(params[3])
                state['n'] = int(params[1])
                state['m'] = state['n'] * state['q_len']
                return state
    raise ValueError(f"Parameters for lambda={n} and Bmax={Bmax} not found in {PATH_TO_PARAMETERS}")

def benchmark_Setup(n, Bmax=1, P_size=5, iterations=1000):
    read_parameters(n, Bmax, P_size)
    return zz_vm.benchmark_ntl_setup(state['n'], state['m'], state['q_len'], state['p_len'], iterations)

def benchmark_input(iterations=1000):
    return zz_vm.run_benchmark_OKDM(state['n'], state['m'], state['q_len'], state['p_len'], iterations)
    
def benchmark_load_or_mul(iterations=1000):
    return zz_vm.run_benchmark_DDEC(state['n'], state['m'], state['q_len'], state['p_len'], iterations)
    
def benchmark_last_mul(iterations=1000):
    return zz_vm.run_benchmark_last_mul(state['n'], state['m'], state['q_len'], state['p_len'], iterations)

def benchmark_last_mem_add(iterations=1000):
    b = 0
    return zz_vm.run_benchmark_last_mem_add(b, state['n'], state['q_len'], state['p_len'], iterations)

def benchmark_mem_add(iterations=1000):
    b = 0
    return zz_vm.benchmark_mem_add(b, state['n'], state['m'], state['q_len'], state['p_len'], state['Bmax'], iterations)
