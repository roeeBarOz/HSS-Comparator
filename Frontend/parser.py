import re


def parse_sum_of_products(poly_str):
    poly_str = poly_str.replace(" ", "")
    # Split into terms by + or - (keeping track of terms)
    # This regex splits by + or - but ignores them for the term content
    terms = re.split(r'\+|(?=-)', poly_str) 
    
    parsed_poly = []
    plus_or_minus = []
    
    if poly_str[0] == 'x':
        plus_or_minus.append(1)
    
    for c in poly_str:
        if c == '+':
            plus_or_minus.append(1)
        elif c == '-':
            plus_or_minus.append(-1)
    
    for term in terms:
        if not term: continue
        current_term_indices = []
        
        # 1. Match x_i^k (variable with exponent)
        # 2. Match x_i (variable without exponent)
        # Pattern: x_ (group 1: index) then optional ^ (group 2: exponent)
        matches = re.findall(r"x_(\d+)(?:\^(\d+))?", term)
        
        for var_idx, exponent in matches:
            idx = int(var_idx)
            # If no exponent, count is 1. Otherwise, count is the exponent value.
            count = int(exponent) if exponent else 1
            
            # Extend the list: x_1^3 becomes [1, 1, 1]
            current_term_indices.extend([idx] * count)
            
        if current_term_indices:
            parsed_poly.append(current_term_indices)
            
    return parsed_poly, plus_or_minus

def is_sum_of_products(poly_str):
    poly = poly_str.replace(" ", "")
    
    # SOP cannot have parentheses
    if "(" in poly or ")" in poly:
        return False

    # Split by + or - (but handle the sign properly)
    terms = re.split(r'[+-]', poly)
    
    # Valid: digits, x_ followed by digits, *, or ^
    valid_pattern = re.compile(r'^(\d+|x_\d+|\*|\^)+$')
    
    for term in terms:
        if not term: continue
        if not valid_pattern.match(term):
            return False
    return True

# Example: "x_1 * x_2 * x_3 + x_1 * x_4" 
# Returns: [[1, 2, 3], [1, 4]]
def main():
    poly_str = "x_1^5 * x_2 * x_3 - x_1 * x_4 + x_5"
    parsed_poly = []
    plus_or_minus = []
    if is_sum_of_products(poly_str):
        parsed_poly, plus_or_minus = parse_sum_of_products(poly_str)
    print(parsed_poly)
    print(plus_or_minus)

if __name__ == "__main__":
    main()