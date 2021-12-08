require([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util) {

  model.addService('javaservice', 'JavaService', LanguageServiceClient);

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

  function createLabel(astNodeInfo) {
    var labelClass = '';

    if (astNodeInfo.tags.indexOf('implicit') > -1)
      labelClass = 'label-implicit';

    var labelValue = astNodeInfo.astNodeValue;

    // Create dom node for return type of a function and place it at the end of
    // signature.
    if (astNodeInfo.symbolType === 'Function') {
      var init = labelValue.slice(0, labelValue.indexOf('('));
      var returnTypeEnd = init.lastIndexOf(' ');

      //--- Constructor, destructor doesn't have return type ---//

      if (returnTypeEnd !== -1) {
        var funcSignature = init.slice(returnTypeEnd);

        labelValue = funcSignature
          + ' : <span class="label-return-type">'
          + init.slice(0, returnTypeEnd)
          + "</span>";
      }
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
           tags.indexOf('package-private') > -1 ?
               'icon-visibility icon-package-private' :
           null;
  }

  function groupReferencesByVisibilities(references, parentNode, nodeInfo) {
    var res = [];
    var visibilities = ['public', 'private', 'protected', 'package-private'];

    visibilities.forEach(function (visibility) {
      var nodes = references.filter(function (reference) {
        return reference.tags.indexOf(visibility) > -1;
      });

      if (!nodes.length)
        return;

      res.push({
        id          : nodeInfo.id + visibility + parentNode.refType,
        name        : createReferenceCountLabel(visibility, nodes.length),
        refType     : parentNode.refType,
        hasChildren : true,
        cssClass    : 'icon-visibility icon-' + visibility,
        getChildren : function () {
          var res = [];

          nodes.forEach(function (reference) {
            res.push({
              id          : visibility + reference.id,
              name        : createLabel(reference),
              refType     : parentNode.refType,
              nodeInfo    : reference,
              hasChildren : false,
              cssClass    : getCssClass(reference)
            });
          });

          return res;
        }
      });
    });

    return res;
  }

  function loadReferenceNodes(parentNode, nodeInfo, refTypes) {
    var res = [];
    var fileGroupsId = [];

    var references = model.javaservice.getReferences(
      nodeInfo.id,
      parentNode.refType);

    if (parentNode.refType === refTypes['Constructor'] ||
        parentNode.refType === refTypes['Method'] ||
        parentNode.refType === refTypes['Data member'] ||
        parentNode.refType === refTypes['Inner type'])
      return groupReferencesByVisibilities(references, parentNode, nodeInfo);

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

                    var refCount = model.javaservice.getReferenceCount(
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

                    var calls = model.javaservice.getReferences(
                      this.nodeInfo.id,
                      refTypes['This calls']);

                    calls.forEach(function (call) {
                      if (call.entityHash === nodeInfo.entityHash)
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

    var references = model.javaservice.getFileReferences(
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

  var javaInfoTree = {
    id: 'java-infotree',
    render : function (elementInfo) {
      var ret = [];

      ret.push(createRootNode(elementInfo));

      if (elementInfo instanceof AstNodeInfo) {
        //--- Properties ---//

        var props = model.javaservice.getProperties(elementInfo.id);

        for (var propName in props) {
          var propId = propName.replace(/ /g, '-');
          var labelName = propName.replace('<','&lt;').replace('>','&gt;');
          var labelValue =
            props[propName].replace('<','&lt;').replace('>','&gt;');

          var label
            = '<span class="label">' + labelName + '</span>: '
            + '<span class="value">' + labelValue + '</span>';

          ret.push({
            name        : label,
            parent      : 'root',
            nodeInfo    : elementInfo,
            cssClass    : 'icon-' + propId,
            hasChildren : false
          });
        }

        //--- References ---//

        var refTypes = model.javaservice.getReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          var refCount =
            model.javaservice.getReferenceCount(
              elementInfo.id, refTypes[refType]);

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

        var refTypes = model.javaservice.getFileReferenceTypes(elementInfo.id);
        for (var refType in refTypes) {
          var refCount = model.javaservice.getFileReferenceCount(
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

  viewHandler.registerModule(javaInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.javaservice
  });
});
