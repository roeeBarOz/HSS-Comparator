# fast_mod_exp.py
# This is the Python file you will run.
# It imports our compiled C++ module and provides a clean
# Python function to call it.

import rns_avx2_backend  # This is our compiled C++ module
import threading
import time
import random
import sympy

# --- Context Caching ---
# The RnsContext precomputation is very expensive. We must cache it.
# We'll create one context for each modulus we use.
MODULUS_CACHE = {}
# Thread-lock to make the cache safe for multi-threaded apps
CACHE_LOCK = threading.Lock()

def _get_context(modulus_str: str):
    """
    Retrieves a cached RnsContext or creates a new one.
    This is the key to high performance.
    """
    # Fast path: check if it's already in the cache
    if modulus_str in MODULUS_CACHE:
        return MODULUS_CACHE[modulus_str]

    # Slow path: acquire lock, check again, and create if needed
    with CACHE_LOCK:
        # Check again in case another thread created it
        # while we were waiting for the lock
        if modulus_str in MODULUS_CACHE:
            return MODULUS_CACHE[modulus_str]
        
        # This calls the C++ constructor we bound
        ctx = rns_avx2_backend.RnsContext(modulus_str)
        
        MODULUS_CACHE[modulus_str] = ctx
        return ctx

# --- The Final Function ---

def mod_exp(base_str: str, exp_str: str, modulus_str: str) -> str:
    """
    Performs fast modular exponentiation (base^exp) % modulus
    using the RNS Montgomery AVX2 C++ backend.

    All arguments and the return value are strings.
    """
    if not isinstance(base_str, str) or \
       not isinstance(exp_str, str) or \
       not isinstance(modulus_str, str):
        raise TypeError("All arguments must be strings.")

    # 1. Get the precomputed context (from cache or create new)
    ctx = _get_context(modulus_str)

    # 2. Call the C++ 'mod_exp' method we bound
    #    This one function call does all the work in C++
    result_str = ctx.mod_exp(base_str, exp_str)

    return result_str

# --- Example Usage ---
if __name__ == "__main__":
    # aes = []
    # bes = []
    # p = str(sympy.randprime(1, 2**(32*32)))
    # for i in range(100):
    #     a = random.randint(1, int(p)-1)
    #     b = random.randint(1, int(p)-1)
    #     aes.append(str(a))
    #     bes.append(str(b))
    # start = time.time()
    # for i in range(100):
    #     mod_exp(aes[i], bes[i], p)
    # end = time.time()
    # print(f"100 modular exponentiations took {end - start} seconds")
    # start = time.time()
    # for i in range(100):
    #     a = int(aes[i])
    #     b = int(bes[i])
    #     p = int(p)
    #     expected = a*b % p
    # end = time.time()
    # print(f"100 modular exponentiations in pure Python took {end - start} seconds")
    a = "1213732911"
    b = "2685934482"
    p = sympy.randprime(1, 2**512)
    print(f"Modulus p = {p}")
    start = time.time()
    result = mod_exp(a, b, str(p))
    end = time.time()
    print(f"Modular exponentiation took {end - start} seconds")
    # start = time.time()
    # for _ in range(100):
    #     result = mod_exp(a, b, str(p))
    # end = time.time()
    # print(f"Modular exponentiation took {(end - start)/100} seconds")
    # print(f"{a} ^ {b} mod {p} = {result}")
    # res = pow(int(a), int(b), int(p))
    # print(f"Expected: {res}")

