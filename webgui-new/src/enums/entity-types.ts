import i18n from 'i18n/i18n';

const referenceTypeArray = [
  i18n.t('referenceTypes.DEFINITION'),
  i18n.t('referenceTypes.DECLARATION'),
  i18n.t('referenceTypes.USAGE'),
  i18n.t('referenceTypes.THIS_CALLS'),
  i18n.t('referenceTypes.CALLS_OF_THIS'),
  i18n.t('referenceTypes.CALLEE'),
  i18n.t('referenceTypes.CALLER'),
  i18n.t('referenceTypes.VIRTUAL_CALL'),
  i18n.t('referenceTypes.FUNC_PTR_CALL'),
  i18n.t('referenceTypes.PARAMETER'),
  i18n.t('referenceTypes.LOCAL_VAR'),
  i18n.t('referenceTypes.RETURN_TYPE'),
  i18n.t('referenceTypes.OVERRIDE'),
  i18n.t('referenceTypes.OVERRIDDEN_BY'),
  i18n.t('referenceTypes.READ'),
  i18n.t('referenceTypes.WRITE'),
  i18n.t('referenceTypes.TYPE'),
  i18n.t('referenceTypes.ALIAS'),
  i18n.t('referenceTypes.INHERIT_FROM'),
  i18n.t('referenceTypes.INHERIT_BY'),
  i18n.t('referenceTypes.DATA_MEMBER'),
  i18n.t('referenceTypes.METHOD'),
  i18n.t('referenceTypes.FRIEND'),
  i18n.t('referenceTypes.UNDERLYING_TYPE'),
  i18n.t('referenceTypes.ENUM_CONSTANTS'),
  i18n.t('referenceTypes.EXPANSION'),
  i18n.t('referenceTypes.UNDEFINITION'),
];

const fileReferenceTypeArray = [
  i18n.t('fileReferenceTypes.INCLUDES'),
  i18n.t('fileReferenceTypes.TYPES'),
  i18n.t('fileReferenceTypes.FUNCTIONS'),
  i18n.t('fileReferenceTypes.MACROS'),
];

const diagramTypeArray = [
  i18n.t('diagramTypes.FUNCTION_CALL'),
  i18n.t('diagramTypes.DETAILED_CLASS'),
  i18n.t('diagramTypes.CLASS_OVERVIEW'),
  i18n.t('diagramTypes.CLASS_COLLABORATION'),
  i18n.t('diagramTypes.COMPONENT_USERS'),
  i18n.t('diagramTypes.EXTERNAL_DEPENDENCY'),
  i18n.t('diagramTypes.EXTERNAL_USERS'),
  i18n.t('diagramTypes.INCLUDE_DEPENDENCY'),
  i18n.t('diagramTypes.INTERFACE'),
  i18n.t('diagramTypes.SUBSYSTEM_DEPENDENCY'),
];

export { referenceTypeArray, fileReferenceTypeArray, diagramTypeArray };
