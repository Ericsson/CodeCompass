require([
  'dojo/on',
  'dojo/query',
  'dojo/dom',
  'dojo/dom-style',
  'dojo/dom-construct',
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
  'codecompass/urlHandler',
  'codecompass/viewHandler',
  'codecompass/view/component/HtmlTree',
  'codecompass/view/component/ContextMenu'],
function (on, query, dom, style, domConstruct, topic, declare, Memory,
  Observable, NodeListTraverse, ContentPane, Tree, ObjectStoreModel, util,
  model, urlHandler, viewHandler, HtmlTree, ContextMenu) {

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
     * Called after a widget and its children have been created and added to the
     * page.
     */
     startup : function () {
       this.inherited(arguments);

       var that = this;

       var contextmenu = new ContextMenu({
         targetNodeIds : [this.id],
         selector      : '.dijitTreeNode'
       });

       on(this, '.dijitTreeNode:contextmenu', function (event) {
         var fileInfo = dijit.byNode(
          query(event.target).closest('.dijitTreeNode')[0]).item.fileInfo;

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

      var labels = model.project.getLabels();
      for (var key in labels) {
        that._store.put({
          id       : 'label_' + key,
          name     : 'Jump to: <span class="cc-label-name">' + key + '</span>',
          fileInfo : { isDirectory : false },
          shortcut : labels[key],
          parent   : 'project'
        });
      };

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

      if (item.shortcut)
        return 'icon icon-tag';

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

      if (node.isFolderUpElement || node.label)
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
      var fileInfo = urlHandler.getFileInfo();

      if (!fileInfo)
        return;

      var path = fileInfo.path.split('/');
      for (var i = 1; i < path.length; ++i)
        path[i] = path[i - 1] + '/' + path[i];
      path[0] = '/';
      path = path.map(function (path) {
        return model.project.getFileInfoByPath(path).id;
      });
      path.unshift('project');

      this.set('path', path);
      this._setFilePath(fileInfo.path);
    },

    _setFilePath : function (path) {
      if (!path)
        return;

      this._filePath = path;

      var paneWidth = util.getFullWidth(this.domNode);
      var titleWidth = util.getFullWidth(this.fileManagerTitle);
      var restWidth = paneWidth - titleWidth;

      var title = this.fileManagerTitle.innerHTML;
      var fontWidth = titleWidth / title.length;
      var maxChar = restWidth / fontWidth;

      var start = path.length - maxChar + 3;
      if (start > 0) {
        var correctWidth = path.slice(start, path.length);
        path = "../" + correctWidth.replace(/[^\/]*\//, '');
      }

      this.fileManagerPath.innerHTML = path;
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
      this._setFilePath(item.fileInfo.path);
    },

    /**
     * Iterate over the shortcut and call callback function on each directory.
     * {String } shortcut Directory path.
     * {Function} cb Callback function.
     */
    shortcutVisitor : function (shortcut, cb) {
      var that = this;

      var currentNode = this.getChildren()[0];

      var path = shortcut.split('/');
      path[0] = '/';

      path.forEach(function (directory) {
        if (!directory.length)
          return;

        children = currentNode.getChildren();
        index = util.findIf(children, function (child) {
          return child.label.match(
            /(<[^>]*>)?([^<]*)(<[^>]*>)?/)[2] === directory;
        });
        currentNode = children[index];

        cb(currentNode);
      });

      return currentNode;
    },

    onClick : function (item, node, event) {
      var that = this;

      if (item.isFolderUpElement) {
        var parent = node.getParent();
        var children = parent.getParent().getChildren();
        this._collapseNode(parent);
        this._displayElements(children, true);
        this._setFilePath(parent.item.fileInfo.path);
      } else if (item.shortcut) { // Click on a label
        if (this._previousPath === item.shortcut)
          return;

        if (this._previousPath)
          this.shortcutVisitor(this._previousPath, function (node) {
            that._collapseNode(node);
          });

        var currentNode = this.shortcutVisitor(item.shortcut, function (node) {
          that._expandNode(node);
        });

        var children = currentNode.getChildren();
        this._displayElements(children, true);
        this._setFilePath(item.shortcut);
        this._previousPath = item.shortcut;
      } else {
        this.inherited(arguments);
      }
    },

    _displayElements : function (nodes, display) {
      nodes.forEach(function (node) {
        if (node.item.id !== 'project' && node.item.shortcut === undefined)
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

    resize : function (changeSize, resultSize){
      this.inherited(arguments);
      this.currentChild._setFilePath(this.currentChild._filePath);
    },

    startup : function () {
      this.inherited(arguments);

      var fileManagerTitleBox = domConstruct.create('span', {
        id : 'filemanagerButton',
      }, this.id + '_button_title', 'replace');

      this.currentChild.fileManagerTitle = domConstruct.create('span', {
        id : 'fileManagerTitle',
        innerHTML : this.title
      }, fileManagerTitleBox, 'last');

      this.currentChild.fileManagerPath = domConstruct.create('span', {
        id : 'fileManagerPath',
      }, fileManagerTitleBox, 'last');
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
