require([
  'dijit/tree/ObjectStoreModel',
  'dijit/TitlePane',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/mouse',
  'dojo/topic',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/view/component/HtmlTree'],
function (ObjectStoreModel, TitlePane, declare, Memory, Observable, mouse,
  topic, model, viewHandler, HtmlTree) {

  var InfoTree = declare(HtmlTree, {
    constructor : function () {
      var that = this;
 
      //--- Store and model ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          return node.getChildren ? node.getChildren() : [];
        }
      }));

      var dataModel = new ObjectStoreModel({
        store : that._store,
        query : { id : 'root' },
        mayHaveChildren : function (node) {
          return node.hasChildren;
        }
      });

      //--- Tree ---//

      this._rootNode = {
        id          : 'root',
        name        : 'Info Tree',
        cssClass    : 'icon-info',
        hasChildren : true,
        getChildren : function () {
          return that._store.query({ parent : 'root' });
        }
      };

      this._store.put(this._rootNode);

      this.set('model', dataModel);
      this.set('openOnClick', false);
    },

    /**
     * This function loads the first level of the info tree based on the given
     * info object.
     * @param {AstNodeInfo | FileInfo} elementInfo Thrift object.
     */
    loadInfoTree : function (elementInfo) {
      var that = this;

      //--- Remove previous tree data ---//

      this._store.query().forEach(function (node) {
        that._store.remove(node.id);
      });

      //--- Update root node label ---//

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

      this._rootNode.name = rootLabel + ': ' + rootValue;
      this._store.put(this._rootNode);

      //--- Set first level nodes ---//

      // TODO: Get the module according to the file type.
      viewHandler.getModules({ 
        type : viewHandler.moduleType.InfoTree
      }).forEach(function (item) {
        item.render(elementInfo).forEach(function (infoNode) {
          infoNode.parent = 'root';
          that._store.put(infoNode);
        });
      });
    },

    onClick : function (item, node, event) {
      if (item.nodeInfo) {
        var astNodeInfo = item.nodeInfo;
        var range = astNodeInfo.range.range;

        topic.publish('codecompass/openFile', {
          fileId     : astNodeInfo.range.file,
          line       : astNodeInfo.range.range.startpos.line,
          selection  : [
            range.startpos.line,
            range.startpos.column,
            range.endpos.line,
            range.endpos.column]
        });
      }

      if (item.hasChildren)
        this._onExpandoClick({node: node});
    },

    getIconClass : function (item, opened) {
      var baseClass = this.inherited(arguments);

      if (item.cssClass)
        return 'icon ' + item.cssClass;

      if (baseClass === 'dijitLeaf')
        return 'icon icon-node';

      return baseClass;
    }
  });

  var infoTree = new InfoTree({
    id    : 'infotree',
    title : 'Info Tree'
  });

  viewHandler.registerModule(infoTree, {
    type     : viewHandler.moduleType.Accordion,
    priority : 30
  });

  topic.subscribe('codecompass/infotree', function (message) {
    var infoTree = viewHandler.getModule('infotree');
    infoTree.loadInfoTree(message.elementInfo, message.fileType);
    topic.publish('codecompass/selectAccordion', 'infotree');
  });

  var fileOutline = new TitlePane({
    title   : 'File Outline',
    class   : 'fileOutline',
    open    : false,
    content : new InfoTree({
      id    : 'fileoutline',
      style : { overflow : 'auto', maxHeight : '300px' }
    }),

    /**
     * This function populates file outline module if the file ID to be set
     * differs from the previously loaded one.
     * @param {FileId} fileId FileId thrift object. If not set then file id is
     * read from the url.
     */
    render : function (fileInfo) {
      if (!fileInfo)
        return;

      if (fileInfo.id === this.prevFileId)
        return this;

      this.prevFileId = fileInfo.id;

      try {
        this.content.loadInfoTree(fileInfo);
      } catch (ex) {
        console.warn('Exception trown by server', ex);
      }

      return this;
    }
  });

  viewHandler.registerModule(fileOutline, {
    type     : viewHandler.moduleType.ContextButton,
    priority : 10
  });
});
