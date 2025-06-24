import Encryptions.Paillier as Paillier
from NTL_interfaces.ZZ_interface import {
    zz_mod,
    zz_div,
    zz_pow,
    zz_mul,
    zz_random_smaller_than_n, 
    zz_sub
}

kappa
Bsk
Bmsg
pk_paillier

def Setup(n):
    # step 1
    pk_paillier, sk = Paillier.Gen(n)
    d = []
    while(sk != "0"):
        d.append(zz_mod(sk, Bsk))
        sk = zz_div(sk, Bsk)
    # step 2
    random_range = zz_mul(zz_pow("2", kappa), Bsk)
    d_0 = []
    d_1 = []
    for digit in d:
        digit_1 = zz_random_smaller_than_n(random_range)
        d_1.append(digit_1)
        d_0.append(zz_sub(digit_1, digit))
    # step 3
    

def Input(pk, x):


def Load(b, ek, i, id):


def Add(b, ek, i1, i2, id):


def Add(b, ek, m1, m2, id):


def Mul(b, ek, i, m, id):


def Output(b, ek, m, n_out, id):

