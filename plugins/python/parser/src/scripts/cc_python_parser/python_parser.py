from cc_python_parser.parser import Parser
from cc_python_parser.skip_file import read_skip_file
from cc_python_parser.persistence.persistence import init_persistence, ModelPersistence


def parse(source_root: str, persistence):
    model_persistence = ModelPersistence(persistence)
    p = Parser([source_root], model_persistence, read_skip_file(model_persistence))
    p.parse()


if __name__ == '__main__':
    parse("dummy", None)   # error!
