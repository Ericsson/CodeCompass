define([
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/mouse',
  'dojo/topic',
  'codecompass/registration/model',
  'codecompass/viewHandler'],
function (Tree, ObjectStoreModel, declare, Memory, Observable, mouse, topic,
  model, viewHandler) {
  
  /**
   * This function places the given info nodes under a given node.
   * @param {InfoTree} infoTree InfoTree instance.
   * @param {Array} infoNodes InfoNode thrift objects to insert in the tree.
   * @param {String | Number} parentId Node under which the info nodes are
   * added.
   */
  function storeInfoNodes(infoTree, infoNodes, parentId) {
    var parentNode = infoTree._store.get(parentId);

    infoNodes.forEach(function (infoNode) {
      var currentInfoNode = parentNode;

      infoNode.category.forEach(function (category) {
        var categoryNode = infoTree._store.query({
          parent : currentInfoNode.id,
          name   : category
        })[0];

        if (!categoryNode)
          var insertedNodeId = infoTree._store.put({
            parent : currentInfoNode.id,
            name   : category,
            loaded : true
          });

        currentInfoNode = categoryNode || infoTree._store.get(insertedNodeId);
      });

      if (infoNode.value) {
        var inName = (infoNode.label ? infoNode.label : '');
        if (infoNode.value !== "[ignored]") {
          inName = inName + (inName ? ': ' : '') + infoNode.value;
        }
        
        infoTree._store.put({
          parent   : currentInfoNode.id,
          infoNode : infoNode,
          loaded   : true,
          name     : inName
        });
      } else {
        currentInfoNode.loaded   = false;
        currentInfoNode.infoNode = infoNode;
      }
    });
  }
  
  /**
   * This function returns an array of InfoNode thrift object which are queried
   * from the server either by getSubInfoTree or getSubInfoTreeForFile service
   * functions depending on whether the currently loaded node id is an AstNodeId
   * or FileId.
   * @param {InfoTree} infoTree InfoTree instance.
   * @param {InfoNode} infoNode InfoNode thrift object.
   */
  function getSubTree(infoTree, infoNode) {
    return infoTree._currentElementId instanceof model.AstNodeId
      ? infoTree._languageService.getSubInfoTree(
          infoTree._currentElementId, infoNode.query)
      : infoTree._languageService.getSubInfoTreeForFile(
          infoTree._currentElementId, infoNode.query);
  }

  return declare(Tree, {
    openOnClick : true,

    constructor : function () {
      var that = this;

      //--- Store and model ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          if (!node.loaded)
            storeInfoNodes(that, getSubTree(that, node.infoNode), node.id);

          return that._store.query({ parent : node.id });
        }
      }));

      var dataModel = new ObjectStoreModel({
        store : that._store,
        query : { id : 'root' },
        mayHaveChildren : function (node) {
          return !node.infoNode || node.infoNode.query.queryId;
        }
      });

      //--- Tree ---//

      that._store.put({
        id     : 'root',
        loaded : true,
        name   : 'Info Tree'
      });

      this.set('model', dataModel);
    },

    /**
     * This function loads the first level of the info tree based on the given
     * info object.
     * @param {AstNodeInfo | FileInfo} elementInfo Thrift object.
     * @param {FileType} fileType Thrift object.
     */
    loadInfoTree : function (elementInfo, fileType) {
      var that = this;

      this._languageService = model.getLanguageService(fileType);
      this._currentElementId = elementInfo.astNodeId || elementInfo.file;

      //--- Remove previous tree data ---//

      this._store.query().forEach(function (node) {
        that._store.remove(node.id);
      });

      //--- Set root node ---//

      this._store.put({
        id       : 'root',
        infoNode : { query : { queryId : 0 } },
        loaded   : true,
        name     : elementInfo instanceof model.AstNodeInfo
                 ? elementInfo.astNodeType + ': ' + elementInfo.astNodeValue
                 : 'File: ' + elementInfo.name
      });

      //--- Set further nodes ---//

      if (this._languageService)
        storeInfoNodes(
          this,
          elementInfo instanceof model.AstNodeInfo
            ? this._languageService.getInfoTree(elementInfo.astNodeId)
            : this._languageService.getInfoTreeForFile(elementInfo.file),
          'root');
    },

    onShow : function () {},
    
    onClick : function(item, node, event) {
      if(item.infoNode){
        var astNodeInfo = item.infoNode.astValue;
        
        if (astNodeInfo.documentation.indexOf("[Implicit]") > -1)
        {
          var textModule = viewHandler.getModule("text");
          var splitted = astNodeInfo.documentation.split("|");
          
          // Keep this synchronised with the implicit symbol's handler!
          var clickedFileInfo = new FileInfo();
          clickedFileInfo.type = splitted[1];
          clickedFileInfo.parseStatus = parseInt(splitted[3]);
          clickedFileInfo.name = "[compiler generated function]";
          clickedFileInfo.path = splitted[2];
          clickedFileInfo.isGenerated = true;
          
          var infoBox = textModule._languageService.getInfoBox(astNodeInfo.astNodeId);

          textModule._infoBoxWidget.set('fileInfo', clickedFileInfo);
          textModule._infoBoxWidget.set('data', infoBox);
          textModule._infoBoxWidget.set('style', 'display: block');
          textModule._infoBoxWidget.resize();
        } else {
          topic.publish('codecompass/openFile', {
            fileId     : astNodeInfo.range.file,
            line       : astNodeInfo.range.range.startpos.line,
            moduleId   : 'text',
            newTab     : mouse.isMiddle(event),
            info       : 'Info tree: ' + astNodeInfo.astNodeValue,
            newSession : true,
            selection  : {
              from : astNodeInfo.range.range.startpos,
              to   : astNodeInfo.range.range.endpos
            }
          });
        }
      }
    },
    
    getIconClass : function (item, opened) {
      switch(item.name) {
        case 'Aliases':
          return 'into_tree_aliases';
        case 'Inherits From':
          return 'into_tree_inherit_from';
        case 'Inherited By':
          return 'into_tree_inherit_by';
        case 'Functions':
        case 'Methods':
          return 'into_tree_methods';
        case 'Friends':
          return 'into_tree_friend';
        case 'Members':
        case 'Local Variables':
          return 'into_tree_member';
        case 'Usage':
          return 'into_tree_usage';
        case 'Reads':
          return 'into_tree_read';
        case 'Writes':
          return 'into_tree_write';
        case 'Declarations':
          return 'into_tree_declaration';
        case 'Parameters':
          return 'into_tree_parameter';  
        case 'Calls':
        case 'Callees':
          return 'into_tree_callees';  
        case 'Callers':
          return 'into_tree_callers';  
        case 'Assigned To Function Pointer':
          return 'into_tree_assigned'; 
        case 'Enum Constants':
          return 'into_tree_enum'; 
        case 'Overrides':
          return 'into_tree_overrides';        
        case 'Overriden by':
          return 'into_tree_overriden_by';  
        case 'Undefinitions':
          return 'info_tree_undef'; 
        case 'Expansions':
          return 'info_tree_expansion'; 
        case 'Includes':
          return 'info_tree_include'; 
        case 'Macros':
          return 'info_tree_macro'; 
        case 'Types':
          return 'info_tree_type'; 
        default:
          if(item.id == 'root')
            return 'info_tree_root';
          else
            return this.inherited(arguments);
      }
      
    },

  });
});
