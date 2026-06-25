from Measurements.Measure_paillier import measure_Paillier
from Measurements.Measure_LWE import measure_LWE
from polynomial_parser.data import Plan
from Measurements.measurement_output import Measurements_Output, Online_and_Offline

def measure_schemes(Bmax, num_of_mults, num_of_adds, num_of_unique_inputs, plan: Plan, times=40):
    try:
        print(f"Measuring schemes with Bmax={Bmax}, num_of_mults={num_of_mults}, num_of_adds={num_of_adds}, times={times}")
        pai_times: Online_and_Offline = get_times(128, Bmax, num_of_adds, num_of_mults, times, get_measure_Paillier(), plan, num_of_unique_inputs)
        lwe_times: Online_and_Offline = get_times(128, Bmax, num_of_adds, num_of_mults, times, get_measure_LWE(), plan, num_of_unique_inputs)
        output = {
            "Paillier": {
                "offline_ms": pai_times.get_offline_time(),
                "online_ms": pai_times.get_online_time(),
                "total_ms": pai_times.get_total_time()
            },
            "LWE": {
                "offline_ms": lwe_times.get_offline_time(),
                "online_ms": lwe_times.get_online_time(),
                "total_ms": lwe_times.get_total_time()
            }
        }
        return output
    except Exception as e:
        print(f"Error occurred: {e}")
        return None
    
def get_online_time(plan: Plan, times: Measurements_Output):
    total_online_time = 0
    for cycle in plan.get_cycles():
        ops = plan.get_cycle_operations(cycle)
        max_operation = 0
        if ops[0] and times.get_load_time() > max_operation:
            max_operation = times.get_load_time()
        if ops[1] and times.get_mul_time() > max_operation:
            max_operation = times.get_mul_time()
        if ops[2] and times.get_add_memory_time() > max_operation:
            max_operation = times.get_add_memory_time()
        total_online_time += max_operation
    return total_online_time

def get_times(security_parameter, Bmax, num_of_adds, num_of_mults, times, func, plan: Plan, num_of_unique_inputs):
    times_per_op: Measurements_Output = func(security_parameter, Bmax, num_of_adds, num_of_mults, times)
    online_time = get_online_time(plan, times_per_op)
    offline_time = times_per_op.get_setup_time() + times_per_op.get_input_time() * num_of_unique_inputs
    return Online_and_Offline(online_time, offline_time)
    
def get_measure_Paillier():
    return lambda security_parameter, Bmax, num_of_adds, num_of_mults, times: measure_Paillier(security_parameter, Bmax, num_of_mults, times)

def get_measure_LWE():
    return lambda security_parameter, Bmax, num_of_adds, num_of_mults, times: measure_LWE(security_parameter, Bmax, num_of_mults, num_of_adds, times)