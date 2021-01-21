# -*- coding: utf-8 -*-
"""
    pygments.lexers.phon
    ~~~~~~~~~~~~~~~~~~~~~~

    Lexer for the Phonometrica language.

    :copyright: Copyright 2006-2015 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

from pygments.lexer import RegexLexer, include, words
from pygments.token import Text, Comment, Operator, Keyword, Name, String, \
    Number, Punctuation

__all__ = ['PhonLexer']


class PhonLexer(RegexLexer):
    name = 'Phonometrica'
    aliases = ['phon']
    filenames = ['*.phon']
    mimetypes = ['text/x-phon']

    tokens = {
        'root': [
            (r'[^\S\n]+', Text),
            (r'#.*?\n', Comment.Single),
            (r'[^\S\n]+', Text),
            # Please note that keyword and operator are case insensitive.
            (r'(?i)(true|false|null|undefined)\b', Keyword.Constant),
            (words((
                'and', 'as', 'assert', 'break', 'class', 'continue', 'debug', 'do', 'downto', 'else', 'elsif', 
                'end', 'for', 'foreach', 'function', 'get', 'if',
                'in', 'inherits', 'local', 'method', 'not', 'or', 'option', 'pass', 'print', 'ref', 'repeat', 
                'return', 'step', 'then', 'throw', 'to', 'until', 'var', 'while'), prefix=r'(?i)\b', suffix=r'\b'),
             Keyword.Reserved),
            (r'"\[(([^\]%]|\n)|%(.|\n)|\][^"])*?\]"', String),
            (r'"([^"%\n]|%.)*?"', String),
            (r'\'([^"%\n]|%.)*?\'', String),
            include('numbers'),
            (r"'([^'%]|%'|%%)'", String.Char),
            (r"(//|\\\\|>=|<=|:=|/=|~|/~|[\\?!#%&@|+/\-=>*$<^\[\]])", Operator),
            (r"([{}():;,.])", Punctuation),
            (r'([a-z]\w*)|([A-Z][A-Z0-9_]*[a-z]\w*)', Name),
            (r'([A-Z][A-Z0-9_]*)', Name.Class),
            (r'\n+', Text),
        ],
        'numbers': [
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'0[bB][01]+', Number.Bin),
            (r'0[cC][0-7]+', Number.Oct),
            (r'([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)', Number.Float),
            (r'[0-9]+', Number.Integer),
        ],
    }


def setup(sphinx):
  from sphinx.highlighting import lexers
  sphinx.add_lexer('phon', PhonLexer)
  