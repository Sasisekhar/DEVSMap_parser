import json

fixed_keys = ['include_sets', 'parameters']

class Parser:

    def __init__(self, file):
        self.__DEVSMap = json.load(file)

        json_keys = self.__DEVSMap.keys()

        # Extract model name
        model_name = [name for name in json_keys if name not in fixed_keys]
        if(len(model_name) != 1):
            print("ERROR: Multiple model names")
            exit(-1)
        self.__model_name = model_name[0]

        #Extract list of parameters
        try:
            self.__params = self.__DEVSMap['parameters']
        except:
            print("WARNING: Model has no parameter key.")
            self._params = None

        #Extract list of included sets
        try:
            self.__sets = self.__DEVSMap['include_sets']
        except:
            print("ERROR: Model has no include_sets key.")
            exit(-1)

    
    def get_model_name(self):
        return self.__model_name
    
    def get_params(self):
        return self.__params
    
    def get_sets(self):
        return self.__sets