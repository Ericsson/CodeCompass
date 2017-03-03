define([
  'dijit/Tooltip',
  'dijit/tree/ObjectStoreModel',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'codecompass/view/component/HtmlTree',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (Tooltip, ObjectStoreModel, declare, Memory, Observable, topic,
  HtmlTree, model, viewHandler, util) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  /**
   * This function creates a html label for a git commit.
   * @param {GitCommit} commit Thrift git commit object.
   * @return HTML Commit label.
   */
  function createLabel(commit) {
    var avatarLabel = commit.author.charAt(0).toUpperCase();
    return '<div class="git-commit">'
      + '<div class="git-avatar" style="background-color:'
      + util.strToColor(commit.author) + '">' + avatarLabel + '</div>'
      + '<div class="git-message">' + commit.summary + '</div>'
      + '<div class="git-author">' + commit.author + '</div>'
      + '</div>';
  }

  /**
   * This functions creates a html tooltip message for a git commit.
   * @param {GitCommit} commit Thrift git commit object.
   * @return HTML Commit tooltip content.
   */
  function createTooltip(commit) {
    var time = util.timeAgo(new Date(commit.time * 1000));
    return '<div class="git-commit-tooltip">'
      + '<div class="git-sha">#' + commit.oid.substr(0,8) + '</div>'
      + '<div class="git-message">' + commit.message + '</div>'
      + '<div class="commit-meta">'
      +   '<div class="git-author">' + commit.author + '</div>'
      +   '<div class="git-committed-on"> committed on '
      +     '<span class="git-time">' + time  + '</span></div>'
      + '</div></div>';
  }

  /**
   * VersionNavigator is the navigation panel on the left side that shows
   * repositories commits and branches.
   */
  var GitNavigator = declare(HtmlTree, {
    /**
     * Number of commits which has to be loaded one time when clicking on More
     * button.
     */
    _numOfCommitsToLoad  : 15,

    constructor : function () {
      var that = this;
 
      //--- Store and model ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          return node.getChildren ? node.getChildren(node) : [];
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

      this._data.push({
        id          : 'root',
        name        : 'List of repositories',
        cssClass    : 'icon-list',
        hasChildren : true,
        getChildren : function () {
          return that._store.query({ parent : 'root' });
        }
      });

      this.set('model', dataModel);
      this.set('openOnClick', false);

      model.gitservice.getRepositoryList().forEach(function (repo) {

        //--- Repository node ---//

        that._store.put({
          id          : repo.id,
          name        : 'Repository ' + repo.id + " (" + repo.path + ")",
          cssClass    : 'icon-repository',
          hasChildren : true,
          loaded      : true,
          parent      : 'root',
          getChildren : function () {
            var ret = [];

            //--- Branches parent node ---//

            ret.push({
              id          : repo.id + '_branches',
              name        : 'Branches',
              cssClass    : 'icon-branch',
              hasChildren : true,
              getChildren : function () {
                return that.getBranches(repo);
              }
            });

            //--- Tags parent node ---//

            ret.push({
              id          : repo.id + '_tags',
              name        : 'Tags',
              cssClass    : 'icon-tag',
              hasChildren : true,
              getChildren : function () {
                return that.getTags(repo);
              }
            });

            return ret;
          }
        });
      });
    },

    /**
     * Shows a tooltip for a tree node if we set the tooltip message.
     */
    _onNodeMouseEnter : function (node, evt) {
      if (node.item.tooltip)
        Tooltip.show(node.item.tooltip, node.labelNode, ['above']);
    },

    /**
     * Hide the tooltip.
     */
    _onNodeMouseLeave : function (node, evt) {
      Tooltip.hide(node.labelNode);
    },

    onClick : function (item, node, event) {
      if (item.onClick)
        item.onClick(item, node, event);
      else if (item.hasChildren)
        this._onExpandoClick({node: node});
    },

    /**
     * Overridable function to return CSS class name to display icon.
     */
    getIconClass : function (item, opened) {
      var baseClass = this.inherited(arguments);

      if (item.cssClass)
        return 'icon ' + item.cssClass;

      return baseClass;
    },

    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if ( state.center !== 'gitcommitview' ||
          !state.gitrepo ||
          !state.gitcommit ||
          !state.gitbranch)
        return;

      topic.publish('codecompass/selectAccordion', this.id);

      var that = this;

      //--- Search the current commit in the tree ---//

      var path = ['Repository ' + state.gitrepo];
      var branchName = state.gitbranch;
      if (branchName) {
        branchName.indexOf('refs/tags') === -1 
          ? path.push('Branches')
          : path.push('Tags');

        path.push(branchName);
      }

      var currentNode = this.getChildren()[0];
      path.forEach(function (directory) {
        children = currentNode.getChildren();
        index = util.findIf(children, function (child) {
          return child.label.indexOf(directory) !== -1;
        });
        currentNode = children[index];
        that._expandNode(currentNode);
      });

      var commit = model.gitservice.getCommit(state.gitrepo, state.gitcommit);

      if (!commit.oid)
        return;

      var index = -1;
      do {
        children = currentNode.getChildren();
        index = util.findIf(children, function (child) {
          return child.item.oid === commit.oid;
        });

        if (index === -1) {
          var moreBtn = children[children.length - 1];

          // We have loaded all the commits and there is no more 'More' button.
          if (moreBtn.item.name !== 'More')
            return;

          moreBtn.item.onClick();
        } else {
          children[index].setSelected(true);
        }
      } while(index === -1);
    },

    /**
     * This function load branches for an existing repo.
     * @param {GitRepository} repo Thrift git repository object.
     * @return Branches of the repository.
     */
    getBranches : function (repo) {
      var that = this;

      var ret = [];

      model.gitservice.getBranchList(repo.id).forEach(function (branchName) {
        var branchCommitsId = repo.id + '_branches_' + branchName;
        ret.push({
          id          : branchCommitsId,
          name        : 'Commits in ' + branchName,
          cssClass    : repo.head == branchName
                      ? 'icon-head' 
                      : 'icon-commits-in',
          hasChildren : true,
          getChildren : function (node) {
            var topObj = model.gitservice.getReferenceTopObject(
              repo.id, branchName);

            if (topObj.type === GitObjectType.GIT_OBJ_COMMIT) {
              that._data.push({
                id          : branchCommitsId + "_view",
                name        : '(branches view)',
                cssClass    : 'icon-branchview',
                parent      : branchCommitsId,
                hasChildren : false
              });

              that.loadCommits(repo.id, branchName, topObj.oid, 0,
                branchCommitsId);
            }
            return that._store.query({ parent : branchCommitsId });
          }
        });
      });

      return ret;
    },

    /**
     * This function returns tags for the repo.
     * @param {GitRepository} repo Thrift git repository object.
     * @return Tags of the repository.
     */
    getTags : function (repo) {
      var that = this;

      var ret = [];

      model.gitservice.getTagList(repo.id).forEach(function (tagName) {
        var tagCommitsId = repo.id + '_tags_' + tagName;
        ret.push({
          id          : tagCommitsId,
          name        : 'Commits in ' + tagName,
          cssClass    : 'icon-commit',
          hasChildren : true,
          getChildren : function () {
            var topObj = model.gitservice.getReferenceTopObject(
              repo.id, tagName);

            if (topObj.type === GitObjectType.GIT_OBJ_TAG) {
              var tag = model.gitservice.getTag(repo.id, topObj.oid);

              that._data.push({
                id          : tagCommitsId + "_view",
                name        : '(Annotated Tag) '  + tag.summary 
                            + ' ('   + tag.tagger + ')',
                cssClass    : 'icon-branchview',
                parent      : tagCommitsId,
                hasChildren : false
              });

              that.loadCommits(repo.id, tagName, topObj.oid, 0, tagCommitsId);
            }
            return that._store.query({ parent : tagCommitsId });
          }
        });
      });

      return ret;
    },

    /**
     * Loads commits dynamically either under the reference or in place of the
     * load more commits list item.
     */
    loadCommits : function (repoId, branchName, topCommit, offset, parentid) {
      var that = this;

      var filteredCommits = model.gitservice.getCommitListFiltered(repoId,
        topCommit, this._numOfCommitsToLoad, offset, this._filterText);

      //--- Add commits to the store ---//

      filteredCommits.result.forEach(function (commit) {
        that._store.put({
          id            : parentid + '_commit' + commit.oid,
          oid           : commit.oid,
          parent        : parentid,
          name          : createLabel(commit),
          cssClass      : 'icon-commit',
          tooltip       : createTooltip(commit),
          hasChildren   : false,
          onClick       : function (item, node, event) {
            topic.publish('codecompass/gitCommitView', {
              center     : 'gitcommitview',
              gitrepo    : repoId,
              gitcommit  : commit.oid,
              gitbranch  : branchName
            });
          }
        });
      });

      //--- Remove previous more button if exists ---//

      var moreBtn = this._store.query({name : 'More', parent : parentid})[0];
      if (moreBtn)
        this._store.remove(moreBtn.id);

      //--- Add more button at the end of the store ---//

      if (filteredCommits.hasRemaining) {
        that._store.put({
          id          : parentid + '_more',
          name        : 'More',
          parent      : parentid,
          cssClass    : 'icon-node',
          hasChildren : true,
          onClick     : function () {
            that.loadCommits(repoId, branchName, topCommit,
              filteredCommits.newOffset, parentid);
          }
        });
      } else {
        // TODO: If no item has remaining, set the focus to the last element.
      }
    },
  });

  var navigator = new GitNavigator({
    id    : 'gitnavigator',
    title : 'Revision Control Navigator'
  });

  viewHandler.registerModule(navigator, {
    type : viewHandler.moduleType.Accordion
  });

});
