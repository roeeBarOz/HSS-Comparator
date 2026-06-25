import random

import HSS_schemes.Paillier_HSS as Paillier
from Measurements.measurement_output import Measurements_Output

def measure_Paillier(security_parameter, Bmsg_length, num_of_mults, times=40):
    setup_time = Paillier.benchmark_setup('1'*security_parameter, Bmsg_length, num_of_mults, times)
    pk, ek0, ek1 = Paillier.Setup('1'*security_parameter, Bmsg_length, num_of_mults)
    input_time = Paillier.benchmark_input(pk, ek0, ek1, times)
    I = Paillier.Input(pk, str(random.randint(0, 2**Bmsg_length - 1)))[0]  # Use a sample input for the multiplication benchmark
    load_time = Paillier.benchmark_load(pk, ek0, ek1, times)
    add_memory_time = Paillier.benchmark_add_memory(pk, ek0, ek1, times)
    mul_time = Paillier.benchmark_mul(pk, ek0, ek1, I, times)
    return Measurements_Output(setup_time, input_time, load_time, add_memory_time, mul_time)
