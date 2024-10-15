import datetime
from parserconfig import ParserConfig

def log(msg):
    date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"{date} [pythonparser] {msg}{bcolors.ENDC}")

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def log_config(config: ParserConfig):
    if config.debug:
        log(f"{bcolors.WARNING}Parsing in debug mode!")

    if config.type_hint:
        log(f"{bcolors.OKGREEN}Type hint support enabled!")
    else:
        log(f"{bcolors.OKBLUE}Type hint support disabled!")

    if config.stack_trace:
        log(f"{bcolors.WARNING}Stack trace enabled!")

    if not (config.ast):
        log(f"{bcolors.WARNING}All AST modules disabled!")

    if not (config.ast_function_call):
        log(f"{bcolors.WARNING}AST function call parsing disabled!")

    if not (config.ast_import):
        log(f"{bcolors.WARNING}AST import parsing disabled!")

    if not (config.ast_inheritance):
        log(f"{bcolors.WARNING}AST inheritance parsing disabled!")

    if not (config.ast_annotations):
        log(f"{bcolors.WARNING}AST annotation parsing disabled!")

    if not (config.ast_function_signature):
        log(f"{bcolors.WARNING}AST function signature parsing disabled!")
