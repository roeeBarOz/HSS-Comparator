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


state = {}

def Setup(n):
    (pk, sk) = LWE.Gen(n, "0", "0") # need to be modified
    n = len(n)
    s_0 = vec_random(n)
    s_1 = vec_sub(sk, s_0)
    k_prf = int(zz_random(128))
    ek_0 = (k_prf, s_0)
    ek_1 = (k_prf, s_1)
    state['zero_encrypted'] = LWE.Enc("0", pk)
    return (pk, ek_0, ek_1)

def Input(pk, x):
    zero_encrypted = state.get("zero_encrypted")
    p = state.get("p")
    q = state.get("q")
    return OKDM(x, zero_encrypted, p, q)

def Load(b, ek, C, id):
    """
    Here C is the ciphertext of the input, represented as a dxd matrix
    """
    k_prf, s = ek
    return vec_add_scalar(DDEC(C, s), str((1-2*int(b)) + int.from_bytes(prf.apply(k_prf.to_bytes(16, 'big'), id), 'big')))

def Add_Inputs(b, ek, C1, C2, id):
    return mat_add(C1, C2)

def Add_Memory_Values(b, ek, m1, m2, id):
    k_prf, _ = ek
    return vec_add_scalar(vec_add(m1, m2), str((1-2*int(b)) + int.from_bytes(prf.apply(k_prf.to_bytes(16, 'big'), id), 'big')))

def Mul(b, ek, C, m, id):
    k_prf, _ = ek
    return vec_add_scalar(DDEC(C, m), str((1-2*int(b)) + int.from_bytes(prf.apply(k_prf.to_bytes(16, 'big'), id), 'big')))

def Output(b, ek, m, r, id):
    pass
