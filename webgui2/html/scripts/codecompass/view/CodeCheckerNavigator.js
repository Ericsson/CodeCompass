define([
  'dijit/Tooltip',
  'dijit/Tree',
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
  'codecompass/registration/model',
  'codecompass/view/component/ContextMenu',
  'codecompass/viewHandler'],
function (Tooltip, Tree, ObjectStoreModel, BorderContainer, MenuItem,
  Button, Form, Textarea, Dialog, dom, style, declare, Memory, Observable,
  topic, mouse, on, query, entities, model, ContextMenu, viewHandler) {

  var NavTree = declare(Tree, {
    getIconClass : function (item, opened) {
      switch (item.type) {
        case 'bugloadmore' : return 'dijitFolderOpened';
        case 'bug'         : return 'bugreporticon';
        case 'bugstep'     : return 'bugstepicon';
        default            : return this.inherited(arguments);
      }
    },

    _onNodeMouseEnter : function (node, evt) {
      if (node.item.nodeInfo) {
        Tooltip.show(
          node.item.nodeInfo.tooltip || entities.encode(node.item.name),
          node.domNode, ['above']);
      }
    },

    _onNodeMouseLeave : function (node, evt) {
      Tooltip.hide(node.domNode);
    }    
  });
  
  function drawBugPath(points, bubbles, fid) {
    function filt(node) { return node.fileId === fid; }
    
    var textModule = viewHandler.getModule('text');

    textModule.clearLines();
    textModule.clearBubbles();
    textModule.addBubbles(bubbles.filter(filt));
    textModule.addLines(points.filter(filt));
  }
  
  function itemClicked(checkerNavigator, item, newTab) {
    if (item.loaded)
      return;
    
    item.loaded = true;
    
    switch (item.type) {
      case 'bug':
        item.loaded = false; // So we can click many times on it

        var lastBugPosition = item.nodeInfo.bugInfo.lastBugPosition;

        var bugFrom = new model.Position();
        bugFrom.line = lastBugPosition.startLine;
        bugFrom.column = lastBugPosition.startCol;

        var bugTo = new model.Position();
        bugTo.line = lastBugPosition.endLine;
        bugTo.column = lastBugPosition.endCol + 1;

        var fileInfo = model.project.getFileInfoByPath(
          item.nodeInfo.bugInfo.checkedFile);

        var info
          = "Bug: "    + item.nodeInfo.bugInfo.checkerMsg
          + ", file: " + fileInfo.path
          + ", line: " + lastBugPosition.startLine;

        topic.publish('codecompass/openFile', {
          fileId     : fileInfo.file,
          line       : lastBugPosition.startLine,
          moduleId   : 'text',
          newTab     : newTab,
          info       : info,
          newSession : true,
          selection  : {
            from : bugFrom,
            to   : bugTo
          }
        });
        
        if (!item._points) {
          var reportDetails = model.codecheckerservice.getReportDetails(
            item.nodeInfo.bugInfo.reportId);

          item._points  = reportDetails.executionPath;
          item._bubbles = reportDetails.pathEvents;

          checkerNavigator.loadSteps(item._points, item);

          // This is needed because CodeChecker gives different positions.
          item._points.forEach (function (point ) { --point.startCol;  });
          item._bubbles.forEach(function (bubble) { --bubble.startCol; });
        }
  
        drawBugPath(item._points, item._bubbles, item.nodeInfo.bugInfo.fileId);
        
        style.set(checkerNavigator.contextButton.domNode, 'display', 'block');
        break;
        
      case 'bugstep':
        item.loaded = false; // So we can click many times on it
        
        var bugFrom = new model.Position();
        bugFrom.line = item.stepInfo.startLine;
        bugFrom.column = item.stepInfo.startCol;

        var bugTo = new model.Position();
        bugTo.line = item.stepInfo.endLine;
        bugTo.column = item.stepInfo.endCol + 1;
        
        var parent = checkerNavigator._store.get(item.parent);
        
        topic.publish('codecompass/openFile', {
          fileId     : item.stepInfo.fileInfo.file,
          line       : item.stepInfo.startLine,
          moduleId   : 'text',
          newTab     : newTab,
          info       : item.name,
          newSession : true,
          selection  : {
            from : bugFrom,
            to   : bugTo
          }
        });
        
        drawBugPath(parent._points, parent._bubbles, item.stepInfo.fileId);
          
        break;
        
      case 'bugloadmore':
        var parentNode
          = checkerNavigator._store.query({ id : item.parent })[0];
        checkerNavigator._store.remove(parentNode.id + '_bugloadmore');
        checkerNavigator.loadBugs(
          item.nodeInfo.offset, item.nodeInfo.filter, parentNode);
        break;
        
      case 'bugsinfile':
        // Bugs under this node are loaded in onShow() method
        break;
        
      case 'severity':
        var filter = new codeCheckerDBAccess.ReportFilter();
        filter.severity = item.severityId;
        model.codecheckerservice.getRunResultTypes(-1, [filter]).forEach(
        function (elem) {
          checkerNavigator._store.put({
            id       : item.severityId + "_" + elem.checkerId,
            type     : 'checkerid',
            name     : elem.checkerId + ' (' + elem.count + ')',
            parent   : item.id,
            loaded   : false,
            nodeInfo : { bugCount : elem.count }
          });
        });
        break;
        
      case 'checkerid':
        var filter = new codeCheckerDBAccess.ReportFilter();
        var severity = item.id.split('_')[0];
        var id = item.id.split('_')[1];
        filter.checkerId = id;
        filter.severity = severity;
        checkerNavigator.loadBugs(0, filter, item);
        break;
    }
  }
  
  function loadErrorsForFile(navigator, force) {
    
    //--- Load file info ---//

    if (!force &&
        (navigator._fileInfo && navigator._fileId.fid === navigator._fileInfo.file.fid ||
        !navigator._fileId))
      return;

    try {
      navigator._fileInfo = model.project.getFileInfo(navigator._fileId);
    } catch (ex) {
      console.warn('Invalid file id', navigator._fileId);
      return;
    }

    //--- Load errors for the file ---//

    navigator._store.query({ parent : 'bugsinfile' }).forEach(function (child) {
      navigator._store.remove(child.id);
    });

    navigator._store.remove('bugsinfile');

    var bugsInFileNode = {
      id     : 'bugsinfile',
      type   : 'bugsinfile',
      parent : 'root',
      loaded : false
    };

    var filter = new codeCheckerDBAccess.ReportFilter();
    filter.filepath = navigator._fileInfo.path;

    try {
      var bugCount = model.codecheckerservice.getRunResultCount(-1, [filter]);
    } catch (e) {
      bugsInFileNode.name = "Error reaching CodeChecker server.";
      navigator._store.put(bugsInFileNode);
      return;
    }

    bugsInFileNode.name
      = bugCount + ' bugs in ' + entities.encode(navigator._fileInfo.path);
    bugsInFileNode.nodeInfo = { bugCount : bugCount };

    navigator._store.put(bugsInFileNode);
    navigator.loadBugs(0, filter, bugsInFileNode);
  }

  /**
   * This function assembles the form and the input dialog which asks the reason
   * of suppression.
   * @param {Function} okCallback A callback function which is invoked when OK
   * button is pressed. The callback function is passed the comment text as
   * parameter.
   */
  function createSuppressDialog(okCallback) {
    var suppressForm = new Form();

    dom.place(
      dom.toDom("Why do you want to suppress this bug?"),
      suppressForm.domNode);

    var textarea = new Textarea({ style : 'margin: 5px 0px' });
    dom.place(textarea.domNode, suppressForm.domNode);

    var okButton = new Button({
      label   : 'OK',
      onClick : function () {
        okCallback(textarea.get('value'));
        suppressDialog.hide();
      }
    });
    dom.place(okButton.domNode, suppressForm.domNode);

    var cancelButton = new Button({
      label   : 'Cancel',
      onClick : function () {
        textarea.set('value', '');
        suppressDialog.hide();
      }
    });
    dom.place(cancelButton.domNode, suppressForm.domNode);

    var suppressDialog = new Dialog({
      title   : 'Suppress bug',
      content : suppressForm
    });
    
    return suppressDialog;
  }

  return declare(BorderContainer, {
    constructor : function () {
      var that = this;

      //--- Context Button ---//

      var suppressDialog = createSuppressDialog(function (comment) {
        model.codecheckerservice.suppressBug(
          [], that._currentReportId, comment);
  
        that._tree.collapseAll();
        
        that._store.query({ parent : 'overview' }).forEach(function (checker) {
          that._store.query({ parent : checker.id }).forEach(function (bug) {
            that._store.remove(bug.id);
          });
          that._store.remove(checker.id);
        });
        
        that._store.get('overview').loaded = false;
        
        loadErrorsForFile(that, true);
        
        style.set(that.contextButton.domNode, 'display', 'none');
      });

      this.contextButton = new Button({
        label   : 'Suppress bug',
        onClick : function () { suppressDialog.show(); }
      });
      
      style.set(this.contextButton.domNode, 'width', '100%');
      style.set(this.contextButton.domNode.firstChild, 'display', 'block');
      style.set(this.contextButton.domNode, 'margin-left', '0px');
      style.set(this.contextButton.domNode, 'display', 'none');

      //--- Tree field ---//
      
      this._data = [];

      /**
       * The store contains the bug nodes. Such a node has a type attribute
       * which can be one of the following ones:
       * - bug: Type of a node representing a bug (leaves in the tree).
       * - bugloadmore: "More..." for loading more bugs in a buglist
       */
      this._store = new Observable(new Memory({
        data        : that._data,
        getChildren : function (node) {
          return that._store.query({ parent : node.id });
        }
      }));

      this._store.put({
        id     : 'root',
        type   : 'root',
        loaded : true
      });
      
      this._store.put({
        id     : 'overview',
        type   : 'overview',
        name   : 'Bug overview',
        parent : 'root',
        loaded : false
      });
      
      var severityLevels = [
        {id : model.Severity.UNSPECIFIED, name : 'Unspecified'},
        {id : model.Severity.STYLE, name : 'Style'},
        {id : model.Severity.LOW, name : 'Low'},
        {id : model.Severity.MEDIUM, name : 'Medium'},
        {id : model.Severity.HIGH, name : 'High'},
        {id : model.Severity.CRITICAL, name : 'Critical'}
      ];
      
      model.codecheckerservice.getRunResultTypes(-1, [], function (result) {
        severityLevels.forEach(function(severityLevel){
          var count = 0;
          result.forEach(function (elem) {
            if(elem.severity === severityLevel.id)
              count += elem.count;
          });
          that._store.put({
            id     : 'severity_' + severityLevel.id,
            type   : 'severity',
            severityId  : severityLevel.id, 
            name   : severityLevel.name + ' (' + count + ')',
            parent : 'overview',
            loaded : false
          });
        });
      });
      
      this._store.put({
        id     : 'bugsinfile',
        type   : 'bugsinfile',
        name   : 'Please, select a file first',
        parent : 'root',
        loaded : false
      });

      var checkerDataModel = new ObjectStoreModel({
        store : that._store,
        query : { id : 'root' },
        mayHaveChildren : function (node) {
          return node.type !== 'bugstep' && node.type !== 'bugloadmore';
        }
      });

      this._tree = new NavTree({
        region      : 'center',
        model       : checkerDataModel,
        openOnClick : true,
        showRoot    : false,
        onOpen      : function (item, node) {
          if (item.id !== 'root')
            this.onClick(item, node);
        },
        onClick     : function (item, node, event) {
          itemClicked(that, item, event && mouse.isMiddle(event));
          
          if (item.type === 'bug')
            that._currentReportId = item.nodeInfo.bugInfo.reportId;
        }
      });
    },
    
    startup : function () {
      var contextmenu = new ContextMenu({
        targetNodeIds : [this.id],
        selector      : '.dijitTreeNode'
      });

      on(this, '.dijitTreeNode:contextmenu', function (event) {
        var item = dijit.byNode(
          query(event.target).closest('.dijitTreeNode')[0]).item;

        contextmenu.clear();

        if (item.type === 'bug' || item.type === 'checkerid') {
          var checkerId = item.type === 'bug' ? item.checkerId : item.id;

          contextmenu.addChild(new MenuItem({
            label   : 'Open Checker Documentation',
            onClick : function () {
              marked.setOptions({
                highlight : function (code) {
                  return hljs.highlightAuto(code).value;
                }
              });

              new Dialog({
                title: 'Checker documentation for bug: <b>' + checkerId + '</b>',
                content: marked(model.codecheckerservice.getCheckerDoc(checkerId))
              }).show();
            }
          }));
        }
      });
    },
    
    /**
     * Removes the children of the node identified by id from _store.
     * @param {Object} node An element of this._store.
     */
    removeChildren : function (node) {
      var children = this._store.getChildren(node);
      for (var i = 0; i < children.length; ++i) {
        this._store.remove(children[i].id);
      }
    },

    /**
     * Loads bugs dynamically either under the reference or in place of the load
     * more bugs list item.
     * @param {Number} offset Zero indexed number from which the bug list should
     * be loaded.
     * @param {ReportFilter} filter Thrift object.
     * @param {Object} parentNode Parent node under which new nodes are
     * inserted.
     */
    loadBugs : function (offset, filter, parentNode) {
      var that = this;

      var NUMBER_OF_BUGS_TO_LOAD = 15;

      var runResults = model.codecheckerservice.getRunResults(
        -1,
        NUMBER_OF_BUGS_TO_LOAD,
        offset,
        [], //TODO sort
        [filter]
      );
    
      runResults.forEach(function (currBug) {
        var path = currBug.checkedFile;
        var file = path.substr(path.lastIndexOf('/') + 1);
        
        that._store.put({
          checkerId : currBug.checkerId,
          loaded    : false,
          type      : 'bug',
          parent    : parentNode.id,
          nodeInfo  : {
            bugInfo : currBug,
            tooltip : null //TODO tooltip here
          },
          name      : file + ':' + currBug.lastBugPosition.startLine + ", "
                    + entities.encode(currBug.checkerMsg)
        });
      });

      // Check length as well because of the filtering
      if (offset + NUMBER_OF_BUGS_TO_LOAD < parentNode.nodeInfo.bugCount) {
        this._store.put({
          id       : parentNode.id + '_bugloadmore',
          name     : 'More ...',
          loaded   : false,
          type     : 'bugloadmore',
          parent   : parentNode.id,
          nodeInfo : {
            offset : offset + NUMBER_OF_BUGS_TO_LOAD,
            filter : filter
          }
        });
      }
    },

    loadSteps : function (points, parentNode) {
      var that = this;
      
      var fileInfos = {};
      
      points.forEach(function (point, index) {
        
        // Load file info from server if not loaded yet.
        if (!fileInfos[point.fileId]) {
          var fileInfo
            = model.codecheckerservice.getSourceFileData(point.fileId, false);
    
          fileInfos[point.fileId]
            = model.project.getFileInfoByPath(fileInfo.filePath);
        }
        
        //--- Set file info in the bugstep node ---//
        
        point.fileInfo = fileInfos[point.fileId];
        
        //--- Create label of the node ---//
        
        var name
          = 'Step ' + (index + 1)
          + ': '    + point.fileInfo.name
          + ' '     + point.startLine
          + ':'     + point.startCol;
        
        //--- Add node ---//
        
        that._store.put({
          loaded   : false,
          type     : 'bugstep',
          parent   : parentNode.id,
          name     : name,
          stepInfo : point
        });
      });
    },

    setState : function (state) {
      if (!state.fid)
        return;

      var fileId = new model.FileId();
      fileId.fid = state.fid;
      this._fileId = fileId;
      
      if (this._isShown)
        loadErrorsForFile(this);
    },

    postCreate : function () {
      this.addChild(this._tree);

      var that = this;
      
      // Subscribe to openFile to always know the currently opened file
      topic.subscribe('codecompass/openFile', function (message) {
        that._fileId = message.fileId;
        
        if (that._prevFid !== that._fileId.fid) {
          style.set(that.contextButton.domNode, 'display', 'none');
          that._prevFid = that._fileId.fid;
        }
        
        if (that._isShown)
          loadErrorsForFile(that);
      });
    },

    onShow : function () {
      this._isShown = true;
      loadErrorsForFile(this);
    },
    
    onHide : function () {
      this._isShown = false;
    }
    
  });
});

