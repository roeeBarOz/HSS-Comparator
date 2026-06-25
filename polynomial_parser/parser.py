import sympy as sp
import math

from polynomial_parser.data import Plan_Step, Static_Data, Plan, Op_Counts

def get_max_bit_size(expr, var_sizes):
    if isinstance(expr, sp.Symbol): return var_sizes.get(str(expr), var_sizes.get('default', 8))
    elif isinstance(expr, sp.Number):
        val = abs(float(expr))
        return 1 if val == 0 else math.floor(math.log2(val)) + 1
    elif isinstance(expr, sp.Add):
        arg_sizes = [get_max_bit_size(arg, var_sizes) for arg in expr.args]
        return max(arg_sizes) + math.ceil(math.log2(len(arg_sizes)))
    elif isinstance(expr, sp.Mul):
        return sum(get_max_bit_size(arg, var_sizes) for arg in expr.args)
    elif isinstance(expr, sp.Pow):
        base, exp = expr.args
        if isinstance(exp, sp.Number) and exp > 0:
            return get_max_bit_size(base, var_sizes) * int(exp)
    return var_sizes.get('default', 8)

def analyze_and_schedule(poly_str, num_processors, var_sizes):
    try:
        parsed_poly = sp.sympify(poly_str)
        expanded_poly = sp.expand(parsed_poly)
        
        # 1. חילוץ נתונים סטטיים בסיסיים
        free_symbols = expanded_poly.free_symbols
        num_unique_inputs = len(free_symbols)
        max_bit_size = get_max_bit_size(expanded_poly, var_sizes)
        
        tasks = {}
        counter = [1]
        def new_var():
            name = f"t_{counter[0]}"; counter[0] += 1; return name

        # בניית המשימות לפי מגבלות הארכיטקטורה (כפל קלט בזיכרון בלבד)
        def build_multiplication_chain(expr):
            if isinstance(expr, sp.Symbol):
                mem_var = new_var()
                tasks[mem_var] = {'op': 'To_Memory', 'args': [expr]}
                return mem_var
            elif isinstance(expr, sp.Mul):
                args = list(expr.args)
                secrets = [a for a in args if not isinstance(a, sp.Number)]
                if not secrets: return None
                current_mem = new_var()
                tasks[current_mem] = {'op': 'To_Memory', 'args': [secrets[0]]}
                for secret in secrets[1:]:
                    next_mem = new_var()
                    tasks[next_mem] = {'op': 'Mul_In_Mem', 'args': [secret, current_mem]}
                    current_mem = next_mem
                return current_mem

        term_results = []
        if isinstance(expanded_poly, sp.Add):
            for term in expanded_poly.args:
                res = build_multiplication_chain(term)
                if res: term_results.append(res)
        else:
            res = build_multiplication_chain(expanded_poly)
            if res: term_results.append(res)

        while len(term_results) > 1:
            next_level = []
            for i in range(0, len(term_results), 2):
                if i + 1 < len(term_results):
                    sum_mem = new_var()
                    tasks[sum_mem] = {'op': 'Add_Memory', 'args': [term_results[i], term_results[i+1]]}
                    next_level.append(sum_mem)
                else:
                    next_level.append(term_results[i])
            term_results = next_level

        # 2. תזמון המשימות לליבות וספירת פעולות (ללא זמנים)
        available_vars = set([str(s) for s in free_symbols])
        pending_tasks = set(tasks.keys())
        execution_plan = Plan()
        cycle = 1
        
        op_counts = Op_Counts()

        while pending_tasks:
            ready_queue = []
            for task_name in pending_tasks:
                dependencies = [str(arg) for arg in tasks[task_name]['args'] if isinstance(arg, sp.Symbol) or type(arg)==str]
                if all(dep in available_vars for dep in dependencies): ready_queue.append(task_name)
            
            ready_queue.sort()
            tasks_this_cycle = ready_queue[:num_processors]
            if not tasks_this_cycle: break 
                
            plan_step = []
            for idx, task_name in enumerate(tasks_this_cycle):
                task_info = tasks[task_name]
                op = task_info['op']
                plan_step.append(Plan_Step(core_idx=idx+1, task_name=task_name, op=op, args=task_info['args']))   
                             
                if op == 'To_Memory': op_counts.increment_to_mem()
                elif op == 'Mul_In_Mem': op_counts.increment_muls()
                elif op == 'Add_Memory': op_counts.increment_add_mems()
                
                pending_tasks.remove(task_name)
            
            execution_plan.add_step(cycle, plan_step)
            for task_name in tasks_this_cycle: available_vars.add(task_name)
            cycle += 1

        # מחזירים רק נתונים יבשים לאתר
        return Static_Data(
            success=True,
            optimized_poly=str(expanded_poly),
            plan=execution_plan,
            plan_len=len(execution_plan),
            num_unique_inputs=num_unique_inputs,
            max_bit_size=max_bit_size,
            op_counts=op_counts
        )

    except Exception as e:
        return Static_Data(
            success=False,
            optimized_poly=None,
            plan=None,
            plan_len=0,
            num_unique_inputs=0,
            max_bit_size=0,
            op_counts=None
        )