class Static_Data:
    def __init__(self, success, optimized_poly, plan, plan_len, num_unique_inputs, max_bit_size, op_counts):
        self.success = success
        self.optimized_poly = optimized_poly
        self.plan = plan
        self.plan_len = plan_len
        self.num_unique_inputs = num_unique_inputs
        self.max_bit_size = max_bit_size
        self.op_counts = op_counts
        
    def get_success(self):
        return self.success
    
    def get_optimized_poly(self):
        return self.optimized_poly
    
    def get_plan(self):
        return self.plan
    
    def get_plan_len(self):
        return self.plan_len
    
    def get_num_unique_inputs(self):
        return self.num_unique_inputs
    
    def get_max_bit_size(self):
        return self.max_bit_size
    
    def get_op_counts(self):
        return self.op_counts

class Op_Counts:
    def __init__(self, to_mem=0, muls=0, add_mems=0):
        self.to_mem = to_mem
        self.muls = muls
        self.add_mems = add_mems
        
    def increment_to_mem(self):
        self.to_mem += 1
        
    def increment_muls(self):
        self.muls += 1
        
    def increment_add_mems(self):
        self.add_mems += 1
        
    def get_to_mem(self):
        return self.to_mem
    
    def get_muls(self):
        return self.muls
    
    def get_add_mems(self):
        return self.add_mems
    
class Plan:
    def __init__(self):
        self.plan = {}
        
    def add_step(self, cycle, actions):
        self.plan[cycle] = actions
        
    def get_plan(self):
        return self.plan
    
    def __len__(self):
        return len(self.plan)
    
    def get_cycles(self):
        return list(self.plan.keys())
    
    def get_cycle_operations(self, cycle):
        output = [False, False, False]
        for action in self.plan.get(cycle, []):
            op = action.get_op()
            if op == 'To_Memory': output[0] = True
            elif op == 'Mul_In_Mem': output[1] = True
            elif op == 'Add_Memory': output[2] = True
        return tuple(output)
    
    def items(self):
        return self.plan.items()
    
class Plan_Step:
    def __init__(self, core_idx, task_name, op, args):
        self.core_idx = core_idx
        self.task_name = task_name
        self.op = op
        self.args = args
        
    def __str__(self):
        args_str = ", ".join(str(arg) for arg in self.args)
        return f"Core {self.core_idx}: {self.task_name} = {self.op}({args_str})"
    
    def get_op(self):
        return self.op
    