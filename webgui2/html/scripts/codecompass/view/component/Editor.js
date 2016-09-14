define([
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/_base/window',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/mouse',
  'dojo/on',
  'dojo/topic',
  'dojo/dnd/Moveable',
  'dojo/query',
  'dijit/_WidgetBase',
  'dojox/layout/ResizeHandle',
  'codecompass/view/component/BuildLogDialog',
  'codecompass/registration/model',
  'codecompass/util'],
function (array, declare, win, dom, style, mouse, on, topic, Moveable, query,
  _WidgetBase, ResizeHandle, BuildLogDialog, model, util) {
  
  /**
   * This function is a callback function for handling clicks on CodeMirror
   * window. Its purpose is to call onClick, onCtrlClick, onRightClick, etc.
   * functions.
   * @param {Editor} editor Editor module object.
   * @param {Object} event Mouse event.
   */
  function clickHandler(editor, event) {    
    var pos = editor.codeMirror.coordsChar({
      top  : event.clientY,
      left : event.clientX
    });
    
    pos = {
      line   : pos.line + editor.codeMirror.options.firstLineNumber,
      column : pos.ch + 1
    };

    editor[
      'on' +
      (event.ctrlKey        ? 'Ctrl'  : '') +
      (mouse.isRight(event) ? 'Right' : mouse.isMiddle(event) ? 'Middle' : '') +
      'Click'].call(editor, pos);

    // This is needed because Chrome has an extra 'focus' event which closes
    // dojo contextmenu in Chrome browser.
    if (mouse.isRight(event)) {
      event.preventDefault();
    }
  }

  /**
   * This function refreshes the editor module view. This function can be used
   * to fill the height of the page with the editor.
   * @param {Editor} editor Editor module object to refresh.
   */
  function refresh(editor) {
    setTimeout(function () {

      // It's needed to compute the parent's height by hand, because using 100%
      // explicitly is too slow for CodeMirror.

      //--- Get parent height ---//
      var heightEditor
        = style.getComputedStyle(editor._domElements.editor).height;
      var heightHeader
        = style.getComputedStyle(editor._domElements.header).height;

      // 11 = padding and border for header element
      var height = parseInt(heightEditor) - parseInt(heightHeader) - 11 -
                   (editor.displaySearch ? 20 : 0) + 'px';
           
      if (editor.displaySearch) {
        style.set(editor.codeMirror.getWrapperElement(), 'paddingTop', '20px');
        query('.CodeMirror-vscrollbar').style('top', '22px');
      }

      //--- Set height ---//

      editor.codeMirror.setSize('100%', height);
      editor.codeMirror.refresh();
    }, 0);
  }

  /**
   * This function places a clickable path in the header of an Editor module.
   * When click on a part of the path, tha codecompass/openPath message will be
   * sent.
   * @param {Editor} editor Editor module of which the header will contain the
   * clickable path.
   * @param {String} path An arbitrary absolute path.
   */
  function buildPath(editor, path) {
    path = path.split('/').slice(1);

    var result = dom.create('span');
    var string = '';

    array.forEach(path, function (file) {
      string += '/' + file;
      var s = string; // This is needed because of closure affairs.

      dom.place(dom.toDom('/'), result);
      dom.place(dom.create('span', {
        innerHTML : file,
        class     : 'pathelement',
        onclick   : function () { topic.publish('codecompass/openPath', s); }
      }), result);
    });

    dom.place(result, editor._domElements.path, 'only');
  }
  
  /**
   * This function resets the JsPlumb instance.
   * @param {Editor} editor Editor instance.
   * TODO: Why does the JsPlumb have to be reinstantiated?
   */
  function resetJsPlumb(editor) {
    if (editor.jsPlumbInstance)
      editor.jsPlumbInstance.reset();
    
    // The position of thie DOM element is set to relative so jsPlumb lines
    // work properly in the text view.
    var jsPlumbParentElement
      = query('.CodeMirror-lines', editor.codeMirror.getWrapperElement())[0];
    style.set(jsPlumbParentElement, 'position', 'relative');

    editor.jsPlumbInstance = jsPlumb.getInstance({
      Container : jsPlumbParentElement,
      Anchor     : ['Perimeter', { shape : 'Ellipse' }],
      Endpoint   : ['Dot', { radius : 1 }],
      PaintStyle : { strokeStyle : 'blue', lineWidth : 1 },
      ConnectionsDetechable : false,
      ConnectionOverlays    : [['Arrow',
        { location : 1, length : 10, width : 8 }]]
    });
  }
  
  function lineDrawer(editor, from, to) {
    if (editor._lineMarks.length === 0)
      return;
    
    /**
     * This function returns the &lt;span&gt; element which belongs to the given
     * textMarker.
     * @param {TextMarker} textMarker CodeMirror object.
     * @return {Object|Null} A DOM object which belongs to the given text marker
     * or null if not found.
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
      '.CodeMirror-code', editor.codeMirror.getWrapperElement())[0].children;
    var spans = {};
    var markers = {};

    editor._lineMarks.forEach(function (textMarker) {
      // If not in viewport
      try {
        var line = textMarker.lines[0].lineNo();
      } catch (ex) {
        return;
      }
      if (line < from || line >= to)
        return;

      spans[line] = query('.checkerstep', cmLines[line - from]);

      if (markers[line])
        markers[line].push(textMarker);
      else
        markers[line] = [textMarker];
    });

    // Sort the markers by the position of their start point in the given line,
    // so that they are placed on the same index as the corresponding <span>
    // element in the array "spans".
    for (var line in markers)
      markers[line].sort(function (left, right) {
        return left.find().from.ch - right.find().from.ch;
      });

    resetJsPlumb(editor);

    var prev;
    editor._lineMarks.forEach(function (textMarker) {
      var current = getSpanToMarker(textMarker);

      if (!current)
        return;

      if (prev)
        editor.jsPlumbInstance.connect({
          source : prev,
          target : current
        });

      prev = current;
    });
  }
  
  return declare(_WidgetBase, {
    constructor : function () {
      var that = this;

      this._lineWidgets = [];
      this._lineMarks = [];

      // TODO: in code bites it's unnecessary to refresh all editors
      // on window resize.
      on(win.global, 'resize', function () { refresh(that); });
      
      this._buildLogDialog = new BuildLogDialog({
        title  : 'Build Logs',
        editor : that
      });
    },

    buildRendering : function () {
      var that = this;

      //--- DOM elements ---//

      if (this.srcNodeRef)
        require(['dojo/dom-class'], function (domClass) {
          if (!domClass.contains(that.srcNodeRef, 'editor'))
            domClass.add(that.srcNodeRef, 'editor');
        });

      this._domElements = {
        editor : this.srcNodeRef || dom.create('div', { class : 'editor' }),
        header : dom.create('div', { class : 'header' }),

        filename    : dom.create('span', { class : 'filename'    }),
        parseStatus : dom.create('span', { class : 'parsestatus' }),
        path        : dom.create('span', { class : 'path'        }),
        closeImg    : dom.create('img',  { class : 'close', src : '/images/close.png' }),
        colons      : dom.toDom('<span class="colons">::</span>')
      };

      dom.place(this._domElements.parseStatus, this._domElements.header);
      dom.place(this._domElements.filename, this._domElements.header);
      dom.place(this._domElements.colons, this._domElements.header);
      dom.place(this._domElements.path, this._domElements.header);

      if (this.resizable) {
        new ResizeHandle({
          targetContainer : this._domElements.editor,
          animateMethod   : 'combine'
        }).placeAt(this._domElements.editor);

        topic.subscribe('/dojo/resize/stop', function (inst) {
          if (inst.targetDomNode.id !== that.id)
            return;

          that.onResize({
            width  : that.domNode.style.width,
            height : that.domNode.style.height
          });

          refresh(that);
        });
      }

      //--- Resize and move ---//

      if (this.draggable)
        new Moveable(
          this._domElements.editor, { handle : this._domElements.header });

      this.domNode = this._domElements.editor;

      //--- Add header ---//

      dom.place(this._domElements.header, this._domElements.editor);

      if (this.closable) {
        dom.place(this._domElements.closeImg, this._domElements.header);
        on(this._domElements.closeImg, 'click', function () {
          that.onClose(that);
        });
      }

      //--- Create CodeMirror ---//

      this.codeMirror = new CodeMirror(this._domElements.editor, {
        matchBrackets   : this.matchBrackets,
        firstLineNumber : this.firstLineNumber,
        lineNumbers     : this.lineNumbers,
        readOnly        : this.readOnly,
        mode            : this.mode,
        foldGutter      : true,
        gutters         : ["CodeMirror-linenumbers", "CodeMirror-foldgutter"]
      });

      // Save default line number formatter so we can reset
      this._cmDefaultLineNumberFormatter
        = this.codeMirror.getOption("lineNumberFormatter");
      
      refresh(this);

      function cmClickHandler(event) { clickHandler(that, event); }

      this.codeMirror.on('contextmenu', function (cm, event) {
        event.preventDefault();
      });
      
      this.codeMirror.on('beforeSelectionChange', function (cm, range) {
        if (range.anchor.ch   !== range.head.ch ||
            range.anchor.line !== range.head.line)
          that.onSelectionChange({
            from : { line : range.anchor.line + 1, column : range.anchor.ch + 1 },
            to   : { line : range.head.line   + 1, column : range.head.ch   + 1 }
          });
      });
      
      this.codeMirror.on('viewportChange', function (cm, from, to) {
        lineDrawer(that, from, to);
      });
      
      var clickableArea
        = query('.CodeMirror-scroll', this.codeMirror.getWrapperElement())[0];
      
      on(clickableArea, 'contextmenu', cmClickHandler);
      on(clickableArea, 'mouseup', cmClickHandler);

      this._marks = {}; /*< This stores text selections which are returned by
                            markText CodeMirror function. This object maps an
                            automatically generated number to the TextMarker
                            object. From outside only the number should be used.
                        */
      this._marksNonStatic = {}; /*< This stores selections, which can change */
      this._markIdCounter = 0;
      
      if (this.displaySearch)
        this.codeMirror.execCommand("find");
      
      resetJsPlumb(this);
    },
    
    addBubbles : function (bubbles) {
      var that = this;
      
      var fln = this.codeMirror.options.firstLineNumber;
      
      bubbles.forEach(function (bubble) {
        var left = that.codeMirror.defaultCharWidth() * bubble.startCol + 'px';
        
        var element = dom.create('div', {
          style     : 'margin-left: ' + left,
          class     : 'checkMsg',
          innerHTML : bubble.msg
        });

        that._lineWidgets.push(that.codeMirror.addLineWidget(
          bubble.startLine - fln, element));
      });
    },
    
    clearBubbles : function () {
      this._lineWidgets.forEach(function (widget) { widget.clear(); });
      this._lineWidgets = [];
    },
    
    addLines : function (points) {
      var fln = this.codeMirror.options.firstLineNumber;
      
      for (var i = 0; i < points.length; ++i) {
        var point = this.codeMirror.doc.markText(
          { line : points[i].startLine - fln, ch : points[i].startCol },
          { line : points[i].endLine   - fln, ch : points[i].endCol   },
          { className : 'checkerstep' });
        this._lineMarks.push(point);
      }
      
      var range = this.codeMirror.getViewport();
      lineDrawer(this, range.from, range.to);
    },

    clearLines : function () {
      resetJsPlumb(this);
      this._lineMarks.forEach(function (mark) { mark.clear(); });
      this._lineMarks = [];
    },

    /**
     * This function sets the text of the CodeMirror window. 
     * @param {String} content Content of the CodeMirror window.
     * TODO: This function should also set the 'mode' attribute for syntax
     * highlight in CodeMirror.
     */
    _setContentAttr : function (content) {
      this.clearLines();
      this.clearBubbles();
      
      this.codeMirror.doc.setValue(content);
      
      refresh(this);
    },
    
    /**
     * This function returns the text of the CodeMirror window.
     */
    _getContentAttr : function () {
      return this.codeMirror.doc.getValue();
    },

    /**
     * This function returns the given line as a string.
     * @param {Number} line Line number
     */
    getLine : function (line) {
      return this.codeMirror.doc.getLine(line - 1);
    },

    /**
     * This function sets the content of the header.
     * @param {FileInfo} fileInfo The header content is taken from this
     * parameter.
     */
    setHeader : function (fileInfo) {
      var that = this;
      this.fileInfo = fileInfo;
      
      dom.place(
        dom.toDom(util.getFilename(fileInfo.path)),
        this._domElements.filename, 'only');
        
      buildPath(this, fileInfo.path);

      switch (fileInfo.parseStatus) {
        case model.FileParseStatus.None:
          dom.place(dom.toDom('(Not parsed)'), this._domElements.parseStatus, 'only');
          style.set(this._domElements.parseStatus, 'backgroundColor', 'red');
          break;
          
        case model.FileParseStatus.OnlyInSearchIndex:
          dom.place(dom.toDom('(Only in search index)'), this._domElements.parseStatus, 'only');
          style.set(this._domElements.parseStatus, 'backgroundColor', 'yellow');
          break;

        case model.FileParseStatus.FullyParsed:
          dom.place(dom.toDom('(Fully parsed)'), this._domElements.parseStatus, 'only');
          style.set(this._domElements.parseStatus, 'backgroundColor', 'lightgreen');
          break;

        case model.FileParseStatus.PartiallyParsed:
          dom.place(dom.toDom('(Partially parsed)'), this._domElements.parseStatus, 'only');
          style.set(this._domElements.parseStatus, 'backgroundColor', 'orange');
          break;

        case model.FileParseStatus.VCView:
          dom.place(dom.toDom('(Version Control View)'), this._domElements.parseStatus, 'only');
          style.set(this._domElements.parseStatus, 'backgroundColor', 'lightblue');
          break;
          
      }
      
      on(this._domElements.parseStatus, 'click', function () {
        that._buildLogDialog.clearBuildLogs();
        that._buildLogDialog.addBuildLogs(model.project.getBuildLog(fileInfo.file));
        that._buildLogDialog.show();
      });
    },

    /**
     * This function jumps to the given position.
     * @param {Number} line Line number.
     * @param {Number} column Column number.
     */
    jumpTo : function (line, column) {
      var that = this;
      setTimeout(function () {
        var selPosPixel
          = that.codeMirror.charCoords({ line : line, ch : column }, 'local');
        var editorSize = {
          width  : style.get(that._domElements.editor, 'width'),
          height : style.get(that._domElements.editor, 'height')
        };
        
        that.codeMirror.scrollIntoView({
          top    : selPosPixel.top - 100,
          bottom : selPosPixel.top + editorSize.height - 150,
          left   : selPosPixel.left < editorSize.width - 100
                 ? 0
                 : selPosPixel . left - 50,
          right  : selPosPixel.left < editorSize.width - 100
                 ? 10
                 : selPosPixel.left + editorSize.width - 100
        });
      }, 0);
    },

    /**
     * This function selects the given range.
     * @param {Object} range Range of the selection which contains a 'from' and
     * 'to' attribute. These attributes are also ojects with 'line' and 'column'
     * attributes.
     */
    _setSelectionAttr : function (range) {
      var that = this;
      setTimeout(function () {
        that.codeMirror.doc.setSelection(
          { line : range.from.line - 1, ch : range.from.column - 1 },
          { line : range.to.line   - 1, ch : range.to.column   - 1 });
      }, 0);
    },

    /**
     * This function returns the selected range if any. If there is no selected
     * range then it returns the cursor position.
     */
    _getSelectionAttr : function () {
      var from = this.codeMirror.doc.getCursor('start');
      var to   = this.codeMirror.doc.getCursor('end');

      return {
        from : { line : from.line + 1, column : from.ch + 1 },
        to   : { line : to.line   + 1, column : to.ch   + 1 }
      };
    },

    /**
     * This function returns a token info at the given position. In the
     * returning object there are properties like 'start' and 'end' which
     * determine the border positions of the token, 'string' which is the
     * token's text, 'type' like keyword, comment, etc. and 'state'.
     * @param {Object} pos This is a coordinate object which has to have 'line'
     * and 'column' properties.
     */
    getTokenAt : function (pos) {
      var token = this.codeMirror.getTokenAt({
        line : pos.line - this.codeMirror.options.firstLineNumber,
        ch   : pos.column
      });

      ++token.start;
      ++token.end;

      return token;
    },

    /**
     * This function returns the word under the clicked position. A string is
     * considered to be a word if its characters match the /[a-z0-9_]/i pattern.
     * @param {Object} pos This is a coordinate object which has to have 'line'
     * and 'column' properties.
     */
    getWordAt : function (pos) {
      var pattern = /[a-z0-9_]/i;
      
      var line = this.codeMirror.getLine(
        pos.line - this.codeMirror.options.firstLineNumber);

      var start = pos.column - 1;
      while (start > -1 && line[start] && line[start].match(pattern))
        --start;
      var end = pos.column - 1;
      while (end < line.length && line[end] && line[end].match(pattern))
        ++end;
      
      return {
        string : line.substring(start + 1, end),
        start  : start + 2,
        end    : end + 1
      };
    },

    /**
     * This function returns the selected text.
     */
    getSelectedText : function () {
      return this.codeMirror.getRange(
        this.codeMirror.doc.getCursor('start'),
        this.codeMirror.doc.getCursor('end'));
    },

    /**
     * This function returns the search input field's DOM node if any.
     */
    getSearchInput : function () {
      var searchInput = null;
      require(['dojo/dom'], function (dom) {
        searchInput = dom.byId('searchinput');
      });
      return searchInput;
    },

    /**
     * This function calls the markText() function of CodeMirror, thus its
     * options are the same except that 'from' and 'to' objects has to have
     * 'line' and 'column' attributes.
     * @param {Object} from Mark text from this position.
     * @param {Object} to Mark text till this position.
     * @param {Object} options Pass these options to CodeMirror's markText
     * function.
     * @param {Boolean} isStatic If true then this mark won't be deleted by the
     * clearAllMarks() function.
     * @return {Number} This function returns an id which can be passed to
     * clearMark() function to remove this selection.
     */
    markText : function (from, to, options, isStatic) {
      var fln = this.codeMirror.options.firstLineNumber;

      this._marks[this._markIdCounter] = this.codeMirror.markText(
        { line : from.line - fln, ch : from.column - 1 },
        { line : to.line   - fln, ch : to.column   - 1 },
        options);

      if (!isStatic)
        this._marksNonStatic[this._markIdCounter] = true;

      return this._markIdCounter++;
    },

    /**
     * This function clears a text mark. 
     * @param {Number} markId Id of the selection to remove.
     */
    clearMark : function (markId) {
      if (markId in this._marks) {
        this._marks[markId].clear();
        delete this._marks[markId];
      }
    },
    
     /**
     * This function clear all the text mark
     */
    clearAllMarks : function () {
      for (var markId in this._marksNonStatic)
        this.clearMark(markId);
    },

    /**
     * This function is the same as CodeMirror's getScrollInfo() function. The
     * returning object has the following properties: left, top, width, height,
     * clientWidth, clientHeight
     */
    getScrollInfo : function () {
      return this.codeMirror.getScrollInfo();
    },

    /**
     * This function resizes the editor element. It's recommended to use this
     * function for resizing instead of resizing the domNode directly, because
     * this way the editor's content is also refreshed.
     * @param {Object} size New size of the editor. This object can have a
     * 'width' and 'height' property.
     */
    _setSizeAttr : function (size) {
      if (size.width)
        style.set(this._domElements.editor, 'width',  size.width);
      if (size.height)
        style.set(this._domElements.editor, 'height', size.height);
      
      refresh(this);
    },

    /**
     * This function returns the size of the editor. The returning object has a
     * 'width' and 'height' attribute.
     */
    _getSizeAttr : function () {
      return {
        width  : this._domElements.editor.style.width,
        height : this._domElements.editor.style.height
      };
    },

    /**
     * This function sets the position of the editor.
     * @param {Object} position Position object which has a 'top' and 'left'
     * attribute.
     */
    _setPositionAttr : function (position) {
      if (position.top)
        style.set(this._domElements.editor, 'top',  position.top);
      if (position.left)
        style.set(this._domElements.editor, 'left', position.left);
    },

    /**
     * This function returns the position of the editor. The returning object
     * has a 'top' and a 'left' attribute.
     */
    _getPositionAttr : function () {
      return {
        top  : this._domElements.editor.style.top,
        left : this._domElements.editor.style.left
      };
    },

    /**
     * By this function one can set the highlighting mode of the editor.
     * @param {String} mode Source code highlighting mode. See possible values
     * here: http://codemirror.net/mode/
     */
    _setModeAttr : function (mode) {
      this.codeMirror.setOption('mode', mode);
      this.mode = mode;
    },

    /**
     * This function sets whether the search bar should be displayed or not.
     * @param {Boolean} display True if display should always be seen.
     */
    _setDisplaySearchAttr : function (display) {
      this.displaySearch = display;
      this.codeMirror.execCommand("find");
      refresh(this);
    },

    /**
     * This function can be used to refresh the editor.
     */
    resize : function () { refresh(this); },
    
    /**
     * By this function one can format the line numbers.
     * @param {Function} formatter This is a callback function which gets a
     * line number as parameter and has to return the text of a decorated
     * line number as String or as a DOM node. If the parameter is not set
     * then simple line numbers will be used.
     * TODO: actually a one element array which contains a DOM node.
     */
    setLineNumberFormatter : function (formatter) {
      this.codeMirror.setOption(
        "lineNumberFormatter",
        formatter || this._cmDefaultLineNumberFormatter);
    },

    /**
     * This function returns the line and column based on a given position in
     * pixels. The returning object has 'line' and 'column' properties.
     * @param {Object} pos The position object has to have an 'x' and 'y'
     * property.
     */
    lineColByPosition : function (pos) {
      var pos = this.codeMirror.coordsChar({
        left : pos.x,
        top  : pos.y
      });

      return {
        line   : pos.line + this.codeMirror.options.firstLineNumber,
        column : pos.ch + 1
      };
    },
    
    /**
     * This function returns the number of lines in the loaded content.
     */
    getLineCount : function () {
      return this.codeMirror.doc.lineCount();
    },

    // TODO: create proper getter and setter methods so that changing them have
    // effect.
    firstLineNumber : 1,
    lineNumbers     : true,
    readOnly        : true,
    matchBrackets   : true,
    mode            : 'text/x-c++src',
    resizable       : false,
    draggable       : false,
    closable        : false,
    displaySearch   : false,

    onClose           : function () {},
    onClick           : function () {},
    onCtrlClick       : function () {},
    onRightClick      : function () {},
    onCtrlRightClick  : function () {},
    onMiddleClick     : function () {},
    onCtrlMiddleClick : function () {},
    onResize          : function () {},
    onSelectionChange : function () {}
  });
});
