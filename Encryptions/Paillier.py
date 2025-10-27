from NTL_interfaces.ZZ_p_interface import (
    zz_p_mul,
    zz_p_pow,
    zz_p_inv,
    zz_p_init,
    zz_p_add
)
from NTL_interfaces.ZZ_interface import (
    zz_sub,
    zz_mul,
    zz_div,
    zz_gcd,
    zz_random_prime,
    zz_random_smaller_than_n
)

def Gen(n: str) -> str:
    """
        Generate n, g, lambda, mu for Paillier encryption.
        n: string representation of the bit length of the primes to be generated.
        g, q: random primes of bit length n.
        n: product of g and q.
        l: least common multiple of g-1 and q-1.
        mu: modular inverse of l.
        pk: public key (n, g).
        sk: secret key (l, mu).
        Returns a tuple (pk, sk) where pk is the public key and sk is the secret key.
    """
    n = len(n)
    p = zz_random_prime(n, 100)
    q = zz_random_prime(n, 100)
    gcd = zz_gcd(zz_mul(p, q), zz_mul(zz_sub(p, "1"), zz_sub(q, "1"))) # good gcd
    while gcd != "1":
        p = zz_random_prime(n, 100)
        q = zz_random_prime(n, 100)
        gcd = zz_gcd(zz_mul(p, q), zz_mul(zz_sub(p, "1"), zz_sub(q, "1")))
    n = zz_mul(p, q) # good n
    l = zz_mul(zz_sub(p, "1"), zz_sub(q, "1")) # good lcm
    zz_p_init(n)
    mu = zz_p_inv(l) # mu = l^-1 mod n, good mu
    sk = zz_mul(l, mu) # sk = (l, mu)
    return (n, sk)

def Enc(m: str, pk: str, g: str = "not given") -> str:
    """
        Encrypt a message m using the public key pk.
        m: string representation of the message to be encrypted.
        pk: public key.
        Returns the ciphertext as a string.
    """
    N_squared = zz_mul(pk, pk)
    r = zz_random_smaller_than_n(N_squared)
    zz_p_init(N_squared)
    c1 = ""
    if g == "not given":
        c1 = zz_p_add("1", zz_mul(pk, m))
    else:
        c1 = zz_p_pow(g, m)
    c2 = zz_p_pow(r, pk)
    c = zz_p_mul(c1, c2)
    return c

def Dec(pk: str, c: str, sk: str) -> str:
    """
        Decrypt a ciphertext c using the secret key sk.
        c: string representation of the ciphertext to be decrypted.
        sk: secret key.
        pk: public key.
        Returns the decrypted message as a string.
    """
    zz_p_init(zz_mul(pk, pk))
    c1 = zz_p_pow(c, sk)
    return zz_div(zz_sub(c1, "1"), pk)
