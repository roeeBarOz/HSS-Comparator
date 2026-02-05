import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import Encryptions.Paillier as Paillier
import time

def measure(sizes, times, message):
    for size in sizes:
        n = "1" * size
        start = time.time()
        for _ in range(times):
            Paillier.Gen(n)
        end = time.time()
        print(f"Average key generation time for size {size}: {(end - start) / times:.6f} seconds")
        pk, sk = Paillier.Gen(n)
        start = time.time()
        for _ in range(times):
            Paillier.Enc(message, pk)
        end = time.time()
        c = Paillier.Enc(message, pk)
        print(f"Average encryption time for size {size}: {(end - start) / times:.6f} seconds")
        start = time.time()
        for _ in range(times):
            Paillier.Dec(pk, c, sk)
        end = time.time()
        print(f"Average decryption time for size {size}: {(end - start) / times:.6f} seconds")
