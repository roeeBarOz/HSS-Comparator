import Measurements.Measure_paillier_hss as measure

measure.measure(
    sizes=[512, 1024, 2048],
    times=10,
    message="123456789123456789123456789"
)