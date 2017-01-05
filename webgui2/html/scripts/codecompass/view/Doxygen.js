define([
  'dojo/dom-attr',
  'dojo/dom-construct',
  'dojo/_base/declare',
  'dojo/topic',
  'dijit/layout/ContentPane',
  'codecompass/urlHandler'],
function (attr, dom, declare, topic, ContentPane, urlHandler) {
  return declare(ContentPane, {
    style : 'overflow: hidden',
    
    constructor : function () {
      var that = this;
      
      this.content = dom.create('iframe', {
        style : 'width: 100%; height: 100%; border: none'
      });
      
      topic.subscribe('codecompass/doxygen', function (message) {
        var wsid = urlHandler.getStateValue('wsid');
        attr.set(that.content, 'src', 'doxygen/' + wsid + '/' + message.path);
      });
    }
  });
});