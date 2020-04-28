define([
  'dojo/_base/declare',
  'dojo/dom-attr',
  'dojo/dom-class',
  'dojo/dom-construct',
  'dijit/layout/ContentPane'],
function (declare, domAttr, domClass, domConstruct, ContentPane) {

  return declare(ContentPane, {
    show: function (heading, errorMsg) {
      domClass.remove(this.containerNode, 'hide');

      domAttr.set(this._heading, 'innerHTML', heading);
      domAttr.set(this._errorText, 'innerHTML', errorMsg);
    },

    hide: function () {
      domClass.add(this.containerNode, 'hide');
    },

    postCreate: function () {
      this.inherited(arguments);

      this._heading = domConstruct.create('span', {
        class: 'heading'
      }, this.containerNode);

      this._errorText = domConstruct.create('span', null, this.containerNode);
    }
  });
});
