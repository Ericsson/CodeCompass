define([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util) {

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
    var fileGroupsId = [];

    var references = model.cppservice.getReferences(
      parentNode.nodeInfo.id,
      parentNode.refType);

    references.forEach(function (reference) {
      if (parentNode.refType === refTypes['Caller'] ||
          parentNode.refType === refTypes['Usage']) {

        //--- Group nodes by file name ---//

        var fileId = reference.range.file;
        if (fileGroupsId[fileId])
          return;

        fileGroupsId[fileId] = parentNode.refType + fileId + reference.id;

        var referenceInFile = references.filter(function (reference) {
          return reference.range.file === fileId;
        });

        var fileInfo = model.project.getFileInfo(fileId);
        res.push({
          id          : fileGroupsId[fileId],
          name        : fileInfo.name + ' (' + referenceInFile.length + ')',
          refType     : parentNode.refType,
          nodeInfo    : reference,
          hasChildren : true,
          cssClass    : util.getIconClass(fileInfo.path),
          getChildren : function () {
            var that = this;
            var res = [];

            referenceInFile.forEach(function (reference) {
              if (parentNode.refType === refTypes['Caller']) {
                res.push({
                  id          : reference.id,
                  name        : createLabel(reference),
                  nodeInfo    : reference,
                  refType     : parentNode.refType,
                  cssClass    : 'icon icon-Method',
                  hasChildren : true,
                  getChildren : function () {
                    var res = [];

                    //--- Recursive Node ---//

                    res.push({
                      id          : 'Caller-' + reference.id,
                      name        : parentNode.name,
                      nodeInfo    : reference,
                      refType     : parentNode.refType,
                      cssClass    : parentNode.cssClass,
                      hasChildren : true,
                      getChildren : parentNode.getChildren
                    });

                    //--- Call ---//

                    var calls = model.cppservice.getReferences(
                      this.nodeInfo.id,
                      refTypes['This calls']);

                    calls.forEach(function (call) {
                      if (call.mangledNameHash ===
                          parentNode.nodeInfo.mangledNameHash)
                        res.push({
                          name        : createLabel(call),
                          refType     : parentNode.refType,
                          nodeInfo    : call,
                          hasChildren : false,
                          cssClass    : getCssClass(call)
                        });
                    });
                    return res;
                  }
                });
              } else if (parentNode.refType === refTypes['Usage']) {
                res.push({
                  id          : fileGroupsId[fileId] + reference.id,
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

  /**
   * This function returns file references children.
   * @param parentNode Reference type node in Info Tree.
   */
  function loadFileReferenceNodes(parentNode) {
    var res = [];

    var references = model.cppservice.getFileReferences(
      parentNode.nodeInfo.id,
      parentNode.refType);

    references.forEach(function (reference) {
      res.push({
        name        : createLabel(reference),
        refType     : parentNode.refType,
        nodeInfo    : reference,
        hasChildren : false,
        cssClass    : getCssClass(reference)
      });
    });

    return res;
  }

  var cppInfoTree = {
    render : function (elementInfo) {
      var ret = [];

      if(elementInfo instanceof AstNodeInfo) {
        //--- Properties ---//

        var props = model.cppservice.getProperties(elementInfo.id);
        for (var propName in props) {
          var propId = propName.replace(/ /g, '-');
          var label
            = '<span class="label">' + propName + '</span>: '
            + '<span class="value">' + props[propName] + '</span>';

          ret.push({
            name        : label,
            nodeInfo    : elementInfo,
            cssClass    : 'icon-' + propId,
            hasChildren : false
          });
        }

        //--- References ---//

        var refTypes = model.cppservice.getReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          ret.push({
            name        : refType,
            nodeInfo    : elementInfo,
            refType     : refTypes[refType],
            cssClass    : 'icon-' + refType.replace(/ /g, '-'),
            hasChildren : true,
            getChildren : function () {
              return loadReferenceNodes(this, refTypes);
            }
          });
        };

      } else if (elementInfo instanceof FileInfo) {

        //--- File references ---//

        var refTypes = model.cppservice.getFileReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          ret.push({
            name        : refType,
            nodeInfo    : elementInfo,
            refType     : refTypes[refType],
            cssClass    : 'icon-' + refType.replace(/ /g, '-'),
            hasChildren : true,
            getChildren : function () {
              return loadFileReferenceNodes(this);
            }
          });
        };

      }

      return ret;
    }
  };

  viewHandler.registerModule(cppInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.cppservice
  });
});
