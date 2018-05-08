require([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util) {

  function getChildrenTreeElements(parent, fileId) {
    if (!parent.hasChildren)
      return [];

    var astDetail = model.cppreparseservice.getDetail(
      fileId, parent.id);

    var ret = [];

    // Create the node for the child node's detail.
    ret.push({
      name        : astDetail.otherStuff,
      cssClass    : 'icon icon-info',
      hasChildren : false
    });

    astDetail.children.forEach(function (child) {
      ret.push({
        id          : child.visitId,
        name        : child.type,
        parent      : parent.visitId,
        hasChildren : true,
        getChildren : function () {
          return getChildrenTreeElements(this, fileId);
        }
      });
    });

    return ret;
  }

  var cppReparseInfoTree = {
    id : 'cppreparse-ast',
    render : function (elementInfo) {
      var ret = [];

      var fileId;
      var astBasic;

      if (elementInfo instanceof AstNodeInfo) {
        // The root element will be expanded to the first level of children.
        fileId = elementInfo.range.file;
        astBasic = model.cppreparseservice.getBasicForNode(
          elementInfo.id);
      } else if (elementInfo instanceof FileInfo) {
        fileId = elementInfo.id;
        astBasic = model.cppreparseservice.getBasic(fileId);
      } else {
        console.error("Invalid element", elementInfo);
        return;
      }

      var astDetail = model.cppreparseservice.getDetail(
        fileId, astBasic.visitId);

      // Create the root node for the AST.
      ret.push({
        id : 'root',
        name : astBasic.type,
        hasChildren : astBasic.hasChildren
      });

      // Create the node for the root node's detail.
      ret.push({
        name        : astDetail.otherStuff,
        parent      : 'root',
        cssClass    : 'icon icon-info',
        hasChildren : false
      });

      // Add the basic node for the children, if any.
      astDetail.children.forEach(function (child) {
        ret.push({
          id          : child.visitId,
          name        : child.type,
          parent      : 'root',
          hasChildren : true,
          getChildren : function () {
            return getChildrenTreeElements(this, fileId);
          }
        });
      });

      return ret;
    }
  };

  viewHandler.registerModule(cppReparseInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    fileType : 'cpp-reparse'
  });
});
