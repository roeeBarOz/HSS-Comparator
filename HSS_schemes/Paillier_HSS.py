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
)
import PRF.aes_prf as prf

kappa = None
Bsk = None
Bmsg = None
N = None

def Setup(n):
    # step 0: setting kappa
    n = len(n)
    kappa = zz_div(zz_mul(n, 2), 3) # as n refers to the length of p, q, and not the length of N=p*q.
    # step 1: generate pk_paillier and digits of sk_paillier, and set Bsk, Bmsg
    # (with the specific case of the article "The Rise of Paillier")
    pk_paillier, sk = Paillier.Gen(n)
    N, g = pk_paillier
    # set Bsk, Bmsg
    Bmsg = zz_pow(2, kappa)
    Bsk = zz_div(N, zz_pow(Bsk, 2))
    d = []
    while(sk != "0"):
        d.append(zz_mod(sk, Bsk))
        sk = zz_div(sk, Bsk)
    # step 2: secret share the digits of sk
    random_range = zz_mul(zz_pow("2", kappa), Bsk)
    d_0 = []
    d_1 = []
    for digit in d:
        digit_1 = zz_random_smaller_than_n(random_range)
        d_1.append(digit_1)
        d_0.append(zz_sub(digit_1, digit))
    # step 3: selecting k_prf to be kappa bits long
    k_prf = zz_random(kappa)
    # step 4: create ek_0, ek_1
    ek_0 = (k_prf, *d_0) # * flattens the array
    ek_1 = (k_prf, *d_1)
    # step 5: encrypt the digits of sk
    D = []
    for digit in d:
        D.append(Paillier.Enc(pk_paillier, digit))
    pk = ((N, g), D)
    return (pk, ek_0, ek_1)

def Input(pk, x):
    pk, ek_0, ek_1 = pk
    (N, g), D = pk
    X = Paillier.Enc(x, (N, g))
    I = []
    for digit in D:
        pk_for_digit = (N, digit)
        I.append(Paillier.Enc(pk_for_digit, x))
    return (I, I)

def Load(b, ek, I, id):
    zz_p_init(N)
    secret_share_1 = zz_p_add(zz_mod(prf.apply(bytes(ek[0]), bytes("1")), zz_mul(kappa, kappa)).decode(), b)
    memory_value_1 = (secret_share_1, *ek[1:])
    return Mul(b, ek, I, memory_value_1, id)

def Add(b, ek, i1, i2, id):
    zz_p_init(zz_pow(N, 2))
    Z = []
    for j in range(len(i1)):
        i1_j = i1[j]
        i2_j = i2[j]
        Z.append(zz_p_mul(i1_j, i2_j))
    return Z

def Add(b, ek, m1, m2, id):
    Z = []
    for j in range(len(m1)):
        m1_j = m1[j]
        m2_j = m2[j]
        Z.append(zz_add(m1_j, m2_j))
    return Z

def Mul(b, ek, i, m, id):
    yd = "0"
    digit_value = "1"
    for j in range(1, len(m)):
        digit = m[j]
        yd = zz_add(yd, zz_mul(digit_value, digit))
        digit_value = zz_mul(digit_value, Bsk)
    zd = []
    for j in range(len(i)):
        zd.append(zz_mod(DDLog(zz_pow(i[j], yd), N) + prf.apply(bytes(ek[0]), bytes(zz_add(id, j))), N))
    return zd

def Output(b, ek, m, n_out, id):
    return zz_mod(m[0], n_out)

def DDLog(g, N):
    zz_p_init(N)
    h = zz_mod(g, N)
    h_tag = zz_div(zz_sub(g, h), N)
    return zz_p_mul(h_tag, zz_p_inv(h))

