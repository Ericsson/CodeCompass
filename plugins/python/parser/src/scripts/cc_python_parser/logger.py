import pathlib
import logging

directory = pathlib.Path(__file__).resolve().parent

logger = logging.getLogger('test')
logger.setLevel(logging.DEBUG)
file_handler = logging.FileHandler(str(directory) + '/test.log', 'w', encoding='utf-8')
file_handler.setLevel(logging.DEBUG)
logger.addHandler(file_handler)

db_logger = logging.getLogger('db_test')
db_logger.setLevel(logging.DEBUG)
db_file_handler = logging.FileHandler(str(directory) + '/db_test.log', 'w', encoding='utf-8')
db_file_handler.setLevel(logging.DEBUG)
db_logger.addHandler(db_file_handler)
