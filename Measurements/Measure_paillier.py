import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import Encryptions.Paillier as Paillier
import timeit

def measure(sizes, times, message):
    for size in sizes:
        n = "1" * size
        duration = timeit.timeit(lambda: Paillier.Gen(n), number=times)
        print(f"Average key generation time for size {size}: {duration / times:.6f} seconds")
        pk, sk = Paillier.Gen(n)
        duration = timeit.timeit(lambda: [Paillier.Enc(n, message, pk) for i in range(times)], number=1)
        print(f"Average encryption time for size {size}: {duration / times:.6f} seconds")
        duration = timeit.timeit(lambda: [Paillier.Dec(pk, c, sk) for c in ciphertexts], number=1)
        print(f"Average decryption time for size {size}: {duration / times:.6f} seconds")
