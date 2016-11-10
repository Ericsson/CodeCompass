require([
  'dojo/on',
  'dojo/query',
  'dojo/dom-style',
  'dojo/topic',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/NodeList-traverse',
  'dijit/layout/ContentPane',
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'codecompass/util',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/view/component/HtmlTree',
  'codecompass/view/component/ContextMenu',
  ],
function (on, query, style, topic, declare, Memory, Observable,
  NodeListTraverse, ContentPane, Tree, ObjectStoreModel, util, model,
  viewHandler, HtmlTree, ContextMenu) {

  //--- No indent tree ---//

  var HtmlTreeNoIndentNode = declare(Tree._TreeNode, {
    _setLabelAttr : { node : 'labelNode', type : 'innerHTML' },
    _setIndentAttr : 0,
    baseClass : 'project-tree-file-manager'
  });

  var HtmlTreeNoIndent = declare(HtmlTree, {
    _createTreeNode : function (args) {
      return new HtmlTreeNoIndentNode(args);
    }
  });

  /**
   * Common file browser module.
   */
  var FileBrowser = declare(Tree, {
    constructor : function () {
      var that = this;

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          return that.loadChildren(node);
        }
      }));

      this._dataModel = new ObjectStoreModel({
        store : this._store,
        query : { id : 'project' },
        mayHaveChildren : function (node) { return node.fileInfo.isDirectory; }
      });

      this._contextMenu = new ContextMenu();

      this.reloadTree();

      this.set('model', this._dataModel);
      this.set('openOnClick', true);
    },

    /*
     * Called after a widget and its children
     * have been created and added to the page
     */
     startup : function () {
       this.inherited(arguments);

       var that = this;

       //--- Menu ---//
       var contextmenu = new ContextMenu({
         targetNodeIds : [this.id],
         selector      : '.dijitTreeNode'
       });

       on(this, '.dijitTreeNode:contextmenu', function (event) {
         var fileInfo = dijit.byNode(query(event.target)
         .closest('.dijitTreeNode')[0]).item.fileInfo;

         that.buildContextMenu(contextmenu, fileInfo);
       });
     },

     buildContextMenu : function (contextMenu, fileInfo) {
       contextMenu.clear();

       viewHandler.getModules({
         type : viewHandler.moduleType.FileManagerContextMenu,
         fileType : fileInfo.type
       }).forEach(function (menuItem) {
         var item = menuItem.render(fileInfo);
         if (item)
           contextMenu.addChild(item);
       });
     },

    /**
     * This function removes all nodes from the tree and fills the first layer.
     * In this layer there is the root node (indicating the project name), the
     * labeled shortcut nodes and the nodes in the root, which usually means the
     * '/' directory.
     */
    reloadTree : function () {
      var that = this;

      //--- Remove previous nodes ---//

      this._store.query().forEach(function (node) {
        that._store.remove(node.id);
      });

      //--- Insert project node ---//

      var projectNode = {
        id       : 'project',
        name     : 'Project: ', // TODO: name
        fileInfo : { isDirectory : true }
      };

      this._store.put(projectNode);

      //--- Insert shortcuts ---//

      // TODO: labels
//      for (var key in projectInfo.paths)
//        this._store.put({
//          id       : key,
//          name     : 'Jump to ' + key,
//          fileInfo : { isDirectory : false },
//          shortcut : projectInfo.paths[key],
//          parent   : 'project'
//        });

      //--- Insert root files under the project node ---//

      model.project.getRootFiles().forEach(function (fileInfo) {
        that._store.put({
          id       : fileInfo.id,
          name     : fileInfo.name || '/',
          fileInfo : fileInfo,
          parent   : 'project'
        });
      });
    },

    /**
     * Overridable function to return CSS class name to display icon.
     * @param {dojo/data/Item} item Tree item.
     * @param {Boolean} opened Is the tree node opened.
     * @returns {String} CSS class name.
     */
    getIconClass : function (item, opened) {
      var name = item.fileInfo.name || '';
      var cssClass = util.getIconClass(name);

      if (item.isFolderUpElement)
        return 'icon icon-folder-up';

      if (name.toLowerCase().indexOf('makefile') !== -1)
        return 'icon makefile';

      if (item.id === 'project')
        return 'icon icon-project';

      if (cssClass)
        return 'icon ' + cssClass;

      return this.inherited(arguments);
    },

    /**
     * This function loads the child nodes of the given node from the server and
     * returns a list of tree nodes which belong to the child files.
     * @param {Object} node A node of which the children are going to be loaded.
     */
    loadChildren : function (node) {
      if (node.id === 'project')
        return this._store.query({ parent : 'project' });

      if (node.isFolderUpElement)
        return [];

      var children = [];

      model.project.getChildFiles(node.fileInfo.id).forEach(
      function (childFileInfo) {
        var name = childFileInfo.name;
        var parseStatus = childFileInfo.parseStatus;

        if (parseStatus === FileParseStatus.FullyParsed)
          name = '<span style="color: green">' + name + '</span>';
        else if (parseStatus === FileParseStatus.PartiallyParsed)
          name = '<span style="color: orange">' + name + '</span>';

        children.push({
          id       : childFileInfo.id,
          name     : name,
          fileInfo : childFileInfo
        });
      });

      return children;
    },

    onClick : function (item, node, event) {
      if (!node.isExpandable)
        topic.publish('codecompass/openFile', {
          fileId     : item.fileInfo.id,
          moduleId   : 'text',
          info       : 'Open file: ' + item.fileInfo.name,
          newSession : true
        });
    },

    setState : function (state) {
      if (!state.fid)
        return;

      var fileInfo = model.project.getFileInfo(state.fid);

      var path = fileInfo.path.split('/');
      for (var i = 1; i < path.length; ++i)
        path[i] = path[i - 1] + '/' + path[i];
      path[0] = '/';
      path = path.map(function (path) {
        return model.project.getFileInfoByPath(path).id;
      });
      path.unshift('project');

      this.set('path', path);
    }
  });

  /**
   * Midnight Commander style File Manager.
   */
  var ProjectTreeMC = declare([FileBrowser, HtmlTreeNoIndent], {
    loadChildren : function (node) {
      var children = this.inherited(arguments);

      if (!node.isFolderUpElement &&
          node.fileInfo.isDirectory &&
          node.id !== 'project')
        children.unshift({
          name : '..',
          isFolderUpElement : true,
          fileInfo : {
            path : node.fileInfo.path
          }
        });

      return children;
    },

    onOpen : function (item, node) {
      this._displayElements(node.getParent().getChildren(), false);
    },

    onClick : function (item, node, event) {
      if (item.isFolderUpElement) {
        var parent = node.getParent();
        var children = parent.getParent().getChildren();
        this._collapseNode(parent);
        this._displayElements(children, true);
      } else {
        this.inherited(arguments);
      }
    },

    _displayElements : function (nodes, display) {
      nodes.forEach(function (node) {
        // TODO: shortcut
        if (node.item.id !== 'project')
          style.set(node.rowNode, 'display', display ? 'block' : 'none');
      });
    }
  });

  /**
   * Traditional tree style File Manager.
   */
  var ProjectTreeOrig = declare([FileBrowser, HtmlTree], {
  });

  var FileManager = declare(ContentPane, {
    constructor : function () {
      this.projectTreeOrig = new ProjectTreeOrig();
      this.projectTreeMC = new ProjectTreeMC();

      this.currentChild = this.projectTreeMC;
    },

    postCreate : function () {
      this.addChild(this.currentChild);
    },

    setState : function () {
      this.currentChild.setState.apply(this.currentChild, arguments);
    }
  });

  var fileManager = new FileManager({
    id : 'filemanager',
    title : 'File Manager'
  });

  viewHandler.registerModule(fileManager, {
    type : viewHandler.moduleType.Accordion,
    priority : 10
  })

});
