define([
  'codecompass/model',
  'codecompass/viewHandler'],
function (model, viewHandler) {

  model.addService('cppservice', 'CppService', LanguageServiceClient);

  function createLabel(astNodeInfo) {
    var tags = astNodeInfo.tags;

    var label = '';
    if (tags.indexOf('static') > -1)
      label += '<span class="tag tag-static">S</span>';
    if (tags.indexOf('constructor') > -1)
      label += '<span class="tag tag-constructor">C</span>';
    if (tags.indexOf('generated') > -1)
      label += '<span class="tag tag-generated">G</span>';
    if (tags.indexOf('inherited') > -1)
      label += '<span class="tag tag-inherited">I</span>';
    if (tags.indexOf('virtual') > -1)
      label += '<span class="tag tag-virtual">V</span>';

    label
      += astNodeInfo.range.range.startpos.line   + ':'
      +  astNodeInfo.range.range.startpos.column + ': '
      +  astNodeInfo.astNodeValue;

    return label;
  }

  function getCssClass(astNodeInfo) {
    var tags = astNodeInfo.tags;

    return tags.indexOf('public')    > -1 ? 'icon-plus'  :
           tags.indexOf('private')   > -1 ? 'icon-minus' :
           tags.indexOf('protected') > -1 ? 'icon-zero'  : null;
  }

  function loadReferenceNodes(parentNode, refTypes) {
    var res = [];

    var references = model.cppservice.getReferences(
      parentNode.nodeInfo.id,
      parentNode.refType);

    references.forEach(function (reference) {
      if (parentNode.refType === refTypes['Caller']) {
        var calls = model.cppservice.getReferences(
          reference.id,
          refTypes['This calls']);

        calls.forEach(function (call) {

          //--- Recursive Node ---//

          res.push({
            name        : parentNode.name,
            nodeInfo    : reference,
            refType     : parentNode.refType,
            cssClass    : parentNode.cssClass,
            hasChildren : true,
            getChildren : parentNode.getChildren
          });

          //--- Call ---//

          res.push({
            name        : createLabel(call),
            refType     : parentNode.refType,
            nodeInfo    : call,
            hasChildren : false,
            cssClass    : getCssClass(call)
          });
        });
      } else {
        res.push({
          name        : createLabel(reference),
          refType     : parentNode.refType,
          nodeInfo    : reference,
          hasChildren : false,
          cssClass    : getCssClass(reference)
        });
      }
    });

    return res;
  }

  var cppInfoTree = {
    render : function (astNodeInfo) {
      var ret = [];

      //--- Properties ---//

      var props = model.cppservice.getProperties(astNodeInfo.id);
      for (var propName in props) {
        var propId = propName.replace(/ /g, '-');
        var label
          = '<span class="label">' + propName + '</span>: '
          + '<span class="value">' + props[propName] + '</span>';

        ret.push({
          name        : label,
          nodeInfo    : astNodeInfo,
          cssClass    : 'icon-' + propId,
          hasChildren : false
        });
      }

      //--- References ---//

      var refTypes = model.cppservice.getReferenceTypes(astNodeInfo.id);
      for (var refType in refTypes) {
        ret.push({
          name        : refType,
          nodeInfo    : astNodeInfo,
          refType     : refTypes[refType],
          cssClass    : 'icon-' + refType.replace(/ /g, '-'),
          hasChildren : true,
          getChildren : function () {
            return loadReferenceNodes(this, refTypes);
          }
        });
      };

      return ret;
    }
  };

  viewHandler.registerModule(cppInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.cppservice
  });
});
