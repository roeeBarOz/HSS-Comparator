from NTL_interfaces.vec_mat_ZZ_interface import (
    mat_random,
    vec_create_e,
    vec_gaussian,
    mat_mul_vec,
    vec_add,
    vec_prepend_one,
    mat_neg,
    mat_concat_col_first,
    vec_random_binary,
    mat_transpose,
    vec_mul_scalar
)
from NTL_interfaces.ZZ_p_interface import (
    zz_p_init,
    zz_p_mul
)
from NTL_interfaces.ZZ_interface import (
    zz_random_prime,
    zz_mul
)

state = {}

def Gen(n: str, q: str, p: str) -> tuple[str, str]:
    """
        Generate public and secret keys for LWE encryption.
        Returns a tuple (pk, sk) where pk is the public key and sk is the secret key.
    """
    # stuff to notice:
    # 1. im using n for both initialization of p,q and setting the matrix and vectors sizes.
    # 2. I wrote here a lot of wrong stuff, it should all be fixed later.
    n = len(n)
    p = zz_random_prime(n, 100)
    log_n = str(len(n))
    M = zz_mul(n, log_n) # might be modified
    state['n'] = n
    state['M'] = M
    state['q'] = q
    state['p'] = p
    A = sample_matrix(M, n, q)
    s = vec_gaussian(n)
    e = vec_gaussian(M)
    b = vec_add(mat_mul_vec(A, s), e)
    sk = vec_prepend_one(s)
    neg_A = mat_neg(A)
    pk = mat_concat_col_first(b, neg_A)
    return (pk, sk)

def Enc(m: str, pk: str) -> str:
    """
        Encrypt a message m using the public key pk.
        m: string representation of the message to be encrypted.
        pk: public key.
        Returns the ciphertext as a string.
    """
    n = state['n']
    M = state['M']
    q = state['q']
    p = state['p']
    m = vec_create_e(m, n+1, 0) # create (m, 0, ..., 0)
    r = vec_random_binary(M)
    q_divided_by_p = p # might be modified
    zz_p_init(q)
    c = vec_add(mat_mul_vec(mat_transpose(pk), r), vec_mul_scalar(m, q_divided_by_p))
    return c

def sample_matrix(m: str, n: str, q: str) -> str:
    """
        Helper function. takes m,n,q and samples a a matrix in Z_q^{m*n}
    """
    zz_p_init(q)
    return mat_random(m, n)
