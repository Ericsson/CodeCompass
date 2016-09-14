define([
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dojo/mouse',
  'dijit/layout/ContentPane',
  'codecompass/registration/model'],
function (Tree, ObjectStoreModel, array, declare, Memory, Observable, dom,
          style, topic, mouse, ContentPane, model) {


  function addVersionTreeEntriesToTree(treeStore, parentTreeNode,
                                       versionTreeEntries) {
    for (var i = 0; i < versionTreeEntries.length; i++) {
      var newNodeInfo = versionTreeEntries[i];
      
      if (parentTreeNode.nodeInfo.path != "") {
        newNodeInfo.path = parentTreeNode.nodeInfo.path + "/" +
          versionTreeEntries[i].fileName;
      } else {
        newNodeInfo.path = versionTreeEntries[i].fileName;
      }

      treeStore.put({
        id       : parentTreeNode.id + '_' + i,
        name     : versionTreeEntries[i].fileName,
        parent   : parentTreeNode.id,
        loaded   : false,
        nodeInfo : newNodeInfo
      });
    };
  }
  
  
  return declare(ContentPane, {
    constructor : function () {
      var that = this;

      //--- Tree ---//
      
      this._data = [];
      
      this._commitDetails = {};

      this._store = new Observable(new Memory({
        data        : that._data,
        getChildren : function (node) {
          if (!node.loaded) {
            
            var treeEntries = model.versionservice.getTree(
              that._commitDetails.repoId,
              node.nodeInfo.pointedOid
            );
            
            addVersionTreeEntriesToTree(that._store, node, treeEntries);
          }

          return that._store.query({ parent : node.id });
        }
      }));
          
      this._store.put({
        id       : 'root',
        name     : '/',
        loaded   : true,
        nodeInfo : {
          "fileMode" : VersionTreeEntryFileMode.GIT_FILEMODE_TREE,
          "path" : ""
        }
      });
    
      // Create the model
      var versionDataModel = new ObjectStoreModel({
        store: that._store,
        query: {id: 'root'},
        mayHaveChildren : function (node) {
          return node.nodeInfo.fileMode ==
                 VersionTreeEntryFileMode.GIT_FILEMODE_TREE;
        }
      });

      // Create the Tree.
      this._tree = new Tree({          
        model: versionDataModel,
        openOnClick: true,
        onClick: function (item, node, event) {
          if (item.nodeInfo.fileMode !=
            VersionTreeEntryFileMode.GIT_FILEMODE_TREE) {

            topic.publish('codecompass/versionOpenText', {
              moduleId        : 'versiontext',
              newTab          : mouse.isMiddle(event),
              versionrepoid   : that._commitDetails.repoId,
              versioncommitid : that._commitDetails.oid,
              versionpath     : item.nodeInfo.path
            });
          }
        }
      });
    },

    postCreate : function () {
      this.set('content', this._tree.domNode);
      this._tree.startup();
    },
    
    _initializeVersionTreeComponent : function (commitDetails) {
      if (!commitDetails.repoId) return;

      // clear current tree
      // TODO: find a faster solution
      for (var i = this._data.length - 1; i >= 0; --i)
        this._store.remove(this._data[i].id);
      
      this._commitDetails = commitDetails;
      var versiontreeid = commitDetails.treeOid;
      
      var treeEntries = model.versionservice.getTree(
        commitDetails.repoId,
        versiontreeid
      );
      
      var rootNode = {
        id       : 'root',
        name     : '/ (commit ' + commitDetails.oid + ')',
        loaded   : true,
        nodeInfo : {
          "fileMode" : VersionTreeEntryFileMode.GIT_FILEMODE_TREE,
          "path" : ""
        }
      };
      this._store.put(rootNode);
      
      addVersionTreeEntriesToTree(this._store, rootNode, treeEntries);
    },
    
    setRootCommit : function(commitDetails) {
      this._initializeVersionTreeComponent(commitDetails);
    }

    
  });
});
