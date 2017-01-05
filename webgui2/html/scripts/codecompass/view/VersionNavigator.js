define([
  'dijit/Tooltip',
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'dijit/layout/BorderContainer',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dojo/mouse',
  'dojox/html/entities',
  'dijit/layout/ContentPane',
  'codecompass/view/component/IconTextBox',
  'codecompass/registration/model'],
function (Tooltip, Tree, ObjectStoreModel, BorderContainer, declare, Memory,
  Observable, topic, mouse, entities, ContentPane, IconTextBox, model) {
  
  var NavTree = declare(Tree, {
    getIconClass : function (item, opened) {
      switch(item.type){
        case 'tag':
          return 'revision_control_tag';
        case 'branch':
          return 'revision_control_branch';
        case 'repo':
          return 'revision_control_repo';
        case 'repositorylist':
          return 'revision_control_list';
        case 'commit':
          return 'revision_control_commit';
        case 'commitloadmore':
          return 'revision_control_more';
        case 'commitlist':
          if(item.ishead)
            return 'revision_control_branch_head';
          else
            return 'revision_control_commits_in';
      }
      if (item.type === 'openbranchesview' || item.type === 'annotatedtag') {
        return 'versionnavigator_branchesview';
      }
      return item.type === 'commitloadmore'
                         ? 'dijitFolderOpened'
                         : this.inherited(arguments);
    },
    
    _onNodeMouseEnter : function (node, evt) {
      if (node.item.nodeInfo) {
        var tipmsg = node.item.nodeInfo.tooltip ||
                     entities.encode(node.item.name);
        Tooltip.show(tipmsg, node.domNode, ['above']);
      }
    },
    
    _onNodeMouseLeave : function (node, evt) {
      Tooltip.hide(node.domNode);
    }
  });  
  
  /**
   * VersionNavigator is the navigation panel on the left side that shows
   * repositories commits and branches.
   */
  return declare(BorderContainer, {
    constructor : function () {
      var that = this;
      
      //--- Search field ---//
      
      this._searchBox = new IconTextBox({
        region   : 'top',
        icon     : '/images/search-icon.png',
        onSubmit : function (value) {
          that._filterText = value;
          that.rebuildOpenCommitLists();
        }
      });
      
      //--- Tree field ---//
      
      this._data = [];

      /**
       * The store contains the version navigator tree nodes. Such a node has a
       * type attribute which can be one of the following ones:
       * - commit: Type of a node representing a commit (leaves in the tree).
       * - commitloadmore: "More..." for loading more commits in a commit list
       * - repo: Node type for representing a repository. Child of the root.
       * - repositorylist: Root node.
       * - tag
       */
      this._store = new Observable(new Memory({
        data        : that._data,
        getChildren : function (node) {
          if (!node.loaded) {
            that.loadNode(node);
          }

          return that._store.query({ parent : node.id });
        }
      }));
          
      var rootNode = {
        id     : 'root',
        name   : 'List of repositories',
        loaded : true, // It will be loaded initially
        type   : 'repositorylist'
      };

      this._store.put(rootNode);
    
      // Create the model
      var versionDataModel = new ObjectStoreModel({
        store : that._store,
        query : { id : 'root' },
        mayHaveChildren : function (node) {
          return node.type !== 'commit'
              && node.type !== 'commitloadmore'
              && node.type !== 'annotatedtag'
              && node.type !== 'openbranchesview';
        }
      });
      
      // Create the Tree.
      this._tree = new NavTree({
        region      : 'center',
        model       : versionDataModel,
        openOnClick : true,
        onClick     : function (item, node, event) {
          if (item.type === 'commit') {
            // If we click on commit, open commit in center window
            topic.publish('codecompass/versionOpenCommit', {
              newTab          : mouse.isMiddle(event),
              versionrepoid   : item.nodeInfo.repoId,
              versioncommitid : item.nodeInfo.oid,
              resetopentab    : false
            });
          } else if (item.type === 'commitloadmore') {
            that._store.remove(item.parent + '_commitloadmore');
            that.loadCommits(item.nodeInfo.repoId,
                             item.nodeInfo.branchName,
                             item.nodeInfo.topCommitId,
                             item.nodeInfo.offset,
                             item.parent);
          } else if (item.type === 'openbranchesview' || item.type === 'annotatedtag') {
            // Show branches view in center window
            topic.publish('codecompass/versionOpenHistory', {
              newTab          : mouse.isMiddle(event),
              versionrepoid   : item.nodeInfo.repoId,
              versionbranch   : item.nodeInfo.branchName,
              versionpath     : ""
            });
          }
        }    
    
      });

      // Finish initializaition by populating the tree with all references.
      this.loadReferencesAsync();
    },
    
    /**
     * Removes the children of the node identified by id from _store.
     */
    removeChildren : function (node) {
      var children = this._store.getChildren(node);
      for (var i = 0; i < children.length; ++i) {
        this._store.remove(children[i].id);
      }
    },
    
    /**
     * Rebuilds open commit lists. Used when changing search term.
     */
    rebuildOpenCommitLists : function (node) {
      if (node == undefined) {
        // Load root node
        node = this._store.get("root");
      }

      // Reload open commitlists
      if (node.loaded) {
        if (node.type === "commitlist") {
          this.removeChildren(node);
          this.loadNode(node);
        } else {
          var children = this._store.getChildren(node);
          for (var i = 0; i < children.length; ++i) {
            this.rebuildOpenCommitLists(children[i]);
          }
        }
      }
    },
    
    loadReferencesAsync: function(){
      var that=this;      
      that._store.put({
          id       : 'loading',
          name     : 'Loading...',
          loaded   : true,
          type     : '',
          parent   : 'root'          
        });
      //calling it asynchrosnously cause it can take long
      model.versionservice.getRepositoryList(function(repositoryList){
           that.loadReferences(repositoryList);
      });    
    },    
    
    /**
     * Load all references (branches) under the root element.
     */
    loadReferences : function (repositoryList) {
      var that = this;      
      that._store.remove("loading");
      
      repositoryList.forEach(function (repo) {
        // Add repository to the tree
        var repoId = repo.pathHash;
        that._store.put({
          id       : 'repo_' + repoId,
          name     : 'Repository ' + repo.name + " (" + repo.path + ")",
          loaded   : true,
          type     : 'repo',
          parent   : 'root',
          nodeInfo : { repoId : repoId }
        });
          
        var branchId = 'repo_' + repoId + '_branches';
        that._store.put({
          id       : branchId,
          name     : 'Branches',
          loaded   : true,
          type     : 'branch',
          parent   : 'repo_' + repoId,
          nodeInfo : { repoId : repoId }
        });
        
        that.loadBranches(repo, branchId);
        
        var tagId = 'repo_' + repoId + '_tags';
        that._store.put({
          id       : tagId,
          name     : 'Tags',
          loaded   : true,
          type     : 'tag',
          parent   : 'repo_' + repoId,
          nodeInfo : { repoId : repoId }
        });
        
        that.loadTags(repoId, tagId);
        
//        var referenceList = model.versionservice.getReferenceList(repoId);
//        
//        referenceList.forEach(function (branchName) {
//          var star = repo.head === branchName ? '(*) ' : '';
//
//          // Add tree root of the branch
//          that._store.put({
//            id         : 'repo_' + repoId + '_branch_' + branchName,
//            name       : star + 'Commits in ' + branchName,
//            loaded     : false,
//            type       : 'commitlist',
//            parent     : 'repo_' + repoId,
//            nodeInfo   : { repoId : repoId, branchName : branchName }
//          });
//        });
      });
      
      this.loadNode(this._store.get('root'));
    },
    
    loadBranches : function(repo, branchId){
      var that = this;
      var repoId = repo.pathHash;
      var referenceList = model.versionservice.getBrancheList(repoId);
        
      referenceList.forEach(function (branchName) {
        // Add tree root of the branch
        that._store.put({
          id         : 'repo_' + repoId + '_branch_' + branchName,
          name       : 'Commits in ' + branchName,
          loaded     : false,
          type       : 'commitlist',
          ishead      : repo.head === branchName? true: false,
          parent     : branchId,
          nodeInfo   : { repoId : repoId, branchName : branchName }
        });
      });
    },
    
    loadTags : function(repoId, branchId){
      var that = this;
      var referenceList = model.versionservice.getTagList(repoId);
        
      referenceList.forEach(function (tagName) {
        // Add tree root of the branch
        that._store.put({
          id         : 'repo_' + repoId + '_tag_' + tagName,
          name       : 'Commits in ' + tagName,
          loaded     : false,
          type       : 'commitlist',
          class      : 'tag',
          parent     : branchId,
          nodeInfo   : { repoId : repoId, branchName : tagName }
        });
      });
    },
  
    /**
     * Loads a node dynamically.
     * 
     * Currently only commitlist objects have children that need dynamic
     * loading, they load the commits beneath them.
     */
    loadNode : function (node) {
      node.loaded = true;
      
      if (node.type === 'commitlist') {
        var branchTopObject = model.versionservice.getReferenceTopObject(
          node.nodeInfo.repoId,
          node.nodeInfo.branchName);
        
        if (branchTopObject.type === VersionObjectType.GIT_OBJ_COMMIT) {
          //branch view button
          this._store.put({
            id       : node.id + '_openbranchesview',
            name     : '(branches view)',
            loaded   : true,
            type     : 'openbranchesview',
            parent   : node.id,
            nodeInfo : {
              repoId       : node.nodeInfo.repoId,
              branchName   : node.nodeInfo.branchName
            }
          });

          //load commits
          this.loadCommits(
            node.nodeInfo.repoId,
            node.nodeInfo.branchName,
            branchTopObject.oid,
            0,
            node.id);
        } else if (branchTopObject.type === VersionObjectType.GIT_OBJ_TAG) {
          // Load tag
          var branchName = node.nodeInfo.branchName;
          var tag = model.versionservice.getTag(
            node.nodeInfo.repoId,
            branchTopObject.oid);
          
          // Put tag in navigator
          this._store.put({
            id       : node.id + '_tag' + branchTopObject.oid,
            name     : '(Annotated Tag) '  + tag.summary + ' ('   + tag.tagger + ')',
            loaded   : true,
            type     : 'annotatedtag',
            parent   : node.id,
            nodeInfo : {
              repoId       : node.nodeInfo.repoId,
              nextCommitId : branchTopObject.targetOid,
              branchName   : branchName,
              tooltip      : entities.encode(tag.message)
                                       .split("\n").join("<br/>") +
                               "<br/>(" + entities.encode(tag.tagger) + ")"
            }
          });

          // Load target commit of the tag
          this.loadCommits(
            node.nodeInfo.repoId,
            node.nodeInfo.branchName,
            tag.targetOid,
            0,
            node.id);
        } else {
          console.log("Branch type: " + branchTopObject.type);
          alert("This branch is not a commit and non-commit branches are not yet supported.");
        }
      }
    },
    
    /**
     * Loads commits dynamically either under the reference or
     * in place of the load more commits list item.
     */
    loadCommits : function (repoId, branchName, topCommit, offset, parentid) {
      var that = this;
      
      var NUMBER_OF_COMMITS_TO_LOAD = 15;
      
      var gclfr = model.versionservice.getCommitListFiltered(
        repoId,
        topCommit,
        NUMBER_OF_COMMITS_TO_LOAD,
        offset,
        this._filterText);
      
      var commits = gclfr.result;
      
      var nextCommitId = null;
      
      commits.forEach(function (currCommit) {
        nextCommitId = currCommit.parentOids[0];
        
        that._store.put({
          id       : parentid + '_commit' + currCommit.oid,
          name     : currCommit.summary + ' (' + currCommit.author + ')',
          loaded   : true,
          type     : 'commit',
          parent   : parentid,
          nodeInfo : {
            repoId  : currCommit.repoId,
            oid     : currCommit.oid,
            treeOid : currCommit.treeOid,
            tooltip : entities.encode(currCommit.message)
                                .split("\n").join("<br/>") +
                        "<br/>(" + entities.encode(currCommit.author) + ")"
          }
        });
      });
      
      // Check length as well because of the filtering
      if (gclfr.hasRemaining) {
        this._store.put({
          id       : parentid + '_commitloadmore',
          name     : 'More',
          loaded   : false,
          type     : 'commitloadmore',
          parent   : parentid,
          nodeInfo : {
            repoId       : repoId,
            nextCommitId : nextCommitId,
            topCommitId  : topCommit,
            branchName   : branchName,
            offset       : gclfr.newOffset
          }
        });
      }
    },

    postCreate : function () {
      this.addChild(this._searchBox);
      this.addChild(this._tree);
      this._tree.startup();
    }
  });
});
