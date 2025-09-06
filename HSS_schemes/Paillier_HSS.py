import Encryptions.Paillier as Paillier
from NTL_interfaces.ZZ_interface import (
    zz_add,
    zz_mod,
    zz_div,
    zz_pow,
    zz_mul,
    zz_random_smaller_than_n, 
    zz_sub,
    zz_random
)
from NTL_interfaces.ZZ_p_interface import (
    zz_p_add,
    zz_p_init,
    zz_p_mul,
    zz_p_inv,
    batch_zz_p_pow
)
import PRF.aes_prf as prf
import multiprocessing as mp

state = {}

def Setup(n):
    N, sk = Paillier.Gen(n)
    state['N'] = N
    state['N^2'] = zz_pow(N, "2")
    n = str(len(n))
    kappa = "80" # error happens with probabilty 2^-kappa
    state['kappa'] = kappa
    two_to_kappa = zz_pow("2", kappa)
    Bsk = zz_div(sk, zz_pow("2", zz_add(n, zz_div(zz_mul(n, "127"), "128"))))
    Bmsg = zz_div(Bsk, two_to_kappa)
    state["2^kappa"] = two_to_kappa
    state['Bmsg'] = Bmsg
    state['Bsk'] = Bsk
    d = []
    while(sk != "0"):
        d.append(zz_mod(sk, Bsk))
        sk = zz_div(sk, Bsk)
    random_range = zz_mul(Bmsg, Bsk)
    k_prf = zz_random(128)
    d_0, d_1, D = work_for_Setup(d, N, random_range)
    ek_0 = (k_prf, *d_0)
    ek_1 = (k_prf, *d_1)
    pk = (N, D)
    return (pk, ek_0, ek_1)

def work_for_Setup(d, N, random_range):
    queue = mp.Queue()
    def worker(digit, N, queue, i):
        digit1 = zz_random_smaller_than_n(random_range)
        digit0 = zz_sub(digit1, digit)
        result = Paillier.Enc(digit, N)
        queue.put((digit0, digit1, result, i))
    processes = []
    i = 0
    for digit in d:
        p = mp.Process(target=worker, args=(digit, N, queue, i))
        processes.append(p)
        p.start()
        i += 1
    for p in processes:
        p.join()
    results = [None] * len(d)
    d_0 = [None] * len(d)
    d_1 = [None] * len(d)
    while not queue.empty():
        digit0, digit1, result, i = queue.get()
        results[i] = result
        d_0[i] = digit0
        d_1[i] = digit1
    return d_0, d_1, results

def Input(pk, x):
    N, D = pk
    X = Paillier.Enc(x, N)
    I = [X]
    I.extend(encrypt_for_Input(x, N, D))
    return (I, I)

def encrypt_for_Input(x, N, D):
    queue = mp.Queue()
    def worker(x, N, d, queue, i):
        result = Paillier.Enc(x, N, d)
        queue.put((result, i))
    processes = []
    i = 0
    for d in D:
        p = mp.Process(target=worker, args=(x, N, d, queue, i))
        processes.append(p)
        p.start()
        i += 1
    for p in processes:
        p.join()
    results = [None] * len(D)
    while not queue.empty():
        result, i = queue.get()
        results[i] = result
    return results

def Load(b, pk, ek, I, id):
    zz_p_init(state['N'])
    two_to_kappa = state['2^kappa']
    secret_share_1 = zz_p_add(zz_mod(str(int.from_bytes(prf.apply(int(ek[0]).to_bytes(16, 'big'), b'1'), 'big')), two_to_kappa), b)
    memory_value_1 = (secret_share_1, *ek[1:])
    return Mul(b, ek, I, memory_value_1, id)

def Add_Inputs(b, ek, i1, i2, id):
    zz_p_init(state['N^2'])
    Z = []
    for j in range(len(i1)):
        i1_j = i1[j]
        i2_j = i2[j]
        Z.append(zz_p_mul(i1_j, i2_j))
    return Z

def Add_Memory_Values(b, ek, m1, m2, id):
    Z = []
    for j in range(len(m1)):
        m1_j = m1[j]
        m2_j = m2[j]
        Z.append(zz_add(m1_j, m2_j))
    return Z

def Mul(b, ek, i, m, id):
    Bsk = state['Bsk']
    yd = calculate_yd(m[1:], Bsk)
    zd = []
    N = state['N']
    N_squared = state['N^2']
    powers = []
    zz_p_init(N_squared)
    powers = batch_zz_p_pow(i, yd)
    zz_p_init(N)
    zd = calc_for_Mul(powers, int(ek[0]).to_bytes(16, 'big'), N, id)
    return zd

def calc_for_Mul(powers, prf_key, N, id):
    queue = mp.Queue()
    def worker(power, queue, i):
        result = zz_add(DDLog(power, N), str(int.from_bytes(prf.apply(prf_key, zz_add(id, str(i)).encode("utf-8")), 'big')))
        queue.put((result, i))
    processes = []
    for i, power in enumerate(powers):
        p = mp.Process(target=worker, args=(power, queue, i))
        processes.append(p)
        p.start()
    for p in processes:
        p.join()
    results = [None] * len(powers)
    while not queue.empty():
        result, i = queue.get()
        results[i] = result
    return results

def Output(b, ek, m, n_out, id):
    return zz_mod(m[0], n_out)

def DDLog(g, N):
    h = zz_mod(g, N)
    h_tag = zz_div(zz_sub(g, h), N)
    return zz_p_mul(h_tag, zz_p_inv(h))

def calculate_yd(m, Bsk):
    yd = "0"
    digit_value = "1"
    for digit in m:
        yd = zz_add(yd, zz_mul(digit_value, digit))
        digit_value = zz_mul(digit_value, Bsk)
    return yd
