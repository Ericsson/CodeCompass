define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/dnd/Moveable',
  'dojo/topic',
  'dojox/layout/ResizeHandle',
  'dijit/layout/ContentPane',
  'dijit/layout/TabContainer',
  'codecompass/view/component/Editor'],
function (declare, dom, style, Moveable, topic, ResizeHandle, ContentPane, TabContainer, Editor) {
  return declare(ContentPane, {
    buildRendering : function () {
      this.inherited(arguments);
      
      var that = this;
      
      this._editor = new Editor();
      
      this._codePane = new ContentPane({
        title   : 'Code',
        content : this._editor
      });
      
      this._documentationPane = new ContentPane({
        title : 'Documentation',
        style : 'padding: 5px'
      });
      
      this.content = new TabContainer({
        style : 'height: 100%; overflow: hidden'
      });
      
      this.content.addChild(this._codePane);
      this.content.addChild(this._documentationPane);
      
      // TODO: This is ugly!!!
      setTimeout(function () {
        that.content.selectChild(that._documentationPane);
        that.content.selectChild(that._codePane);
      }, 1000);
    },
    
    postCreate : function () {
      var that = this;
      
      if (that.draggable)
        require(['dojo/query', 'dojo/NodeList-traverse'], function (query) {
          new Moveable(that.domNode, {
            handle : query(that.content.containerNode).parent().children()[0]
          });
        });
      
      if (this.resizable) {
        new ResizeHandle({
          targetContainer : this.domNode,
          animateMethod   : 'combine'
        }).placeAt(this.domNode);

        topic.subscribe('/dojo/resize/stop', function (inst) {
          if (inst.targetDomNode.id !== that.id)
            return;

          that.content.resize();
        });
      }
      
      if (this.closable) {
        var img = dom.create('img', {
          src     : '/images/close.png',
          class   : 'closebtn',
          onclick : function () { style.set(that.domNode, { display : 'none' }); }
        });
        
        dom.place(img, this.domNode);
      }
    },
    
    _setDataAttr : function (infoBox) {
      if (infoBox.information) {
        this._editor.set('content', infoBox.information);
        this._documentationPane.set('content', infoBox.documentation);
        this._documentationPane.set('disabled', !infoBox.documentation);
        
        this.data = infoBox;
      }
    },
    
    _setFileInfoAttr : function (fileInfo) {
      if (fileInfo.path) {
        this._editor.setHeader(fileInfo);
        this.fileInfo = fileInfo;
      }
    },
    
    data      : {}, /*< InfoBox thrift object */
    fileInfo  : {}, /*< FileInfo thrift object */
    draggable : true,
    resizable : true,
    closable  : true,
    class     : 'infobox'
  });
});