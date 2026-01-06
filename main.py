import Measurements.Measure_paillier_hss as measure
import NTL_interfaces.ZZ_interface as zz
import NTL_interfaces.ZZ_p_interface as zz_p
import timeit
import NTL_wrappers.Multiplication.fast_mod_exp as fme

# measure.measure(
#     sizes=[512, 1024, 1536, 2048],
#     times=10,
#     message=zz.zz_random(70)
# )

a = zz.zz_random(4096)
b = zz.zz_random(4096)

zz_p.zz_p_init(zz.zz_random(4096))

duration = timeit.timeit(lambda: zz_p.zz_p_pow(a, b), number=100)
print(f"Average zz_p_pow time for size 4096: {duration / 100:.6f} seconds")