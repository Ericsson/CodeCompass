define([  
  'dojo/topic',
  'dojo/on',  
  'dojo/mouse',
  'dojo/query',
  'dojo/window',  
  'dojo/cookie',
  'dojo/dom',
  'dojo/dom-style',
  'dojo/dom-geometry',
  'dojo/dom-construct',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'dijit/registry',
  'dijit/layout/ContentPane',
  'dijit/form/Select',
  'codecompass/registration/model',
  'codecompass/registration/menuHelper',
  'codecompass/util',
  'codecompass/urlHandler',
  'codecompass/viewHandler',  
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/HtmlTree'],

function (topic, on, mouse, query, win, cookie, dom, domStyle, domGeom, domConstruct, declare, Memory, 
          Observable, Tree, ObjectStoreModel, registry, ContentPane, Select, model,
          menuHelper, util, urlHandler, viewHandler, ContextMenu, HtmlTree) { 

  /**
   * This function scrolls the given node into view. This means that when this
   * function is called for a node then the node's +/- expand button comes in
   * to the visible area, even if the node doesn't have any children and the
   * expand button is invisible.
   * @param {FileManager} tree A project tree object.
   * @param {Object} node A node if the tree.
   * @param {Boolean} need to scroll to left
   */
  function scrollNodeInView(tree, node, scrollToLeft) {
    setTimeout(function () {
      if(scrollToLeft)
        tree.domNode.scrollLeft += 1000;
      
      var expando = dojo.byId(node.id);
      if (expando)
        win.scrollIntoView(expando);
    }, 300);
  }
  
  var HtmlTreeNoIndentNode = declare(Tree._TreeNode, {
    _setLabelAttr : { node : "labelNode", type : "innerHTML" },
    _setIndentAttr : 0,    
    baseClass : "project-tree-file-manager",
  });

  var HtmlTreeNoIndent = declare(HtmlTree, {
    _createTreeNode : function (args) {
      return new HtmlTreeNoIndentNode(args);
    }
  });
  
  /*
   * Common File Browser module
   */
  var FileBrowser = declare(Tree, {
    /**
     * Current path in file browser
     */
    currentPath : "", 
    
    /**
     * Title dom element in the file manager
     */
    domFileManagerTitle : null, 
    
    /**
     * The actual path dom element in file manager
     */
    domFileManagerPath : null,
    
    /**
     * File manager type select dom element
     */
    domSelectFileManager : null,
    
    /*
     * Construct the UI for this widget
     */
    buildRendering : function () {
      this.inherited(arguments);
      this.reloadTree();
    },
    
   /*
    * Called after a widget and its children 
    * have been created and added to the page
    */
    startup : function () {
      this.inherited(arguments);

      //--- Menu ---//
      var contextmenu = new ContextMenu({
        targetNodeIds : [this.id],
        selector      : '.dijitTreeNode'
      });

      on(this, '.dijitTreeNode:contextmenu', function (event) {
        var fileInfo
          = dijit.byNode(query(event.target)
          .closest('.dijitTreeNode')[0]).item.fileInfo;
          
        menuHelper.buildFileContextmenu(contextmenu, fileInfo);
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

      this._selectedNode = undefined;

      this._store.query().forEach(function (node) {
        that._store.remove(node.id);
      });      
      
      //--- Insert Project node ---//

      var projectId = new model.ProjectId();
      projectId.prid = 1;

      var projectInfo = model.project.getProjectInfo(projectId);

      var projectNode = {
        id       : 'project',
        name     : 'Project: ' + projectInfo.name,
        loaded   : true,
        fileInfo : { isDirectory : true }
      };

      this._store.put(projectNode);
      
      //--- Insert shortcuts ---//

      for (var key in projectInfo.paths)
        this._store.put({
          id       : key,
          name     : 'Jump to ' + key,
          fileInfo : { isDirectory : false },
          shortcut : projectInfo.paths[key],
          parent   : 'project'
        });

      //--- Insert root files under the Project node ---//
      
      model.project.getRootFiles(projectId).forEach(function (fileInfo) {
        that._store.put({
          name     : fileInfo.name || '/',
          fileInfo : fileInfo,
          parent   : 'project'
        });
      });
    },
    
    /**
   * This function loads the child nodes of the given node from the server and
   * places them into the memory of the tree.
   * @param {Memory} memory An object of type Memory in which the child nodes
   * are loaded.
   * @param {Object} node A node of which the children are going to be loaded.
   */
    loadChildren: function(memory, node) {
      var children = [];

      model.project.getChildFiles(node.fileInfo.file).forEach(
      function (childFileInfo) {
        var name = childFileInfo.name;
        var parseStatus = childFileInfo.parseStatus;

        if (parseStatus === model.FileParseStatus.FullyParsed)
          name = '<span style="color: green">' + name + '</span>';
        else if (parseStatus === model.FileParseStatus.PartiallyParsed)
          name = '<span style="color: orange">' + name + '</span>';

        var obj = {
          name     : name,
          fileInfo : childFileInfo,
          parent   : node.id
        };

        memory.put(obj);
        children.push(obj);
      });

      return children;
    },
    
    /*
     * Overridable function to return CSS class name to display icon
     * @param {dojo/data/Item} item
     * @param {Boolean} opened
     * @returns {String} css class name
     */
    getIconClass : function (item, opened) {
      var cssClass = util.getIconClass(item.name);
      
      if(item.fileInfo.isFolderUpElement)
        return 'folder_up';
      
      if (cssClass)
        return cssClass;
      
      if (item.shortcut)
        return 'green_folder';
      
      if (item.name.toLowerCase().indexOf('makefile') !== -1)
        return 'makefile';
      
      if (item.id === 'project')
        return 'blue_folder';
      
      return this.inherited(arguments);
    },
    
    

    /**
     * When the project tree is the member of a container (e.g. an Accordion
     * panel) then the onShow() function runs when the tree becomes visible in
     * this panel. When this event occurs, then the tree is opened to the file
     * of which the file id is read from the URL.
     */
    onShow : function () {
      this._isShown = true;
      
      this.setFileManagerPath();
      
      var stateValues = urlHandler.getState();

      if (!stateValues.fid || this._prevFid === stateValues.fid)
        return;
      
      this._prevFid = stateValues.fid;

      var fileId = new model.FileId();
      fileId.fid = stateValues.fid;

      try {
        var fileInfo = model.project.getFileInfo(fileId);
        this._selectedNode = this.openPath(fileInfo.path);
      } catch (ex) {
        console.warn('Invalid fileId', fileId);
      }
    },
    
    /*
     * Called when another widget becomes the selected pane or to indicate hide
     */
    onHide : function () {
      this._isShown = false;
    },
    
    /**
     * Set the actual state from the URL
     * @param {Url} url
     */
    setState : function (url) {
      if (!url.fid)
        return;
      
      var fileId = new model.FileId();
      fileId.fid = url.fid;
      
      try {
        var path = model.project.getFileInfo(fileId).path;
        
        topic.publish('codecompass/fileClick', { path : path });
        this.openPath(path);
      } catch (ex) {
        console.warn('Invalid file id', fileId);
      }
    },
    
    /*
     * Calculate the actual width of an element
     * @param node - dom element
     * @returns Return the rights width of the element
     */
    getWidthOfAnElement : function(node){
      if(node){
        var computedStyle = domStyle.getComputedStyle(node);
        return domGeom.getMarginBox(node, computedStyle).w;
      }else
        return 0;
    },
    
    /**
     * Set the fielemanager path in the gui
     */
    setFileManagerPath : function(){
      if(this.currentPath === undefined || !this.domFileManagerTitle)
        return true;
      
      // Calculate the possible size of the path that can be in the gui
      var paneWidth = this.getWidthOfAnElement(dom.byId('filemanager'));
      var titleWidth = this.getWidthOfAnElement(this.domFileManagerTitle);
      var selectWidth = this.getWidthOfAnElement(this.domSelectFileManager.domNode);
      
      var title = this.domFileManagerTitle.innerHTML;
      
      var fontWidth = titleWidth / title.length;
      
      var restWidth = paneWidth - selectWidth - titleWidth;
      var maxChar = restWidth / fontWidth;
      var epsilon = 4;
      var start = this.currentPath.length - maxChar + epsilon;
      
      var path = "";
      if(start < 0) // path fits, no need to cut
        path = this.currentPath;
      else{ // correct the path
        var correctWidthPath = this.currentPath.slice(
          start,
          this.currentPath.length);
        if(this.currentPath.indexOf(start - 1) == '/')
          path = correctWidthPath;
        else{          
          path = "../" + correctWidthPath.replace(/[^\/]*\//, ''); // Remove the begin
        }
        
      }
      this.domFileManagerPath.innerHTML = path;
    },
    
    
  });
  
  // Midnight Commander style File Manager
  var ProjectTreeMC = declare([FileBrowser, HtmlTreeNoIndent], {    
    constructor : function () {
      //--- Initialisation ---//

      var that = this;
      
      topic.subscribe('codecompass/openPath', function (path) {
        that.openPath(path);
      });

      topic.subscribe('codecompass/openFile', function (message) {
        that.openPath(model.project.getFileInfo(message.fileId).path);
      });
      
      //--- Store ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          if (!node.loaded && !node.fileInfo.isFolderUpElement) {
            node.loaded = true;
            return that.loadChildren(this, node);
          }

          return this.query({ parent : node.id });
        }
      }));
      
      //--- Model ---//

      this._dataModel = new ObjectStoreModel({
        store : this._store,
        query : { id : 'project' },
        mayHaveChildren : function (node) {
          return node.fileInfo.isDirectory;
        }
      });
      
      this.set('model', this._dataModel);
    },
    
  /**
   * This function loads the child nodes of the given node from the server and
   * places them into the memory of the tree.
   * @param {Memory} memory An object of type Memory in which the child nodes
   * are loaded.
   * @param {Object} node A node of which the children are going to be loaded.
   */
    loadChildren : function(memory, node){
      var children = this.inherited(arguments);
      if(node.fileInfo.isDirectory){
        var upperFolderNode = {
          id       : node.parent,
          name     : '...',
          loaded   : true,
          fileInfo : { isFolderUpElement : true, isDirectory : true, path : node.fileInfo.path }
        };
        children.unshift(upperFolderNode);
      }
      return children;
    },
    
    /**
     * This function expands the tree along the given path and returns the node
     * which belongs to the last element of the path.
     * Note: there is a shorter implementation, namely using the "path"
     * attribute of the tree, but that's much slower.
     * @param {Tree} tree Tree dojo object.
     * @param {String} path An absolute path.
     */
    openPath : function (path) {
      var paths = path.split('/');
      this.currentPath = paths.slice(0, paths.length - 1).join('/');
      this.setFileManagerPath();
      var that = this;
      
      if (!this._isShown || this._previousPath === path)
        return;
      this._previousPath = path;
      
      // Just for fun :)
      (path = path.substr(
        0, path.length - (path[path.length - 1] === '/')).split('/')
      )[0] = '/';
      
      var currentNode = this.getChildren()[0];
      currentNode.expand();
      this._expandNode(currentNode);
      
      var index, children;
      path.forEach(function (directory) {
        children = currentNode.getChildren();
        index = util.findIf(children, function (child) {
          return child.label.match(
            /(<[^>]*>)?([^<]*)(<[^>]*>)?/)[2] === directory;
        });
        
        if (index !== -1) {
          currentNode = children[index];
          that._expandNode(currentNode);        
          children.forEach(function(child){
            if(child !== currentNode && child.isExpanded){
              that.collapseAllChild(child, true); 
            }
          });    
        }  
      });
      if(children[index].item.fileInfo.isDirectory){
        that.collapseAllChild(children[index], false); 
      }
      
      if (this._selectedNode)
        this._selectedNode.setSelected(false);
  
      currentNode.setSelected(true);
      this._selectedNode = currentNode;
      
      // Recursively hide parent childs
      var recNode = currentNode.getParent();      
      while(recNode && recNode.id != 'filemanager'){
        this.showHideElements(recNode.getChildren(), "hide");
        recNode = recNode.getParent();
      }
      
      // If current item is a file, show parent folder children
      if(!currentNode.item.fileInfo.isDirectory){        
        this.showHideElements(currentNode.getParent().getChildren(), "show");      
      }
          
      // show only the current node childs
      this.showHideElements(currentNode.getChildren(), "show");
            
      // This fixes the bug that after auto-expanding a path, clicking on "+" of
      // a sibling node the tree closes, because it thinks that we click on a
      // higher opened node on the path which thus closes. See clicked node in
      // onClick callback.
      currentNode.focus();
      scrollNodeInView(this, currentNode, false);
      
      return currentNode;
    },

    collapseAllChild : function(node, collapseActualNode){      
      var that = this;
      if(node.item.fileInfo.isDirectory){
        node.getChildren().forEach(function(child){
          if(child.isExpanded){
            dojo.setStyle(child.rowNode, "display", "none");
            that._collapseNode(child);
            that.collapseAllChild(child)
          }
        }); 
        if(collapseActualNode)
          that._collapseNode(node);
      }
    },
    /*
     * This function is hiding or showing a collection of element in nodes
     * @param {array} nodes - Array of nodes
     * @param {string} display - show or hide
     * @returns {undefined}
     */
    showHideElements : function(nodes, display){
      nodes.forEach(function(node){          
        if(node.item.shortcut == undefined && (
           node.item.id != 'project' || 
           node.item.fileInfo.isFolderUpElement)
        ){            
          dojo.setStyle(node.rowNode, "display", 
            display == "show" ? "block" : "none");
        }
      });
    },
    
    /**
     * Callback when a tree node is clicked
     * @param {Object} item
     * @param {Object} node
     * @param {Event} event
     */
    onClick : function (item, node, event) {
      if(node.item.fileInfo.isDirectory){        
        this.setPath(item, node);        
      }
      
      // Up folder event
      if(node.item.fileInfo && node.item.fileInfo.isFolderUpElement){
        
        var parent = node.getParent();        
        var children = parent.getParent().getChildren();
        this.showHideElements(children, "show");
        this.showHideElements(parent.getChildren(), "hide");
                
        this._collapseNode(parent);     
        parent.focus();
        
        return;        
      }
            
      // Hide parent childs, if node is a directory
      if(node.item.fileInfo.isDirectory && 
        node.item.id != 'project')
      {
        this.showHideElements(node.getParent().getChildren(), "hide");
      }
      
      if (this._selectedNode)
        this._selectedNode.setSelected(false);


      if (item.shortcut){        
        this._selectedNode = this.openPath(item.shortcut);        
      }      
      
      else if (!node.isExpandable) {
        topic.publish('codecompass/openFile', {
          fileId     : item.fileInfo.file,
          moduleId   : 'text',
          newTab     : mouse.isMiddle(event),
          info       : 'Open file: ' + item.fileInfo.name,
          newSession : true
        });
      }
      else if (node.isExpanded)
        this._collapseNode(node);
      else {
        this._expandNode(node);
        // Upper Project folder - show childs only if all children is collapsed
        var hasChildCollapsed = false;
        node.getChildren().forEach(function(child){
          if(child.isExpanded){
            hasChildCollapsed = true;
            return;
          }            
        });
        
        if(!hasChildCollapsed || node.item.id != 'project')
          this.showHideElements(node.getChildren(), "show");
        
        if (node.hasChildren()){
          scrollNodeInView(this, node.getChildren()[0], false);
          if(node.getChildren().length >= 2)
            node.getChildren()[1].focus();
          else
            node.getChildren()[0].focus();
        }
      }
      
      if (item.id !== 'project' && !item.shortcut && item.isFolderUpElement) {
        topic.publish('codecompass/fileClick', { path : item.fileInfo.path });        
      }
      this._previousPath = item.fileInfo.path;
    },
    
    /**
     * Set the actual path and refresh the gui
     * @param {Object} item
     * @param {Object} node
     * @returns Set the actual file manager path
     */
    setPath: function(item, node){  
      if(item.fileInfo && item.fileInfo.isDirectory){
        if(item.fileInfo.isFolderUpElement && item.fileInfo.path == "/")
          this.currentPath = "";
        else if(item.fileInfo.isFolderUpElement)
          this.currentPath = node.getParent().getParent().item.fileInfo.path;
        else
          this.currentPath = item.fileInfo.path;
        this.setFileManagerPath();
      }      
    }
    
  });
  
  var ProjectTreeOrig = declare([FileBrowser, HtmlTree], {
    constructor : function () {
      //--- Initialisation ---//

      var that = this;
      
      topic.subscribe('codecompass/openPath', function (path) {
        that.openPath(path);
      });

      topic.subscribe('codecompass/openFile', function (message) {
        that.openPath(model.project.getFileInfo(message.fileId).path);
      });
      
      //--- Store ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          if (!node.loaded) {
            node.loaded = true;
            return that.loadChildren(this, node);
          }

          return this.query({ parent : node.id });
        }
      }));

      //--- Model ---//

      this._dataModel = new ObjectStoreModel({
        store : this._store,
        query : { id : 'project' },
        mayHaveChildren : function (node) {
          return node.fileInfo.isDirectory;
        }
      });
      
      this.set('model', this._dataModel);
    },
    
    /**
     * This function expands the tree along the given path and returns the node
     * which belongs to the last element of the path.
     * Note: there is a shorter implementation, namely using the "path"
     * attribute of the tree, but that's much slower.
     * @param {Tree} tree Tree dojo object.
     * @param {String} path An absolute path.
     */
    openPath : function (path){
      var paths = path.split('/');
      this.currentPath = paths.slice(0, paths.length - 1).join('/');
      this.setFileManagerPath();
      
      var that = this;

      if (!this._isShown || this._previousPath === path)
        return;
      this._previousPath = path;
      
      // Just for fun :)
      (path = path.substr(
        0, path.length - (path[path.length - 1] === '/')).split('/')
      )[0] = '/';
      
      var currentNode = this.getChildren()[0];
      currentNode.expand();
      this._expandNode(currentNode);
  
      path.forEach(function (directory) {
        var children = currentNode.getChildren();
        var index = util.findIf(children, function (child) {
          return child.label.match(
            /(<[^>]*>)?([^<]*)(<[^>]*>)?/)[2] === directory;
        });
  
        if (index !== -1) {
          currentNode = children[index];
          that._expandNode(currentNode);
        }
      });
      
      if (this._selectedNode)
        this._selectedNode.setSelected(false);
  
      currentNode.setSelected();
      this._selectedNode = currentNode;
  
      // This fixes the bug that after auto-expanding a path, clicking on "+" of
      // a sibling node the tree closes, because it thinks that we click on a
      // higher opened node on the path which thus closes. See clicked node in
      // onClick callback.
      currentNode.focus();
      scrollNodeInView(this, currentNode, true);
      
      return currentNode;
    },
    
    /**
     * Callback when a tree node is clicked
     * @param {Object} item
     * @param {Object} node
     * @param {Event} event
     */
    onClick : function (item, node, event){
      this.setPath(item, node);      
      
      if (this._selectedNode)
        this._selectedNode.setSelected(false);

      if (item.shortcut)
        this._selectedNode = this.openPath(item.shortcut);
      else if (!node.isExpandable) {
        this._previousPath = item.fileInfo.path;
        topic.publish('codecompass/openFile', {
          fileId     : item.fileInfo.file,
          moduleId   : 'text',
          newTab     : mouse.isMiddle(event),
          info       : 'Open file: ' + item.fileInfo.name,
          newSession : true
        });
      }
      else if (node.isExpanded)
        this._collapseNode(node);
      else {
        this._expandNode(node);
        if (node.hasChildren())
          scrollNodeInView(this, node.getChildren()[0], true);
      }

      if (item.id !== 'project' && !item.shortcut) {
        topic.publish('codecompass/fileClick', { path : item.fileInfo.path });
        this._previousPath = item.fileInfo.path;
      }
    },
    
    /**
     * Set the actual path and refresh the gui
     * @param {Object} item
     * @param {Object} node
     * @returns Set the actual file manager path
     */
    setPath: function(item, node){
      if(item.fileInfo && item.fileInfo.isDirectory){
        if( item.fileInfo.path == "/")
          this.currentPath = "";
        else
          this.currentPath = item.fileInfo.path;
      }else{
        this.currentPath = item.shortcut;
      }
      this.setFileManagerPath();
    }
  });
  
  return declare( ContentPane, {
    /**
     * Copy previous file manager data
     * @param {object} from
     * @param {object} to
     */
    copyPrevManagerData : function(from, to){
      to.domFileManagerTitle = from.domFileManagerTitle;
      to.domFileManagerPath = from.domFileManagerPath;
      to.domSelectFileManager = from.domSelectFileManager;
    },
    
    constructor: function(){  
      var that = this;
      this.projectTreeOrig = new ProjectTreeOrig();
      this.projectTreeMC = new ProjectTreeMC();
      var fileManager = cookie('fileManager');
      if(fileManager !== undefined && fileManager == 'file-manager-tree'){
        this.currentChild = this.projectTreeOrig;
      }else{
        this.currentChild = this.projectTreeMC;   
      }   
      
      topic.subscribe('codecompass/changefilemanager', function(obj){ 
        that.currentChild.onHide();
        that.removeChild(that.currentChild);
        
        if(obj.fileManager == 'file-manager-tree'){
          that.copyPrevManagerData(that.projectTreeMC, that.projectTreeOrig);
          that.addChild(that.projectTreeOrig);
          that.currentChild = that.projectTreeOrig;
        }else{
          that.copyPrevManagerData(that.projectTreeOrig, that.projectTreeMC);
          that.addChild(that.projectTreeMC);
          that.currentChild = that.projectTreeMC;
        }        
        cookie('fileManager', obj.fileManager , 365);
        that.currentChild.onShow();
        
        var accordionContainer = that.getParent().getParent();
        accordionContainer.resize();
      });
      
      
    },
    resize : function (changeSize, resultSize){ 
      var that = this;
      this.inherited(arguments);
      this.currentChild.setFileManagerPath();  // change the fits of the path    
    },
    setState : function(url){
      this.currentChild.setState(url);
    },
    onShow : function(){
      this.currentChild.onShow();
    },
    onHide : function(){
      this.currentChild.onHide();
    },
    postCreate : function(){
      this.addChild(this.currentChild);
    },
    buildRendering : function() {
      this.inherited(arguments);
    },
    startup : function() {
      this.inherited(arguments);
      
      var that = this;      
      
      var title = this.get('title');
      this.set('title', '');
      
      this.currentChild.domFileManagerTitle = domConstruct.create('span', 
        {
          id : 'file-manager-title',
          innerHTML : title,
        }, this.id + "_button_title", "first");
        
      this.currentChild.domFileManagerPath = domConstruct.create('span', 
        {
          id : 'file-manager-path',
        }, this.currentChild.domFileManagerTitle, "after");
      
      var selectFileManagerBox = domConstruct.create('span', 
        {
          id : 'select-file-manager',
        }, this.currentChild.domFileManagerPath, "after");
            
      this.currentChild.domSelectFileManager = new Select({
        options : [
          {
            label:'MC Style', 
            value:'file-manager-mc'
          },
          {
            label:'Tree Style', 
            value:'file-manager-tree', 
            selected : (cookie('fileManager')!== undefined && 
                        cookie('fileManager') == 'file-manager-tree'?true:false)
          }
        ],
        onChange : function (value) {
          if(value == "file-manager-tree"){
            var fileManager = 'file-manager-tree';
          }else{
            var fileManager = 'file-manager-mc';
          }
          topic.publish('codecompass/changefilemanager', {
            fileManager : fileManager
          });
          if(dijit.byId(fileManager) !== undefined)
            dijit.byId(fileManager).set('checked', true);
          
          cookie('fileManager', fileManager , 365);
        }
      }, selectFileManagerBox);
    }
  });
});
