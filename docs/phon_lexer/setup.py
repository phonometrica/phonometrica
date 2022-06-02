# See: http://www.iamjonas.me/2013/03/custom-syntax-in-pygments.html
# Install the lexer as follows:
# sudo python3 setup.py develop

from setuptools import setup, find_packages

entry_points = """
[pygments.lexers]
phon = phon.lexer:PhonLexer
"""

setup(
    name         = 'phonometrica',
    version      = '0.3',
    description  = __doc__,
    author       = "Julien Eychenne",
    packages     = ['phon'],
    entry_points = entry_points
) 
