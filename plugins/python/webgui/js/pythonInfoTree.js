require([
    'codecompass/model',
    'codecompass/viewHandler',
    'codecompass/util'],
  function (model, viewHandler, util) {
  
    model.addService('pythonservice', 'PythonService', LanguageServiceClient);
  
    function createReferenceCountLabel(label, count) {
      var parsedLabel = $('<div>').append($.parseHTML(label));
      parsedLabel.children('span.reference-count').remove();
      parsedLabel.append('<span class="reference-count">(' + count + ')</span>');
  
      return parsedLabel.html();
    }
  
    function createLabel(astNodeInfo) {
      var labelValue = astNodeInfo.astNodeValue.trim();

      if (labelValue.slice(-1) == ':') labelValue = labelValue.substr(0, labelValue.length - 1);
      if(astNodeInfo.astNodeType && !labelValue.includes(':')) labelValue += ' : <span class="label-return-type">' + astNodeInfo.astNodeType + "</span>";

      var label = '<span>'
        + astNodeInfo.range.range.startpos.line   + ':'
        + astNodeInfo.range.range.startpos.column + ': '
        + labelValue
        + '</span>';
  
      return label;
    }
  
    function loadReferenceNodes(parentNode, nodeInfo, refTypes, scratch) {
      var res = [];
      var fileGroupsId = [];
  
      var references = model.pythonservice.getReferences(
        nodeInfo.id,
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
            name        : createReferenceCountLabel(
                            fileInfo.name, referenceInFile.length),
            refType     : parentNode.refType,
            hasChildren : true,
            cssClass    : util.getIconClass(fileInfo.path),
            getChildren : function () {
              var that = this;
              var res = [];
  
              referenceInFile.forEach(function (reference) {
                if (parentNode.refType === refTypes['Caller'] ||
                    parentNode.refType === refTypes['Usage']) {
                  res.push({
                    id          : fileGroupsId[fileId] + reference.id,
                    name        : createLabel(reference),
                    refType     : parentNode.refType,
                    nodeInfo    : reference,
                    hasChildren : false,
                    cssClass    : null
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
            cssClass    : null
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
  
      var references = model.pythonservice.getFileReferences(
        parentNode.nodeInfo.id,
        parentNode.refType);
  
      references.forEach(function (reference) {
        res.push({
          name        : createLabel(reference),
          refType     : parentNode.refType,
          nodeInfo    : reference,
          hasChildren : false,
          cssClass    : null
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
  
      var label =
        '<span class="root label">'
        + rootLabel + ': ' + rootValue
        + '</span>';
  
      return {
        id          : 'root',
        name        : label,
        cssClass    : 'icon-info',
        hasChildren : true,
        getChildren : function () {
          return that._store.query({ parent : 'root' });
        }
      };
    }
  
    var pythonInfoTree = {
      id: 'python-info-tree',    
      render : function (elementInfo) {
        var ret = [];
  
        ret.push(createRootNode(elementInfo));
  
        if (elementInfo instanceof AstNodeInfo) {
          //--- Properties ---//
  
          var props = model.pythonservice.getProperties(elementInfo.id);
  
          for (var propName in props) {
            var label
              = '<span class="label">' + propName + '</span>: '
              + '<span class="value">' + props[propName] + '</span>';
  
            ret.push({
              name        : label,
              parent      : 'root',
              nodeInfo    : elementInfo,
              cssClass    : 'icon-' + propName.replace(/ /g, '-'),
              hasChildren : false
            });
          }
  
          //--- References ---//
  
          var refTypes = model.pythonservice.getReferenceTypes(elementInfo.id);
          for (var refType in refTypes) {
            var refCount =
              model.pythonservice.getReferenceCount(elementInfo.id, refTypes[refType]);
  
            if (refCount)
              ret.push({
                name        : createReferenceCountLabel(refType, refCount),
                parent      : 'root',
                refType     : refTypes[refType],
                cssClass    : 'icon-' + refType.replace(/ /g, '-'),
                hasChildren : true,
                getChildren : function () {
                  return loadReferenceNodes(this, elementInfo, refTypes);
                }
              });
          };
  
        } else if (elementInfo instanceof FileInfo) {
  
          //--- File references ---//
  
          var refTypes = model.pythonservice.getFileReferenceTypes(elementInfo.id);
          for (var refType in refTypes) {
            var refCount = model.pythonservice.getFileReferenceCount(
              elementInfo.id, refTypes[refType]);
  
            if (refCount)
              ret.push({
                name        : createReferenceCountLabel(refType, refCount),
                parent      : 'root',
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
  
    viewHandler.registerModule(pythonInfoTree, {
      type : viewHandler.moduleType.InfoTree,
      service : model.pythonservice
    });
  });
