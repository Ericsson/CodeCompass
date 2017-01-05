"""This module is for testing the resolvation of lib modules written in Python"""


import os
os.getenv("PYTHONPATH")

from base64 import standard_b64encode
standard_b64encode("Tesing")