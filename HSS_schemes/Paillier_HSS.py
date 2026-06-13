from math import log2

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

def Setup(security_parameter, Bmsg_length, num_of_mults):
    pk_paillier, sk = Paillier.Gen(security_parameter)
    N, g = pk_paillier
    state['N'] = N
    l, _ = sk
    # Bmsg should be lower that N/(Bsk*2^kappa)
    kappa = 40 * log2(num_of_mults) # the correctness parameter of the share conversion from Z_N to Z.
    state['kappa'] = kappa
    Bmsg = zz_pow("2", Bmsg_length)
    state['Bmsg'] = Bmsg
    Bsk = zz_div(N, zz_mul(Bmsg, zz_pow("2", kappa)))
    state['Bsk'] = Bsk
    d = []
    while(l != "0"):
        d.append(zz_mod(l, Bsk))
        l = zz_div(l, Bsk)
    random_range = zz_mul(zz_pow("2", kappa), Bsk)
    d_0 = []
    d_1 = []
    for digit in d:
        digit_1 = zz_random_smaller_than_n(random_range)
        d_1.append(digit_1)
        d_0.append(zz_sub(digit_1, digit))
    k_prf = zz_random(128)
    ek_0 = (k_prf, *d_0)
    ek_1 = (k_prf, *d_1)
    D = []
    for digit in d:
        D.append(Paillier.Enc(digit, pk_paillier))
    pk = ((N, g), D)
    return (pk, ek_0, ek_1)

def Input(pk, x):
    (N, g), D = pk
    X = Paillier.Enc(x, (N, g))
    I = []
    I.append(X)
    for digit in D:
        pk_for_digit = (N, digit)
        I.append(Paillier.Enc(x, pk_for_digit))
    return (I, I)

def Load(b, pk, ek, I, id):
    zz_p_init(pk[0][0])
    kappa = state['kappa']
    secret_share_1 = zz_p_add(zz_mod(str(int.from_bytes(prf.apply(int(ek[0]).to_bytes(16, 'big'), b'1'), 'big')), zz_mul(kappa, kappa)), b)
    memory_value_1 = (secret_share_1, *ek[1:])
    return Mul(b, ek, I, memory_value_1, id)

def Add_Inputs(b, ek, i1, i2, id):
    N = state['N']
    zz_p_init(zz_pow(N, "2"))
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
    N = state['N']
    Bsk = state['Bsk']
    yd = calculate_yd(m[1:], Bsk)
    zd = []
    N_squared = zz_pow(N, "2")
    for j in range(len(i)):
        zz_p_init(N_squared)
        zd.append(zz_mod(zz_add(DDLog(zz_p_pow(i[j], yd), N), str(int.from_bytes(prf.apply(int(ek[0]).to_bytes(16, 'big'), zz_add(id, str(j)).encode("utf-8")), 'big'))), N))
    return zd

def Output(b, ek, m, n_out, id):
    return zz_mod(m[0], n_out)

def DDLog(g, N):
    zz_p_init(N)
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
