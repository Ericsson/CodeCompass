define([
  'dojo/_base/declare',
  'dijit/Tooltip'],
function (declare, Tooltip) {
  return declare(null, {
    _onNodeMouseEnter : function (node, evt) {
      Tooltip.show(node.item.name, node.labelNode, ['above']);
    },

    _onNodeMouseLeave : function (node, evt) {
      Tooltip.hide(node.labelNode);
    }
  });
});
