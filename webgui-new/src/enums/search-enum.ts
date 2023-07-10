/* eslint-disable no-unused-vars */
import i18n from 'i18n/i18n';

export const SearchMethods = {
  EXPRESSION: i18n.t('searchSettings.methods.expression'),
  FILE_REGEX: i18n.t('searchSettings.methods.fileRegex'),
  PATH_REGEX: i18n.t('searchSettings.methods.pathRegex'),
};

export const SearchOptions = {
  TEXT: i18n.t('searchSettings.options.textOption'),
  DEFINITION: i18n.t('searchSettings.options.definitionOption'),
  FILE_NAME: i18n.t('searchSettings.options.fileNameOption'),
  LOG: i18n.t('searchSettings.options.logOption'),
};

export enum SearchMainLanguages {
  C_CPP = 'C/C++',
  JAVA = 'Java',
  JAVASCRIPT = 'JavaScript',
  SHELLSCRIPT = 'ShellScript',
  PEARL = 'Pearl',
  PYTHON = 'Python',
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

export const SearchTypeOptions = {
  TYPE: i18n.t('searchSettings.types.type'),
  FUNCTION: i18n.t('searchSettings.types.function'),
  CONSTANT: i18n.t('searchSettings.types.constant'),
  VARIABLE: i18n.t('searchSettings.types.variable'),
  FIELD: i18n.t('searchSettings.types.field'),
  LABEL: i18n.t('searchSettings.types.label'),
  MACRO: i18n.t('searchSettings.types.macro'),
  MODULE: i18n.t('searchSettings.types.module'),
};
