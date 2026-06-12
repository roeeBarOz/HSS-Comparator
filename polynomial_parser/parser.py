import sympy as sp
import math

def build_binary_tasks(expr, result_var, task_list, counter):
    """
    מפרקת ביטויים ארוכים לפעולות בינאריות (עץ מאוזן) לאפשור מקבול מרבי.
    """
    if isinstance(expr, sp.Symbol) or isinstance(expr, sp.Number): 
        return expr
        
    processed_args = [build_binary_tasks(arg, None, task_list, counter) for arg in expr.args]

    if len(processed_args) <= 2:
        task_list[result_var or f"t_{counter[0]}"] = {'op': type(expr).__name__, 'args': processed_args}
        name = result_var or f"t_{counter[0]}"
        counter[0] += 1
        return sp.Symbol(name)
    
    while len(processed_args) > 1:
        next_level = []
        for i in range(0, len(processed_args), 2):
            if i + 1 < len(processed_args):
                temp_var = f"t_{counter[0]}"
                counter[0] += 1
                task_list[temp_var] = {'op': type(expr).__name__, 'args': [processed_args[i], processed_args[i+1]]}
                next_level.append(sp.Symbol(temp_var))
            else:
                next_level.append(processed_args[i])
        processed_args = next_level

    final_temp = processed_args[0].name
    task_list[result_var or f"t_{counter[0]}"] = task_list.pop(final_temp)
    if not result_var: 
        counter[0] += 1
    return sp.Symbol(result_var or final_temp)

def analyze_and_schedule(poly_str, num_processors):
    """
    מנתחת את הפולינום, מבצעת אופטימיזציה אלגברית, CSE ותזמון לחומרה.
    """
    try:
        parsed_poly = sp.sympify(poly_str)
        optimized_poly = sp.factor(parsed_poly)
        reduced_exprs, final_expr = sp.cse(optimized_poly)
        
        tasks = {}
        counter = [1]
        
        for var, expr in reduced_exprs:
            build_binary_tasks(expr, str(var), tasks, counter)
        build_binary_tasks(final_expr[0], "Final_Result", tasks, counter)

        depths = {}
        def get_depth(arg):
            if isinstance(arg, sp.Number): return 0
            return depths.get(str(arg), 0)

        for task_name, task_info in tasks.items():
            op = task_info['op']
            args = task_info['args']
            max_arg_depth = max((get_depth(arg) for arg in args), default=0)

            if op == 'Mul':
                if any(isinstance(arg, sp.Number) for arg in args):
                    depths[task_name] = max_arg_depth
                else:
                    depths[task_name] = max_arg_depth + 1
            elif op == 'Pow':
                base, exp = args
                if isinstance(exp, sp.Number) and exp > 0:
                    depths[task_name] = get_depth(base) + math.ceil(math.log2(float(exp)))
                else:
                    depths[task_name] = max_arg_depth
            else:
                depths[task_name] = max_arg_depth

        mult_depth = depths.get("Final_Result", 0)

        available_vars = set()
        for task in tasks.values():
            for arg in task['args']:
                if str(arg) not in tasks:
                    available_vars.add(str(arg))

        pending_tasks = set(tasks.keys())
        cycle = 1
        execution_plan = []
        hss_multiplication_cycles = 0 

        while pending_tasks:
            ready_queue = []
            for task_name in pending_tasks:
                dependencies = [str(arg) for arg in tasks[task_name]['args'] if isinstance(arg, sp.Symbol)]
                if all(dep in available_vars for dep in dependencies):
                    ready_queue.append(task_name)
            
            ready_queue.sort()
            tasks_this_cycle = ready_queue[:num_processors]
            if not tasks_this_cycle: break 
                
            plan_step = []
            cycle_has_heavy_mul = False 
            
            for idx, task_name in enumerate(tasks_this_cycle):
                task_info = tasks[task_name]
                op = task_info['op']
                args = task_info['args']
                
                if op == 'Mul':
                    vars_count = sum(1 for arg in args if not isinstance(arg, sp.Number))
                    if vars_count > 1: cycle_has_heavy_mul = True
                elif op == 'Pow':
                    cycle_has_heavy_mul = True
                    
                args_str = ", ".join(str(arg) for arg in args)
                plan_step.append(f"Core {idx+1}: {task_name} = {op}({args_str})")
                pending_tasks.remove(task_name)
            
            execution_plan.append((cycle, plan_step))
            if cycle_has_heavy_mul: hss_multiplication_cycles += 1
            for task_name in tasks_this_cycle: available_vars.add(task_name)
            cycle += 1

        return {
            'success': True, 
            'plan': execution_plan, 
            'total_cycles': len(execution_plan), 
            'hss_muls': hss_multiplication_cycles, 
            'depth': mult_depth, 
            'optimized': str(optimized_poly)
        }
    except Exception as e:
        return {'success': False, 'error': str(e)}