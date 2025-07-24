from NTL_interfaces.ZZ_p_interface import (
    zz_p_mul,
    zz_p_pow,
    zz_p_inv,
    zz_p_init
)
from NTL_interfaces.ZZ_interface import (
    zz_add,
    zz_sub,
    zz_mul,
    zz_div,
    zz_gcd,
    zz_random_prime,
    zz_lcm,
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
    print("p:", p)
    q = zz_random_prime(n, 100)
    print("q:", q)
    gcd = zz_gcd(zz_mul(p, q), zz_mul(zz_sub(p, "1"), zz_sub(q, "1"))) # good gcd
    while gcd != "1":
        p = zz_random_prime(n, 100)
        q = zz_random_prime(n, 100)
        gcd = zz_gcd(zz_mul(p, q), zz_mul(zz_sub(p, "1"), zz_sub(q, "1")))
    n = zz_mul(p, q) # good n
    l = zz_lcm(zz_sub(p, "1"), zz_sub(q, "1")) # good lcm
    g = zz_add(n, "1") # g = n + 1, a common choice for g in Paillier, good g
    zz_p_init(n)
    mu = zz_p_inv(l) # mu = l^-1 mod n, good mu
    pk = (n, g) # good pk
    sk = (l, mu) # good sk
    return (pk, sk)

def Enc(m: str, pk: tuple) -> str:
    """
        Encrypt a message m using the public key pk.
        n: string representation of the modulus.
        m: string representation of the message to be encrypted.
        pk: public key (n, g).
        Returns the ciphertext as a string.
    """
    n, g = pk
    r = zz_random_smaller_than_n(n)
    while zz_gcd(r, n) != "1":
        r = zz_random_smaller_than_n(n)
    zz_p_init(zz_mul(n, n))
    c1 = zz_p_pow(g, m)
    c2 = zz_p_pow(r, n)
    c = zz_p_mul(c1, c2)
    return c

def Dec(pk: tuple, c: str, sk: tuple) -> str:
    """
        Decrypt a ciphertext c using the secret key sk.
        n: string representation of the modulus.
        c: string representation of the ciphertext to be decrypted.
        sk: secret key (l, mu).
        Returns the decrypted message as a string.
    """
    l, mu = sk
    n, g = pk
    zz_p_init(zz_mul(n, n))
    c1 = zz_p_pow(c, l)
    zz_p_init(n)
    return zz_p_mul(mu, zz_div(zz_sub(c1, "1"), n))
