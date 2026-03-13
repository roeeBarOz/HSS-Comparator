import Encryptions.LWE as LWE
from NTL_interfaces.vec_mat_ZZ_p_interface import (
    vec_random,
    vec_sub
)
from NTL_interfaces.ZZ_interface import (
    zz_random
)

def Setup(n):
    (pk, sk) = LWE.Gen(n, "0", "0") # need to be modified
    n = len(n)
    s_0 = vec_random(n)
    s_1 = vec_sub(sk, s_0)
    k_prf = zz_random(128)
    ek_0 = (k_prf, s_0)
    ek_1 = (k_prf, s_1)
    return (pk, ek_0, ek_1)

def Input(pk, x):
    pass

def Load(b, ek, C, id):
    pass

def Add_Inputs(b, ek, C1, C2, id):
    pass

def Add_Memory_Values(b, ek, m1, m2, id):
    pass

def Mul(b, ek, C, m, id):
    pass

def Output(b, ek, C, r, id):
    pass
