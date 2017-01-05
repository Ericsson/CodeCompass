define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojox/html/entities',
  'dijit/layout/ContentPane',
  'dijit/layout/LayoutContainer',
  'dijit/layout/TabContainer',
  'codecompass/view/component/Editor',
  'codecompass/view/component/VersionTree',
  'codecompass/view/component/VersionDiff',
  'codecompass/registration/model'],
function (declare, dom, topic, entities, ContentPane, LayoutContainer,
  TabContainer, Editor, VersionTree, VersionDiff, model) {

  function buildSummaryTable(domNode, commitDetails) {
    dom.empty(domNode);
    
    var tableDomNode, tableRowNode;
    
    tableDomNode = dom.create("table", {}, domNode);
    tableRowNode = dom.create("tr", {}, tableDomNode);
    
    dom.create("td", { innerHTML: "<b>Message:</b>" }, tableRowNode);
    dom.create("td", { innerHTML: entities.encode(commitDetails.message).split("\n").join("<br/>") }, tableRowNode);
    
    tableRowNode = dom.create("tr", {}, tableDomNode);
    
    dom.create("td", { innerHTML: "<b>Author:</b>" }, tableRowNode);
    dom.create("td", { innerHTML: entities.encode(commitDetails.author) }, tableRowNode);
    
    if (commitDetails.author != commitDetails.committer) {
      tableRowNode = dom.create("tr", {}, tableDomNode);
      
      dom.create("td", { innerHTML: "<b>Committer:</b>" }, tableRowNode);
      dom.create("td", { innerHTML: entities.encode(commitDetails.committer) }, tableRowNode);           
    }
    
    tableRowNode = dom.create("tr", {}, tableDomNode);
    
    dom.create("td", { innerHTML: "<b>Id:</b>" }, tableRowNode);
    dom.create("td", { innerHTML: entities.encode(commitDetails.oid) }, tableRowNode);
    
    var date = new Date(commitDetails.time * 1000);
    
    tableRowNode = dom.create("tr", {}, tableDomNode);
    
    dom.create("td", { innerHTML: "<b>Time:</b>" }, tableRowNode);
    dom.create("td", { innerHTML: entities.encode("" + date) }, tableRowNode);
  }
  
  function initializeVersionCommitComponent(
    versionCommitPanel, versionrepoid, versioncommitid) {
    
    var commitDetails =
      model.versionservice.getCommit(versionrepoid, versioncommitid);
    
    buildSummaryTable(
      versionCommitPanel._vdiff_summary.domNode,
      commitDetails);
    
    var fileInfo = {
      path        : '/diff of commit ' + versioncommitid + '/',
      parseStatus : model.FileParseStatus.Nothing
    };
    
    versionCommitPanel._diff_editor.setHeader(fileInfo);
    versionCommitPanel._diff_editor.resize();
    
    var vdo = new model.VersionDiffOptions();
    vdo.contextLines = 3;

    var commitDiffStr = model.versionservice.getCommitDiffAsStringCompact(
      versionrepoid,
      versioncommitid,
      vdo);
    
    versionCommitPanel._diff_editor.set('content', commitDiffStr);
    versionCommitPanel._vdiffActual.setContent(commitDiffStr, commitDetails, vdo);
    versionCommitPanel._browse.setRootCommit(commitDetails);
  }
  
  return declare(TabContainer, {
    constructor : function () {
      var that = this;

      this._vdiff = new ContentPane({
        id    : 'vdiff',
        title : 'Visual diff',
        style : 'padding: 10px'
      });
      
      this._vdiff_summary = new ContentPane({
        id : 'summary_vdiff'
      });
      
      this._vdiffActual = new VersionDiff({
        id : 'vdifff'
      });

      this._diff_summary = new ContentPane({
        id     : 'summary_diff',
        region : 'top'
      });

      this._diff = new LayoutContainer({
        id    : 'diff',
        title : 'Diff'
      });

      this._browse = new VersionTree({
        id    : 'browse',
        title : 'Browse',
        style : 'padding: 10px'
      });
      
      //--- Editor ---//

      this._diff_editor = new Editor({
        region : 'center',
      });
      
      this._diff_editor.set('mode', 'text/x-diff');
      this._diff_editor.set('content', '(diff will appear here)');
      
      //--- Messaging ---//

      topic.subscribe('codecompass/versionOpenCommit', function (message) {
        if (message.newTab)
          return;

        if (message.resetopentab)
          that.selectChild(that._vdiff);

        initializeVersionCommitComponent(
          that,
          message.versionrepoid,
          message.versioncommitid
        );
      });
    },

    postCreate : function () {
      this.addChild(this._vdiff);
      this.addChild(this._diff);
      this.addChild(this._browse);
      this._vdiff.addChild(this._vdiff_summary);
      this._vdiff.addChild(this._vdiffActual);
      this._diff.addChild(this._diff_summary);
      this._diff.addChild(this._diff_editor);
      
      // This is needed for the active tab to initially appear
      this.startup();
    },
    
    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if (state.cent !== this.id)
        return;
      
      var versionrepoid = state.versionrepoid || "";
      var versioncommitid = state.versioncommitid || "";

      initializeVersionCommitComponent(this, versionrepoid, versioncommitid);
    },
  });
});