import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import HSS_schemes.Paillier_HSS as hss
import time

def measure(sizes, times, message):
    for size in sizes:
        n = "1" * size
        start = time.time()
        for i in range(times):
            pk, _, _ = hss.Setup(n)
        end = time.time()
        print(f"Average key generation time for size {size}: {(end - start) / times:.6f} seconds")
        
        pk, ek_0, ek_1 = hss.Setup(n)
        start = time.time()
        for i in range(times):
            I1, I2 = hss.Input(pk, message)
        end = time.time()
        print(f"Average input time for size {size}: {(end - start) / times:.6f} seconds")
        
        start = time.time()
        for i in range(times):
            m1 = hss.Load("0", pk, ek_0, I1, "1")
        end = time.time()
        print(f"Average load time for size {size}: {(end - start) / times:.6f} seconds")
        
        start = time.time()
        for i in range(times):
            result = hss.Add_Inputs("0", ek_0, I1, I2, "3")
        end = time.time()
        print(f"Average input addition time for size {size}: {(end - start) / times:.6f} seconds")

        m1 = hss.Load("0", pk, ek_0, I1, "1")
        m2 = hss.Load("0", pk, ek_0, I2, "2")
        start = time.time()
        for i in range(times):
            result = hss.Add_Memory_Values("0", ek_0, m1, m2, "3")
        end = time.time()
        print(f"Average memory addition time for size {size}: {(end - start) / times:.6f} seconds")

        start = time.time()
        for i in range(times):
            result = hss.Mul("0", ek_0, I1, m1, "3")
        end = time.time()
        print(f"Average multiplication time for size {size}: {(end - start) / times:.6f} seconds")
        print("-" * 100)