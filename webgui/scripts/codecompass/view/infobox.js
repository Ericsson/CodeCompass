require([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dijit/layout/ContentPane',
  'dojox/layout/FloatingPane',
  'dijit/layout/TabContainer',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/view/component/Text'],
function (declare, dom, style, topic, ContentPane, FloatingPane, TabContainer,
  model, viewHandler, Text) {

  var InfoBox = declare(FloatingPane, {
    constructor : function () {
      this._subscribeTopics();
      this.width  = window.innerWidth  / 2;
      this.height = window.innerHeight / 2;
      this.posX   = (window.innerWidth  - this.width ) / 2;
      this.posY   = (window.innerHeight - this.height) / 2;
    },

    postCreate : function () {
      this.inherited(arguments);

      this._documentationPane = new ContentPane({
        class : 'documentation',
        title : 'Documentation',
        style : 'padding: 5px'
      });
      this.addChild(this._documentationPane);

      this.startup();
    },

    resize : function () {
      this.inherited(arguments);
      var pos = arguments[0];
      if (pos && pos.w > 0) {
        this.width = pos.w;
        this.height = pos.h;
        this.posX = pos.x;
        this.posY = pos.y;
      }
    },

    show : function () {
      this.resize();
      style.set(this.domNode, {
        width  : this.width + 'px',
        height : this.height + 'px',
        left   : this.posX + 'px',
        top    : this.posY + 'px',
        'min-width': '200px',
        'min-height': '200px',
        'z-index': 10000,
      });
      this.bringToTop();
      this.inherited(arguments);
    },

    close : function () {
      style.set(this.domNode, { display : 'none' });
    },

    _setDocumentationAttr : function (documentation){
      this._documentationPane.set('content', documentation);
      this._documentationPane.resize();
      this.resize();
    },

    _subscribeTopics : function () {
      var that = this;

      topic.subscribe('codecompass/documentation', function (message) {
        var elementInfo = message.elementInfo;
        var service = model.getLanguageService(message.fileType);

        var documentation = service.getDocumentation(elementInfo.id);
        if (!documentation)
          documentation = "No documentation for this node.";

        that.set('documentation', documentation)

        that.show();
      });
    },

    style: "display: none;",
  });

  var infobox = new InfoBox({
    id : 'infobox',
    title: "Documentation",
    resizable: true,
    dockable: false
  }, dom.create("div", null, window.document.body));

  viewHandler.registerModule(infobox, {
    type : viewHandler.moduleType.Other
  });
});
