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
    zz_p_pow
)
import PRF.aes_prf as prf

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
    D = []
    d_0 = []
    d_1 = []
    for digit in d:
        D.append(Paillier.Enc(digit, N))
        digit1 = zz_random_smaller_than_n(random_range)
        digit0 = zz_sub(digit1, digit)
        d_0.append(digit0)
        d_1.append(digit1)
    ek_0 = (k_prf, *d_0)
    ek_1 = (k_prf, *d_1)
    pk = (N, D)
    return (pk, ek_0, ek_1)

def Input(pk, x):
    N, D = pk
    X = Paillier.Enc(x, N)
    #I = [X]
    I = []
    for digit in D:
        I.append(Paillier.Enc(x, N, digit))
    return (I, I)

def Load(b, pk, ek, I, id):
    # zz_p_init(state['N'])
    # two_to_kappa = state['2^kappa']
    # secret_share_1 = zz_p_add(zz_mod(str(int.from_bytes(prf.apply(int(ek[0]).to_bytes(16, 'big'), b'1'), 'big')), two_to_kappa), b)
    # memory_value_1 = (secret_share_1, *ek[1:])
    memory_value_1 = ek[1:]
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
    # yd = calculate_yd(m[1:])
    yd = calculate_yd(m)
    zd = []
    N = state['N']
    powers = calc_powers(i, yd)
    zz_p_init(N)
    prf_key = int(ek[0]).to_bytes(16, 'big')
    j = 0
    for power in powers:
        zd.append(zz_add(DDLog(power, N), str(int.from_bytes(prf.apply(prf_key, zz_add(id, str(j)).encode("utf-8")), 'big'))))
        j += 1
    return zd

def calc_powers(bases, exponent):
    zz_p_init(state['N^2'])
    return [zz_p_pow(base, exponent) for base in bases]

def Output(b, ek, m, n_out, id):
    return zz_mod(m[0], n_out)

def DDLog(g, N):
    h = zz_mod(g, N)
    h_tag = zz_div(zz_sub(g, h), N)
    return zz_p_mul(h_tag, zz_p_inv(h))

def calculate_yd(m):
    Bsk = state['Bsk']
    yd = "0"
    digit_value = "1"
    for digit in m:
        yd = zz_add(yd, zz_mul(digit_value, digit))
        digit_value = zz_mul(digit_value, Bsk)
    return yd
