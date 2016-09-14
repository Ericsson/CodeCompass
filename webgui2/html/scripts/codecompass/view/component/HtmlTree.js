define([
  'dojo/_base/declare',
  'dijit/Tree'],
function (declare, Tree) {
  var HtmlTreeNode = declare(Tree._TreeNode, {
    _setLabelAttr : { node : "labelNode", type : "innerHTML" }
  });
  
  return declare(Tree, {
    _createTreeNode : function (args) {
      return new HtmlTreeNode(args);
    }
  });
});
