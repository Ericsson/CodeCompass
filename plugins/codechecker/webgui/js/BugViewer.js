define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/on',
  'dojo/query',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dijit/Dialog',
  'dijit/tree/ObjectStoreModel',
  'dijit/layout/ContentPane',
  'dijit/layout/BorderContainer',
  'dijit/form/Button',
  'dijit/form/CheckBox',
  'dijit/form/Textarea',
  'dijit/Tooltip',
  'codecompass/view/component/Text',
  'codecompass/urlHandler',
  'codecompass/viewHandler',
  'codecompass/model'],
function (declare, dom, style, on, query, Memory, Observable, topic,
  Dialog, ObjectStoreModel, ContentPane, BorderContainer, Button, CheckBox,
  Textarea, Tooltip, Text, urlHandler, viewHandler, model) {

  function resetJsPlumb(editor) {
    if (editor._jsPlumbInstance)
      editor._jsPlumbInstance.reset();

    // The position of this DOM element is set to relative so jsPlumb lines work
    // properly in the text view.
    var jsPlumbParentElement
      = query('.CodeMirror-lines', editor._codeMirror.getWrapperElement())[0];
    style.set(jsPlumbParentElement, 'position', 'relative');

    editor._jsPlumbInstance = jsPlumb.getInstance({
      Container             : jsPlumbParentElement,
      Anchor                : ['Perimeter', { shape : 'Ellipse' }],
      Endpoint              : ['Dot', { radius : 1 }],
      PaintStyle            : { strokeWidth : 1, stroke:"#c13333" },
      Connector             : ["Bezier", {curviness : 20}],
      ConnectionsDetachable : false,
      ConnectionOverlays    : [['Arrow', { location : 1, length : 10, width : 8 }]]
    });
  }

  var BugViewer = declare(Text, {
    constructor : function () {
      this.inherited(arguments);

      this._lineWidgets = [];
      this._lineMarks = [];

      this.style = 'padding: 0;';
    },

    postCreate : function () {
      this.inherited(arguments);
    },

    resize : function () {
      this.inherited(arguments);
      this._refresh();
    },

    drawBugPath : function () {
      var that = this;

      this.clearBubbles();
      this.clearLines();

      function filterFunction(obj) {
        return obj.fileId === that.sourceFileData.fileId;
      }

      var reportDetails = model.codecheckerservice.getReportDetails(this.reportData.reportId);

      var points = reportDetails.executionPath.filter(filterFunction);
      var bubbles = reportDetails.pathEvents.filter(filterFunction);

      // This is needed because CodeChecker gives different positions.
      points.forEach(function (point) { --point.startCol; });
      points.forEach(function (bubble) { --bubble.startCol; });

      that.addBubbles(bubbles);
      that.addLines(points);
      that.addOtherFileBubbles(reportDetails.executionPath);
    },

    addBubbles : function (bubbles) {
      var that = this;

      bubbles.forEach(function (bubble) {
        var left = that._codeMirror.defaultCharWidth() * bubble.startCol + 'px';

        var element = dom.create('div', {
          style : 'margin-left: ' + left,
          class : 'cc-checker-message',
          innerHTML : bubble.msg
        });

        that._lineWidgets.push(that._codeMirror.addLineWidget(
          bubble.startLine - 1, element));
      });
    },

    addOtherFileBubbles : function (path) {
      var that = this;

      for (var i = 1; i < path.length; ++i) {
        if (path[i].fileId !== this.sourceFileData.fileId &&
            path[i].fileId !== path[i - 1].fileId) {
          var element = dom.create('div', {
            class : 'otherFileMsg',
            innerHTML : 'bugpath in:<br>' + path[i].filePath.split('/').pop(),
            onclick : (function (i) {
              return function () {
                that.set(
                  'sourceFileData',
                  CC_SERVICE.getSourceFileData(path[i].fileId, true));
                that.drawBugPath();
                that.jumpToPos(path[i].startLine, path[i].startCol);
              };
            })(i)
          });

          this._lineWidgets.push(this._codeMirror.addLineWidget(
            path[i - 1].startLine - 1, element));
        }
      }
    },

    clearBubbles : function () {
      this._lineWidgets.forEach(function (widget) { widget.clear(); });
      this._lineWidgets = [];
    },

    addLines : function (points) {
      var that = this;

      points.forEach(function (p, i) {
        that._lineMarks.push(that._codeMirror.doc.markText(
          { line : p.startLine - 1, ch : p.startCol + 1 },
          { line : p.endLine - 1,   ch : p.endCol       },
          { className : 'checkerstep' }));
      });

      var range = this._codeMirror.getViewport();
      this._lineDrawer(range.from, range.to);
    },

    clearLines : function () {
      this._lineMarks.forEach(function (mark) { mark.clear(); });
      this._lineMarks = [];
      resetJsPlumb(this);
    },

    highlightBugPathEvent : function (bugPathEvent) {
      if (this._currentLineMark)
        this._currentLineMark.clear();

      this._currentLineMark = this._codeMirror.doc.markText(
        { line : bugPathEvent.startLine - 1, ch : bugPathEvent.startCol - 1 },
        { line : bugPathEvent.endLine - 1,   ch : bugPathEvent.endCol       },
        { className : 'currentMark' });
    },

    _setContentAttr : function (content) {
      this.inherited(arguments);
      this._refresh();
    },

    _setFilepathAttr : function (filepath) {
      this.filepath = filepath;
    },

    _setSourceFileDataAttr : function (sourceFileData) {
      this.sourceFileData = sourceFileData;
      this.set('content', sourceFileData.fileContent);
      this.set('filepath', sourceFileData.filePath);
      topic.publish('codecompass/setCenterModule', this.id);

      this.drawBugPath();
      this.jumpToPos(this.reportData.lastBugPosition.startLine, 0);
    },

    _subscribeTopics : function () {
    },

    _lineDrawer : function (from, to) {
      if (this._lineMarks.length === 0)
        return;

      var that = this;

      /**
       * This function returns the <span> element which belongs to the given
       * textmarker.
       * @param {TextMarker} textMarker CodeMirror object.
       * @return {Object|Null} A DOM object which belongs to the given text
       * marker or null if not found.
       * @pre This function assumes that the "spans" and "markers" variables
       * contain the corresponding DOM elements and markers at the same
       * position.
       */
      function getSpanToMarker(textMarker) {
        for (var line in markers) {
          var idx = markers[line].indexOf(textMarker);
          if (idx !== -1)
            return spans[line][idx];
        }
        return null;
      }

      var cmLines = query(
        '.CodeMirror-code', this._codeMirror.getWrapperElement())[0].children;
      var spans = {};
      var markers = {};

      this._lineMarks.forEach(function (textMarker) {
        // If not in viewport
        try {
          var line = textMarker.lines[0].lineNo();
        } catch (ex) {
          return;
        }

        if (line < from || line >= to)
          return;

        spans[line] = [];
        query('.checkerstep', cmLines[line - from]).forEach(function (step) {
          var count
            = (step.getAttribute('class').match(/checkerstep/g) || []).length;
          for (var i = 0; i < count; ++i)
            spans[line].push(step);
        });

        if (markers[line])
          markers[line].push(textMarker);
        else
          markers[line] = [textMarker];
      });

      // Sort the markers by the position of their start point in the given
      // line, so that they are placed on the same index as the corresponding
      // <span> element in the array "spans".
      for (var line in markers)
        markers[line].sort(function (left, right) {
          return left.find().from.ch - right.find().from.ch;
        });

      resetJsPlumb(this);

      var prev;
      this._lineMarks.forEach(function (textMarker) {
        var current = getSpanToMarker(textMarker);

        if (!current)
          return;

        if (prev){
          that._jsPlumbInstance.connect({
            source     : prev,
            target     : current
          });
        }
        prev = current;
      });
    }
  });

  var bugViewer = new BugViewer({ id : 'bugviewer' });

  viewHandler.registerModule(bugViewer, {
    type : viewHandler.moduleType.Center
  });
});