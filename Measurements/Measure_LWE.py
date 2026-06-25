import HSS_schemes.LWE_HSS as LWE
from Measurements.measurement_output import Measurements_Output

def measure_LWE(security_parameter, Bmax, num_of_mults, num_of_adds, times=40):
    setup_time = LWE.benchmark_Setup(security_parameter, Bmax, num_of_mults + num_of_adds, times)
    input_time = LWE.benchmark_input(times)
    load_or_mul_time = LWE.benchmark_load_or_mul(times)
    add_memory_time = LWE.benchmark_mem_add(times)
    return Measurements_Output(setup_time, input_time, load_or_mul_time, add_memory_time, load_or_mul_time)