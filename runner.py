import simple_parser

file = open('counter.json')
parser = simple_parser.Parser(file)
print(parser.get_model_name())