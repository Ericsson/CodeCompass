define([
  'dojo/dom',
  'dojo/_base/declare',
  'dijit/layout/ContentPane',
  'dijit/layout/TabContainer',
  'dojox/image/Lightbox'],
function (dom, declare, ContentPane, TabContainer, Lightbox) {
    
  return declare(TabContainer, {
    constructor : function () {
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

      this._settings = new ContentPane({
        id    : 'settings',
        href  : 'settings.html',
        title : 'Settings',
        style : 'padding: 10px',
        onDownloadEnd : function(){
          if(dojo.byId('select-file-manager') !== undefined){
            var fileManager = dijit.byId('select-file-manager').get('value');
            dijit.byId(fileManager).set('checked', true);
          }
        }
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
      this.addChild(this._settings);
      this.addChild(this._userguide);
    },
    
    onLoad : function (center, state) {
      setTimeout(function () {
        var revisionNo = dom.byId('revisionNo');
        if (revisionNo)
          revisionNo.innerHTML = CC_GIT_VERSION;
      }, 1000);
    },
    
    selectPage : function (pageId) {
      this.selectChild(pageId);
    }
  });
});
