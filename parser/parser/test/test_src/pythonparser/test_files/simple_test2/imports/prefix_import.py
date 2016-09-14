"""This modul is for testing prefixed import names"""

import imports.prefix_imports.prefix.pre.pr as p

p.pr()

from imports.prefix_imports.prefix.pre.pr import pr as r

r()

