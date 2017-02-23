define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojo/dom-style',
  'dojo/date/locale',
  'dijit/layout/ContentPane',
  'dijit/layout/LayoutContainer',
  'dijit/layout/TabContainer',
  'codecompass/view/component/GitDiff',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/util'],
function (declare, dom, topic, style, locale, ContentPane, LayoutContainer,
  TabContainer, GitDiff, model, viewHandler, urlHandler, util) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  var VisualDiff = declare(ContentPane, {
    constructor : function () {
      this._header = new ContentPane();
      this._diff   = new GitDiff();
    },

    postCreate : function () {
      this.addChild(this._header);
      this.addChild(this._diff);
    },

    createHeader : function (repoId, commitId, branchId) {
      dom.empty(this._header.domNode);

      var commit = model.gitservice.getCommit(repoId, commitId);

      //--- Commit header ---//

      var header = dom.create('div', { class : 'header'}, this._header.domNode);
      dom.create('div', { class : 'message', innerHTML: commit.message }, header);

      //--- Commit meta information ---//

      var meta = dom.create('div', { class : 'meta'}, header);

      //--- Author and author avatar ---//

      var avatarLabel = commit.author.charAt(0).toUpperCase();
      var avatar =
        dom.create('div', { class : 'avatar', innerHTML: avatarLabel}, meta);
      style.set(avatar, 'background-color', util.strToColor(commit.author));

      dom.create('div', { class : 'author', innerHTML: commit.author }, meta);

      dom.create('span', { innerHTML: 'committed on ' }, meta);
      var time = util.timeAgo(new Date(commit.time * 1000));
      dom.create('span', { class : 'time',  innerHTML: time }, meta);

      //--- Parent and current commit ids ---//

      var shaBlocks = dom.create('div', { class : 'sha-blocks'}, meta);

      var parentLabel = commit.parentOids.length + ' parent ';
      dom.create('span', { innerHTML: parentLabel}, shaBlocks);
      commit.parentOids.forEach(function (parentId) {
        dom.create('span', {
          class : 'parent',
          innerHTML: parentId.substr(0,8),
          onclick : function () {
            var that = this;
            topic.publish('codecompass/gitCommitView', {
              center     : 'gitcommitview',
              gitrepo    : repoId,
              gitcommit  : parentId,
              gitbranch  : branchId
            });
          }
        }, shaBlocks);
      });

      dom.create('span', { innerHTML: 'commit '}, shaBlocks);
      dom.create('span', { class : 'sha', innerHTML: commit.oid.substr(0,8) },
        shaBlocks);
    },

    init : function (repoId, commitId, branchId) {
      this.createHeader(repoId, commitId, branchId);
      this._diff.loadDiffTable(repoId, commitId, branchId);
    },
  });

  var GitCommitView = declare(TabContainer, {
    constructor : function () {
      var that = this;

      this._vdiff = new VisualDiff({
        id     : 'git-visutaldiff',
        title  : 'Visual diff'
      });

      this._subscribeTopics();
    },

    postCreate : function () {
      this.addChild(this._vdiff);
    },

    loadCommitView : function (repoId, commitId, branchId) {
      this._vdiff.init(repoId, commitId, branchId);
    },

    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if ( state.center !== this.id || !state.gitrepo || !state.gitcommit)
        return;

      this.loadCommitView(state.gitrepo, state.gitcommit, state.gitbranch);
    },

    _subscribeTopics : function () {
      var that = this;

      topic.subscribe('codecompass/gitCommitView', function (message) {
        that.loadCommitView(message.gitrepo, message.gitcommit,
          message.gitbranch);

        topic.publish('codecompass/setCenterModule', that.id);

        urlHandler.setStateValue({
          center    : that.id,
          gitrepo   : message.gitrepo,
          gitcommit : message.gitcommit,
          gitbranch : message.gitbranch
        });
      });
    },
  });

  var commitView = new GitCommitView({id : 'gitcommitview'});
  viewHandler.registerModule(commitView, {
    type : viewHandler.moduleType.Center
  });
});