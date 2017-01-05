define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/mouse',
  'dojo/on',
  'dojo/topic',
  'dojox/html/entities',
  'dijit/layout/ContentPane',
  'dijit/layout/LayoutContainer',
  'dijit/layout/TabContainer',
  'dijit/form/Button',
  'dijit/form/RadioButton',
  'codecompass/registration/model',
  'codecompass/registration/versionCommon',
  'codecompass/view/component/VersionDiff',
  'dojo/date/locale',
  'dijit/Tooltip'
],
function (declare, dom, mouse, on, topic, entities, ContentPane, LayoutContainer,
  TabContainer, Button, RadioButton, model, versionCommon, VersionDiff,
  locale, Tooltip) {
  
  
  
  function initializeVersionHistoryDiffComponent(
    container,
    versionrepoid,
    versioncommitfromid,
    versioncommitid,
    versionpath)
  {
    var hasPath = versionpath ? true : false;
    
    //load commit details
    var fromCommitDetails = model.versionservice.getCommit(versionrepoid, versioncommitfromid);
    var commitDetails = model.versionservice.getCommit(versionrepoid, versioncommitid);
    
    dom.empty(container._histdiffSummary.domNode);
    var topParagraph = dom.create("p", { innerHTML:
      "Difference " +
      (hasPath ? "of file <b>" + entities.encode(versionpath) + "</b> " : "") +
      'between commit '/* + versionCommon.formatId(versioncommitfromid) + " " +
      'and commit ' + versionCommon.formatId(versioncommitid) + ""*/
      }, container._histdiffSummary.domNode);
    versionCommon.placeCommitLink(fromCommitDetails, topParagraph);
    dom.create("span", { innerHTML: ' and commit ' }, topParagraph);
    versionCommon.placeCommitLink(commitDetails, topParagraph);
    
    
    var vdo = new model.VersionDiffOptions();
    vdo.contextLines = 3;
    vdo.fromCommit = versioncommitfromid;
    vdo.pathspec = [ versionpath ];
    
    var commitDiffStr = model.versionservice.getCommitDiffAsStringCompact(
      versionrepoid,
      versioncommitid,
      vdo);
    container._histdiffActual.setContent(commitDiffStr, commitDetails, vdo);
  }

  return declare(TabContainer, {
    constructor : function () {
      var that = this;

      this._histdiff = new ContentPane({
        id    : 'histdiff',
        title : 'History Diff',
        style : 'padding: 10px'
      });
      
      this._histdiffSummary = new ContentPane({
        id : '_histdiffSummary'
      });
      
      this._histdiffActual = new VersionDiff({
        id : '_histdiffActual'
      });

      topic.subscribe('codecompass/versionOpenHistoryDiff', function (message) {
        if (message.newTab)
          return;

        initializeVersionHistoryDiffComponent(
          that,
          message.versionrepoid,
          message.versioncommitfromid,
          message.versioncommitid,
          message.versionpath
        );
      });
      
    },

    postCreate : function () {
      this._histdiff.addChild(this._histdiffSummary);
      this._histdiff.addChild(this._histdiffActual);
      this.addChild(this._histdiff);
      
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
      
      var versionrepoid       = state.versionrepoid       || "";
      var versioncommitfromid = state.versioncommitfromid || "";
      var versioncommitid     = state.versioncommitid     || "";
      var versionpath         = state.versionpath         || "";

      initializeVersionHistoryDiffComponent(
        this,
        versionrepoid,
        versioncommitfromid,
        versioncommitid,
        versionpath);
    },
  });
});