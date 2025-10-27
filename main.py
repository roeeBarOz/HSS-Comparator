import Measurements.Measure_paillier_hss as measure
import NTL_interfaces.ZZ_interface as zz

measure.measure(
    sizes=[512, 1024, 1536, 2048],
    times=10,
    message=zz.zz_random(70)
)