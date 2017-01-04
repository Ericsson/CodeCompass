require([
  'dojo/topic',
  'dojo/_base/declare',
  'dijit/layout/ContentPane',
  'dijit/layout/TabContainer',
  'dojox/image/Lightbox',
  'codecompass/viewHandler'],
function (topic, declare, ContentPane, TabContainer, Lightbox, viewHandler) {

  var InfoPage = declare(TabContainer, {
    constructor : function () {
      var that = this;

      topic.subscribe('codecompass/infopage', function (page) {
        topic.publish('codecompass/setCenterModule', 'infopage');
        that.selectPage(page);
      });

      this._startPage = new ContentPane({
        id    : 'startpage',
        href  : 'startpage.html',
        title : 'Welcome',
        style : 'padding: 10px'
      });

      this._credits = new ContentPane({
        id    : 'credits',
        href  : 'credits.html',
        title : 'Credits',
        style : 'padding: 10px'
      });

      this._userguide = new ContentPane({
        id    : 'userguide',
        href  : 'userguide/userguide.html',
        title : 'User Guide'
      });
    },

    postCreate : function () {
      this.addChild(this._startPage);
      this.addChild(this._credits);
      this.addChild(this._userguide);
    },

    startup : function(){
      this.inherited(arguments);

      var that = this;

      viewHandler.getModules({
        type : viewHandler.moduleType.InfoPage
      }).forEach(function (item) {
        that.addChild(item);
      });
    },
    selectPage : function (pageId) {
      this.selectChild(pageId);
    },
  });

  var infoPage = new InfoPage({
    id : 'infopage'
  });

  viewHandler.registerModule(infoPage, {
    type : viewHandler.moduleType.Center
  });
});
