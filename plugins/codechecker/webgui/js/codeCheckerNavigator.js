define([
  'dijit/Tooltip',
  'dijit/tree/ObjectStoreModel',
  'dijit/layout/BorderContainer',
  'dijit/MenuItem',
  'dijit/form/Button',
  'dijit/form/Form',
  'dijit/form/Textarea',
  'dijit/Dialog',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dojo/mouse',
  'dojo/on',
  'dojo/query',
  'dojox/html/entities',
  'codecompass/model',
  'codecompass/view/component/ContextMenu',
  'codecompass/viewHandler',
  'codecompass/view/component/HtmlTree'],
function (Tooltip, ObjectStoreModel, BorderContainer, MenuItem,
  Button, Form, Textarea, Dialog, dom, style, declare, Memory, Observable,
  topic, mouse, on, query, entities, model, ContextMenu, viewHandler, HtmlTree) {

  model.addService('codecheckerservice', 'CodeCheckerService', codeCheckerDBAccess.codeCheckerDBAccessClient);

  /**
   * Converts a Thrift API severity id to human readable string.
   *
   * @param {String|Number} severityCode Thrift API Severity id
   * @return Human readable severity string.
   */
  severityFromCodeToString = function (severityCode) {
    if (severityCode === 'all')
      return 'All';

    for (var key in Severity)
      if (Severity[key] === parseInt(severityCode))
        return key.toLowerCase();
  }

  /**
   * CodeChecker navigation model
   */
  var BugStoreModelTree = declare(HtmlTree, {
    constructor : function () {
      var that = this;

      this._bugStore = new Observable(new Memory({
        data        : [{ id : 'root' }],
        getChildren : function (node) {
          return this.query({ parent : node.id });
        }
      }));

      this.model = new ObjectStoreModel({
        store           : that._bugStore,
        query           : { id : 'root' },
        mayHaveChildren : function (item) {
          return item.hasChildren;
        }
      });

      this._subscribeTopics();
    },

    postCreate : function () {
      this.inherited(arguments);
      this.loadBugStoreData();
    },

    setState : function (state) {
      if (!state.fid)
        return;

      this._fileId = state.fid;
    },

    /**
     * Create severity levels and gets there report data.
     */
    loadBugStoreData : function () {
      var that = this;

      [
        { id : 'root', name : 'Bugs by priority', cssClass : 'icon icon-bug' },
        { id : 'critical', name : 'Critical', parent : 'root', hasChildren : true },
        { id : 'high', name : 'High', parent : 'root', hasChildren : true },
        { id : 'medium', name : 'Medium', parent : 'root', hasChildren : true },
        { id : 'low', name : 'Low', parent : 'root', hasChildren : true },
        { id : 'style', name : 'Style', parent : 'root', hasChildren : true },
        { id : 'unspecified', name : 'Unspecified', parent : 'root', hasChildren : true }
      ].forEach(function (item) {
        item.cssClass = item.cssClass 
          ? item.cssClass 
          : 'icon icon-severity icon-severity-' + item.id;
        that._bugStore.put(item);
      });

      var filter_sup = new codeCheckerDBAccess.ReportFilter();
      filter_sup.suppressed = true;

      var filter_unsup = new codeCheckerDBAccess.ReportFilter();
      filter_unsup.suppressed = false;

      var NUMBER_OF_BUGS_TO_LOAD = 15;
      
      model.codecheckerservice.getRunResults(
        -1,
        NUMBER_OF_BUGS_TO_LOAD,
        0,
        [],
        [filter_sup, filter_unsup],
        function (result) {
          result.forEach(function (report) { 
            that._addReport(report, severityFromCodeToString(report.severity)); 
            });

          that._bugStore.query({ parent : 'root' }).forEach(function (node) {
            if(node.id === 'bugsinfile')
              return;

            if (that._bugStore.query({ parent : node.id }).length === 0)
              that._bugStore.remove(node.id);
          });
        });
    },

    onClick : function (item, node, event) {
      var that = this;

      if (node.isExpanded)
        this._collapseNode(node);
      else
        this._expandNode(node);

      if (item.hasChildren && (!item.type || item.type !== 'resultnode'))
        return;

      if (item.bugPathEvent) {
        var fileId = item.bugPathEvent.fileId;
        var line = item.bugPathEvent.startLine;
        var column = item.bugPathEvent.startCol;
      } else {
        var fileId = item.report.fileId;
        var line = item.report.lastBugPosition.startLine;
        var column = item.report.lastBugPosition.startCol;
      }

      var bugviewer = viewHandler.getModule('bugviewer');

      if (bugviewer.reportData == undefined ||
          bugviewer.reportData.reportId != item.parent) {
        bugviewer.set('reportData', item.report);
      }

      bugviewer.set(
        'sourceFileData',
        model.codecheckerservice.getSourceFileData(fileId, true));

      if (item.bugPathEvent)
        bugviewer.highlightBugPathEvent(item.bugPathEvent);

      topic.publish('codecompass/openBugs', item.report.checkedFile);
    },

    getIconClass : function (item, opened) {
      if(item.cssClass)
        return item.cssClass;

      return this.inherited(arguments)
    },

    /**
     * Add report results.
     */
    _addReport : function (report, parentNodeId) {
      var that = this;

      var path = report.checkedFile;
      var file = path.substr(path.lastIndexOf('/') + 1);

      var checkerNodeId = parentNodeId + report.checkerId;
      this._bugStore.put({
        id          : checkerNodeId,
        name        : report.checkerId,
        parent      : parentNodeId,
        report      : report,
        hasChildren : true
      });

      var resultNodeId = checkerNodeId + report.reportId;
      this._bugStore.put({
        id          : resultNodeId,
        name        : '<span class="cc-checker-checkedfile">'
                    + file + ':' + report.lastBugPosition.startLine 
                    + '</span>' 
                    + ' - '
                    + ' <span class="cc-checker-reportmessage">' 
                    + report.checkerMsg
                    + '</span>',
        parent      : checkerNodeId,
        report      : report,
        type        : 'resultnode',
        hasChildren : true,
        cssClass    : 'icon icon-node'
      });

      model.codecheckerservice.getReportDetails(report.reportId, 
        function (reportDetails) {
          reportDetails.pathEvents.forEach(function (step, index) {
            that._bugStore.put({
              id           : resultNodeId + '_' + (index + 1),
              name         : 'Line ' + step.startLine + ': ' + step.msg,
              parent       : resultNodeId,
              bugPathEvent : step,
              hasChildren  : false,
              cssClass     : 'icon icon-bug',
              report       : report
            });
          });
      });
    },

    /**
     * Load CodeChecker bugs in a file.
     * @param {FileInfo} fileInfo File Info object
     */
    loadBugsInFile : function(fileInfo){
      var that = this;

      if (this.fileId && this.fileId === fileInfo.id)
        return;

      this.fileId = fileInfo.id;

      this._bugStore.query({ parent : 'bugsinfile' }).forEach(function (node) {
        that._bugStore.remove(node.id);
      });

      this._bugStore.remove('bugsinfile');

      this._bugStore.put({
        id          : 'bugsinfile',
        type        : 'bugsinfile',
        name        : 'Bugs in ' + fileInfo.path,
        hasChildren : true,
        parent      : 'root',
        loaded      : false
      });

      //--- Load errors for the file ---//

      var filter = new codeCheckerDBAccess.ReportFilter();
      filter.filepath = fileInfo.path;

      model.codecheckerservice.getRunResults(
        -1,
        15,
        0,
        [],
        [filter],
        function (result) {
          result.forEach(function (report) { that._addReport(report, 'bugsinfile'); });
        });
    },

    _subscribeTopics : function(){
      var that = this;

      topic.subscribe('codecompass/openFile', function (message) {
        var fileInfo = model.project.getFileInfo(message.fileId);
        that.loadBugsInFile(fileInfo);
      });
      
      topic.subscribe('codecompass/openBugs', function (checkedFilePath) {
        var fileInfo = model.project.getFileInfoByPath(checkedFilePath);
        that.loadBugsInFile(fileInfo);
      });
    },

    _onNodeMouseEnter : function (node) {
      if (node.item.hasChildren)
        Tooltip.show(node.item.name, node.labelNode, ['above']);
    },

    _onNodeMouseLeave : function (node) {
      if (node.item.hasChildren)
        Tooltip.hide(node.labelNode);
    }
  });

  //--- CodeChecker navigatior ---//

  var CodeCheckerNavigator = declare(BorderContainer, {
    constructor : function () {
      this._bugStoreModelTree = new BugStoreModelTree({
        region      : 'center',
        openOnClick : false,
        showRoot    : false
      });
    },
    postCreate : function () {
      this.addChild(this._bugStoreModelTree);
    }
  });

  var codeCheckerNavigator = new CodeCheckerNavigator({
    id    : 'codecheckernavigator',
    title : 'CodeChecker'
  });

  viewHandler.registerModule(codeCheckerNavigator, {
    type    : viewHandler.moduleType.Accordion,
    service : model.codecheckerservice
  });
});

