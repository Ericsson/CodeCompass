require([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util){
  model.addService('pythonservice', 'PythonService', LanguageServiceClient);

  function createLabel(astNodeInfo) {
    return '<span class="">'
        + astNodeInfo.range.range.startpos.line + ':'
        + astNodeInfo.range.range.startpos.column + ': '
        + astNodeInfo.astNodeValue
        + '</span>';
  }

  function getCssClass(astNodeInfo) {
    var tags = astNodeInfo.tags;

    return  tags.indexOf('public')      > -1 ? 'icon-visibility icon-public'  :
            tags.indexOf('private')     > -1 ? 'icon-visibility icon-private' :
            tags.indexOf('semiprivate') > -1 ? 'icon-visibility icon-protected' :
            null;
  }

  function groupReferencesByVisibilities(references, parentNode, nodeInfo){
    var res = [];
    var visibilities = ['public', 'semiprivate', 'private']

    visibilities.forEach(function (visibility){
      var nodes = references.filter(function (reference){
        return reference.tags.indexOf(visibility) > -1;
      });

      if (nodes.length === 0){
        return;
      }

      var visibility_icon_name = visibility === 'semiprivate' ? 'protected' : visibility;

      res.push({
        id: nodeInfo.id + visibility + parentNode.refType,
        name: createReferenceCountLabel(visibility, nodes.length),
        refType: parentNode.refType,
        hasChildren: true,
        cssClass: 'icon-visibility icon-' + visibility_icon_name,
        getChildren: function (){
          var res = [];
          nodes.forEach(function (reference){
            res.push({
              id: visibility + reference.id,
              name: createLabel(reference),
              refType: parentNode.refType,
              nodeInfo: reference,
              hasChildren: false,
              cssClass: getCssClass(reference)
            });
          });
          return res;
        }
      });
    });

    return res;
  }

  function createRootNode(elementInfo) {
    var rootLabel
        = '<span class="root label">'
        + (elementInfo instanceof AstNodeInfo
            ? elementInfo.symbolType
            : 'File')
        + '</span>';

    var rootValue
        = '<span class="root value">'
        + (elementInfo instanceof AstNodeInfo
            ? elementInfo.astNodeValue
            : elementInfo.name)
        + '</span>';

    var label = '<span class="root label">'
        + rootLabel + ': ' + rootValue
        + '</span>';

    return {
      id          : 'root',
      name        : label,
      cssClass    : 'icon-info',
      hasChildren : true,
      getChildren : function () {
        return this._store.query({ parent : 'root' });
      }
    };
  }

  function createReferenceCountLabel(label, count) {
    return label + '<span class="reference-count">(' + count + ')</span>';
  }

  function loadReferenceNodes(parentNode, nodeInfo, refTypes) {
    var res = [];
    var fileGroupsId = [];

    var references = model.pythonservice.getReferences(nodeInfo.id, parentNode.refType);

    if (parentNode.refType === refTypes['Method'] ||
        parentNode.refType === refTypes['Data member']){
      return groupReferencesByVisibilities(references, parentNode, nodeInfo);
    }

    references.forEach(function (reference) {
      if (parentNode.refType === refTypes['Caller'] ||
          parentNode.refType === refTypes['Usage']) {
        var fileId = reference.range.file;
        if (fileGroupsId[fileId]){
          return;
        }

        fileGroupsId[fileId] = parentNode.refType + fileId + reference.id;

        var referenceInFile = references.filter(function (ref) {
          return reference.range.file === fileId;
        });

        var fileInfo = model.project.getFileInfo(fileId);
        res.push({
          id: fileGroupsId[fileId],
          name: createReferenceCountLabel(fileInfo.name, referenceInFile.length),
          refType: parentNode.refType,
          hasChildren: true,
          cssClass: util.getIconClass(fileInfo.path),
          getChildren: function () {
            var that = this;
            var res = [];

            referenceInFile.forEach(function (ref) {
              if (parentNode.refType === refTypes['Caller']) {
                ref.push({
                  id: ref.id,
                  name: createLabel(ref),
                  nodeInfo: ref,
                  refType: parentNode.refType,
                  cssClass: 'icon icon-Method',
                  hasChildren: true,
                  getChildren: function () {
                    var res = [];

                    var refCount = model.pythonservice.getReferenceCount(ref.id, parentNode.refType);

                    if (refCount > 0) {
                      res.push({
                        id: 'Caller-' + ref.id,
                        name: createReferenceCountLabel(parentNode.name, refCount),
                        nodeInfo: ref,
                        refType: parentNode.refType,
                        cssClass: parentNode.cssClass,
                        hasChildren: true,
                        getChildren: parentNode.getChildren
                      });
                    }

                    var calls = model.pythonservice.getReferences(this.nodeInfo.id, refTypes['This calls']);

                    calls.forEach(function (call) {
                      if (call.entityHash === nodeInfo.entityHash)
                        res.push({
                          name: createLabel(call),
                          refType: parentNode.refType,
                          nodeInfo: call,
                          hasChildren: false,
                          cssClass: getCssClass(call)
                        });
                    });

                    return res;
                  }
                });
              } else if (parentNode.refType === refTypes['Usage']) {
                res.push({
                  id: fileGroupsId[fileId] + ref.id,
                  name: createLabel(ref),
                  refType: parentNode.refType,
                  nodeInfo: ref,
                  hasChildren: false,
                  cssClass: getCssClass(ref)
                });
              }
            });
            return res;
          }
        });
      } else {
        res.push({
          name: createLabel(reference),
          refType: parentNode.refType,
          nodeInfo: reference,
          hasChildren: false,
          cssClass: getCssClass(reference)
        });
      }
    });

    return res;
  }

  var pythonInfoTree = {
    id: 'python-info-tree',
    render : function (elementInfo){
      var ret = [];

      ret.push(createRootNode(elementInfo));

      if (elementInfo instanceof AstNodeInfo){
        var properties = model.pythonservice.getProperties(elementInfo.id);

        for (var propertyName in properties) {
          var propertyId = propertyName.replace(/ /g, '-');
          var label =
              '<span class="label">' + propertyName + '</span>: ' +
              '<span class="value">' + properties[propertyName] +'</span>';

          ret.push({
            name: label,
            parent: 'root',
            nodeInfo: elementInfo,
            cssClass: 'icon-' + propertyId,
            hasChildren: false
          });
        }

        var refTypes = model.pythonservice.getReferenceTypes(elementInfo.id);

        for (var refType in refTypes) {
          var refCount = model.pythonservice.getReferenceCount(elementInfo.id, refTypes[refType]);

          if (refCount > 0){
            ret.push({
              name: createReferenceCountLabel(refType, refCount),
              parent: 'root',
              refType: refTypes[refType],
              cssClass: 'icon-' + refType.replace(/ /g, '-'),
              hasChildren: true,
              getChildren: function () {
                return loadReferenceNodes(this, elementInfo, refTypes);
              }
            });
          }
        }
      } else if (elementInfo instanceof FileInfo) {

      }

      return ret;
    }
  };

  viewHandler.registerModule(pythonInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.pythonservice
  });
});
