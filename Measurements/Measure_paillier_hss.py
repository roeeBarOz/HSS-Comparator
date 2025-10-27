import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import HSS_schemes.Paillier_HSS as hss
import timeit

def measure(sizes, times, message):
    for size in sizes:
        n = "1" * size
        duration = timeit.timeit(lambda: hss.Setup(n), number=times)
        print(f"Average key generation time for size {size}: {duration / times:.6f} seconds")

        pk, ek_0, ek_1 = hss.Setup(n)
        duration = timeit.timeit(lambda: hss.Input(pk, message), number=times)
        print(f"Average input time for size {size}: {duration / times:.6f} seconds")

        I1, I2 = hss.Input(pk, message)
        hss.Mul("0", ek_0, I1, I1, "1")  # warm up

        # duration = timeit.timeit(lambda: hss.Load("0", pk, ek_0, I1, "1"), number=times)
        # print(f"Average load time for size {size}: {duration / times:.6f} seconds")

        duration = timeit.timeit(lambda: hss.Add_Inputs("0", ek_0, I1, I2, "3"), number=times)
        print(f"Average input addition time for size {size}: {duration / times:.6f} seconds")

        m1 = hss.Load("0", pk, ek_0, I1, "1")
        m2 = hss.Load("0", pk, ek_0, I2, "2")
        duration = timeit.timeit(lambda: hss.Add_Memory_Values("0", ek_0, m1, m2, "3"), number=times)
        print(f"Average memory addition time for size {size}: {duration / times:.6f} seconds")

        duration = timeit.timeit(lambda: hss.Mul("0", ek_0, I1, m1, "3"), number=times)
        print(f"Average multiplication time for size {size}: {duration / times:.6f} seconds")
        
        # yd = hss.calculate_yd(m1[1:])
        # duration = timeit.timeit(lambda: hss.calc_powers(I1, yd), number=times)
        # print(f"Average power calculation time for size {size}: {duration / times:.6f} seconds")
        print("-" * 100)