class Measurements_Output:
    def __init__(self, setup_time, input_time, load_time, add_memory_time, mul_time):
        self.setup_time = setup_time
        self.input_time = input_time
        self.load_time = load_time
        self.add_memory_time = add_memory_time
        self.mul_time = mul_time
        
    def get_setup_time(self):
        return self.setup_time
    
    def get_input_time(self):
        return self.input_time
    
    def get_load_time(self):
        return self.load_time
    
    def get_add_memory_time(self):
        return self.add_memory_time
    
    def get_mul_time(self):
        return self.mul_time
    
class Online_and_Offline:
    def __init__(self, online_time, offline_time):
        self.online_time = online_time
        self.offline_time = offline_time
        
    def get_online_time(self):
        return self.online_time
    
    def get_offline_time(self):
        return self.offline_time
    
    def get_total_time(self):
        return self.online_time + self.offline_time
    