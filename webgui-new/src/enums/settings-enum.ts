export enum SearchOptions {
  TEXT = 'Text search',
  DEFINITION = 'Definition search',
  FILE_NAME = 'File name search',
  LOG = 'Log search',
}

export enum SearchMethods {
  EXPRESSION = 'Search by expression, "like foo AND bar"',
  FILE_REGEX = 'File name filter regex (*.cpp)',
  PATH_REGEX = 'Path filter regex (click on a dir below)',
}

export enum SearchMainLanguages {
  C_CPP = 'C/C++',
  JAVA = 'Java',
  JAVASCRIPT = 'JavaScript',
  SHELLSCRIPT = 'ShellScript',
  PEARL = 'Pearl',
  PYTHON = 'Python',
  ANY = 'Any',
}

export enum SearchOtherLanguages {
  X_ASM = 'text/x-asm',
  X_BCPL = 'text/x-bcpl',
  X_C = 'text/x-c',
  X_CPP = 'text/x-c++',
  X_AWK = 'text/x-awk',
  X_GAWK = 'text/x-gawk',
  X_PHP = 'text/x-php',
  X_SHELLSCRIPT = 'text/x-shellscript',
  X_FORTRAN = 'text/x-fortran',
  X_PO = 'text/x-po',
  X_JAVA = 'text/x-java',
  X_JAVA_SOURCE = 'text/x-java-source',
  X_XMCD = 'text/x-xmcd',
  X_LISP = 'text/x-lisp',
  X_LUA = 'text/x-lua',
  X_M4 = 'text/x-m4',
  X_MAKEFILE = 'text/x-makefile',
  X_VCARD = 'text/x-vcard',
  X_MSDOS_BATCH = 'text/x-msdos-batch',
  X_PASCAL = 'text/x-pascal',
  X_PERL = 'text/x-perl',
  X_PYTHON = 'text/x-python',
  X_RUBY = 'text/x-ruby',
  X_TCL = 'text/x_tcl',
  X_INFO = 'text/x-info',
  X_TEX = 'text/x-tex',
  X_TEXINFO = 'text/x-texinfo',
}

export enum SearchTypes {
  TYPE = 'Type',
  FUNCTION = 'Function',
  CONSTANT = 'Constant',
  VARIABLE = 'Variable',
  FIELD = 'Field',
  LABEL = 'Label',
  MACRO = 'Macro',
  MODULE = 'Module',
}
