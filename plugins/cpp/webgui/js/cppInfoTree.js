define([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util) {

  model.addService('cppservice', 'CppService', LanguageServiceClient);

  function createTagLabels(tags) {
    var label = '';

    if (!tags)
      return label;

    if (tags.indexOf('static') > -1)
      label += '<span class="tag tag-static" title="Static">S</span>';
    if (tags.indexOf('constructor') > -1)
      label += '<span class="tag tag-constructor" title="Constructor">C</span>';
    if (tags.indexOf('destructor') > -1)
      label += '<span class="tag tag-destructor" title="Destructor">D</span>';
    if (tags.indexOf('implicit') > -1)
      label += '<span class="tag tag-implicit" title="Implicit">I</span>';
    if (tags.indexOf('inherited') > -1)
      label += '<span class="tag tag-inherited" title="Inherited">I</span>';
    if (tags.indexOf('virtual') > -1)
      label += '<span class="tag tag-virtual" title="Virtual">V</span>';
    if (tags.indexOf('global') > -1)
      label += '<span class="tag tag-global" title="Global">G</span>';

    return label;
  }

  function createReferenceCountLabel(label, count) {
    return label + '<span class="reference-count">(' + count + ')</span>';
  }

  function createLabel(astNodeInfo, props) {
    var labelClass = '';

    if (astNodeInfo.tags.indexOf('implicit') > -1)
      labelClass = 'label-implicit';

    var labelValue = astNodeInfo.astNodeValue;
    if (astNodeInfo.symbolType === 'Function' && props)
    {
      // TODO: This "if" won't be necessary when the parser is fixed. Currently
      // no signature is generated for implicit functions.
      if (props['Signature'])
        labelValue = props['Signature'];
    }

    var label = createTagLabels(astNodeInfo.tags)
      + '<span class="' + labelClass + '">'
      + astNodeInfo.range.range.startpos.line   + ':'
      + astNodeInfo.range.range.startpos.column + ': '
      + labelValue
      + '</span>';

    return label;
  }

  function getCssClass(astNodeInfo) {
    var tags = astNodeInfo.tags;

    return tags.indexOf('public')    > -1 ? 'icon-visibility icon-public'  :
           tags.indexOf('private')   > -1 ? 'icon-visibility icon-private' :
           tags.indexOf('protected') > -1 ? 'icon-visibility icon-protected' :
           null;
  }

  function loadReferenceNodes(parentNode, nodeInfo, refTypes) {
    var res = [];
    var fileGroupsId = [];

    var references = model.cppservice.getReferences(
      nodeInfo.id,
      parentNode.refType);

    // For functions we get the properties because for these nodes we show their
    // signatures which can be gathered from properties.
    if (references[0] && references[0].symbolType === 'Function')
      var nodesProps = model.cppservice.getProperties(
        references.map(function (ref) { return ref.id; }));

    references.forEach(function (reference) {
      var props = nodesProps ? nodesProps[reference.id] : null;

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
              if (parentNode.refType === refTypes['Caller']) {
                res.push({
                  id          : reference.id,
                  name        : createLabel(reference, props),
                  nodeInfo    : reference,
                  refType     : parentNode.refType,
                  cssClass    : 'icon icon-Method',
                  hasChildren : true,
                  getChildren : function () {
                    var res = [];

                    //--- Recursive Node ---//

                    var refCount = model.cppservice.getReferenceCount(
                      reference.id, parentNode.refType);

                    if (refCount)
                      res.push({
                        id          : 'Caller-' + reference.id,
                        name        : createReferenceCountLabel(
                                        parentNode.name, refCount),
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
                          nodeInfo.mangledNameHash)
                        res.push({
                          name        : createLabel(call, props),
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
                  name        : createLabel(reference, props),
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
          name        : createLabel(reference, props),
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

    // For functions we get the properties because for these nodes we show their
    // signatures which can be gathered from properties.
    if (references[0] && references[0].symbolType === 'Function')
      var nodesProps = model.cppservice.getProperties(
        references.map(function (ref) { return ref.id; }));

    references.forEach(function (reference) {
      var props = nodesProps ? nodesProps[reference.id] : null;

      res.push({
        name        : createLabel(reference, props),
        refType     : parentNode.refType,
        nodeInfo    : reference,
        hasChildren : false,
        cssClass    : getCssClass(reference)
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

    var label = createTagLabels(elementInfo.tags)
      + '<span class="root label">'
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

  var cppInfoTree = {
    render : function (elementInfo) {
      var ret = [];

      ret.push(createRootNode(elementInfo));

      if (elementInfo instanceof AstNodeInfo) {
        //--- Properties ---//

        var nodesProps = model.cppservice.getProperties([elementInfo.id]);
        var props = nodesProps[elementInfo.id];

        for (var propName in props) {
          var propId = propName.replace(/ /g, '-');
          var label
            = '<span class="label">' + propName + '</span>: '
            + '<span class="value">' + props[propName] + '</span>';

          ret.push({
            name        : label,
            parent      : 'root',
            nodeInfo    : elementInfo,
            cssClass    : 'icon-' + propId,
            hasChildren : false
          });
        }

        //--- References ---//

        var refTypes = model.cppservice.getReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          var refCount =
            model.cppservice.getReferenceCount(elementInfo.id, refTypes[refType]);

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

        var refTypes = model.cppservice.getFileReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          var refCount = model.cppservice.getFileReferenceCount(
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

  viewHandler.registerModule(cppInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.cppservice
  });
});
