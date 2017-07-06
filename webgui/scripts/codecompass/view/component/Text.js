define([
  'dojo/_base/declare',
  'dojo/dom-class',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/query',
  'dojo/topic',
  'dijit/layout/ContentPane',
  'dijit/Dialog',
  'dijit/MenuItem',
  'codecompass/astHelper',
  'codecompass/util',
  'codecompass/model',
  'codecompass/urlHandler',
  'codecompass/viewHandler',
  'codecompass/view/component/ContextMenu'],
function (declare, domClass, dom, style, query, topic, ContentPane, Dialog,
  MenuItem, astHelper, util, model, urlHandler, viewHandler, ContextMenu) {

  var BuildDialog = declare(Dialog, {

    postCreate : function () {
      this.inherited(arguments);

      this._table = dom.create('table', { class : 'buildlogtable' });

      this._addHeader();
      this.set('content', this._table);
    },

    /**
     * This function appends an array of build logs in the table of the dialog.
     * @param {[BuildLog]} buildLogs An array of BuildLog thrift objects.
     */
    addBuildLogs : function (buildLogs) {
      var that = this;

      buildLogs.forEach(function (buildLog) {
        var row = dom.create('tr', { onclick : function () {
          that.hide();

          if (!that.textmodule)
            console.error("Editor object must be given in constructor");

          that.textmodule.set('selection', {
            from : buildLog.range.startpos,
            to   : buildLog.range.endpos
          });

          that.textmodule.jumpToPos(
            buildLog.range.startpos.line,
            buildLog.range.startpos.end);
        }});

        var type = dom.create('span');
        var col;

        switch (buildLog.messageType) {
          case MessageType.Unknown:
            domClass.add(type, 'unknown');
            dom.place(dom.toDom('Unknown'), type);
            break;
          case MessageType.Error:
            domClass.add(type, 'error');
            dom.place(dom.toDom('Error'), type);
            break;
          case MessageType.FatalError:
            domClass.add(type, 'fatalerror');
            dom.place(dom.toDom('Fatal Error'), type);
            break;
          case MessageType.Warning:
            domClass.add(type, 'warning');
            dom.place(dom.toDom('Warning'), type);
            break;
          case MessageType.Note:
            domClass.add(type, 'note');
            dom.place(dom.toDom('Note'), type);
            break;
          case MessageType.CodingRule:
            domClass.add(type, 'codingrule');
            dom.place(dom.toDom('Coding Rule'), type);
            break;
        }

        col = dom.create('td');
        dom.place(type, col);
        dom.place(col, row);

        col = dom.create('td', { innerHTML : buildLog.message });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.startpos.line });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.startpos.column });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.endpos.line });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.endpos.column });
        dom.place(col, row);

        dom.place(row, this._table);
      }, this);
    },

    /**
     * This function clears the build log table in the dialog. Only a header row
     * is left there.
     */
    clearBuildLogs : function () {
      dom.empty(this._table);
      _addHeader();
    },

    /**
     * This function appends a header row in the table of the given dialog
     * instance.
     */
    _addHeader : function () {
      var row = dom.create('tr');
      var col;

      col = dom.create('th', { innerHTML : 'Type' });
      dom.place(col, row);
      col = dom.create('th', { innerHTML : 'Message' });
      dom.place(col, row);
      col = dom.create('th', { innerHTML : 'From', colspan : 2 });
      dom.place(col, row);
      col = dom.create('th', { innerHTML : 'To', colspan : 2 });
      dom.place(col, row);

      dom.place(row, this._table);
    }
  });

  /**
   * This function returns true if the given position is inside the given range.
   * @param {Array} position An array with two elements: line and column
   * respectively.
   * @param {Array} range An array with four elements: start line, start column,
   * end line, end column respectively.
   */
  function positionInRange(position, range) {
    var pos = { line : position[0], column : position[1] };
    var ran = {
      from : { line : range[0], column : range[1] },
      to   : { line : range[2], column : range[3] }
    };

    if (pos.line > ran.from.line && pos.line < ran.to.line)
      return true;

    return ran.from.line === ran.to.line
      ? pos.line === ran.from.line &&
        pos.column >= ran.from.column && pos.column <= ran.to.column
      : pos.line === ran.from.line && pos.column >= ran.from.column ||
        pos.line === ran.to.line   && pos.column <= ran.to.column;
  }

  /**
   * This function adds menu items to the given context menu. The menu items
   * come from the language service which are applicable for the AST node at
   * the given position. Moreover some client-side menu items are also added.
   * @param {Array} position An array with two elements: line and column
   * respectively.
   * @param {FileInfo} fileInfo A Thrift object which contains the information
   * of the file in which the click happens.
   * @param {ContextMenu} contextMenu A Menu which will be filled with menu
   * items.
   */
  function buildContextMenu(position, fileInfo, contextMenu) {

    var astNodeInfo = astHelper.getAstNodeInfoByPosition(position, fileInfo);

    //--- Build menu ---//

    contextMenu.clear();

    if (astNodeInfo)
      viewHandler.getModules({
        type : viewHandler.moduleType.TextContextMenu,
        fileType : fileInfo.type
      }).forEach(function (menuItem) {
        var item = menuItem.render(astNodeInfo, fileInfo);
        if (item)
          contextMenu.addChild(item);
      });

    contextMenu.addChild(new MenuItem({
      label : 'Get permalink to selection',
      onClick : function (event) {
        var text = dom.create('div', {
          innerHTML : 'You can copy the page URL from this text box which\
            encodes the selection too.',
          style : 'margin-bottom: 5px; width: 300px; text-align: center'
        });

        var textInput = dom.create('input', {
          value : urlHandler.getURL(),
          style : 'width: 100%'
        });

        var content = dom.create('div');
        dom.place(text, content);
        dom.place(textInput, content);

        (new Dialog({ title : 'Copy URL', content : content })).show();

        textInput.select();
      }
    }));
  }

  return declare(ContentPane, {
    constructor : function () {
      this.class = 'editor';
      this.firstLineNumber = 1;
      this._buildDialog = new BuildDialog({ textmodule : this });

      this._marks = {};
      this._markIdCounter = 0;

      this._subscribeTopics();
    },

    postCreate : function () {
      var that = this;

      //--- Header ---//

      this._header = {
        header      : dom.create('div',  { class : 'header'      }),
        parsestatus : dom.create('span', { class : 'parsestatus' }),
        filename    : dom.create('span', { class : 'filename'    }),
        path        : dom.create('span', { class : 'path'        }),
        colons      : dom.toDom('<span class="colons">::</span>')
      };

      dom.place(this._header.header, this.domNode);
      dom.place(this._header.parsestatus, this._header.header);
      dom.place(this._header.filename, this._header.header);
      dom.place(this._header.colons, this._header.header);
      dom.place(this._header.path, this._header.header);

      //--- CodeMirror ---//

      this._codeMirror = new CodeMirror(this.domNode, {
        matchBrackets : true,
        lineNumbers : true,
        firstLineNumber : that.firstLineNumber,
        readOnly : true,
        mode : 'text/x-c++src',
        foldGutter : true,
        gutters : ['CodeMirror-linenumbers', 'bugInfo'],
        extraKeys : {}
      });

      this._codeMirror.on("mousedown", function (cm, event) {
        that._eventHandler(event);
      });

      this._codeMirror.on('contextmenu', function (cm, event) {
        event.preventDefault();
      });

      this._contextMenu = new ContextMenu();
    },

    startup : function () {
      this.inherited(arguments);

      // TODO: if add new resizehandler it will not be the last node of domNode
      var codeMirror = query('.CodeMirror', this.domNode)[0];
      this._contextMenu.bindDomNode(
        query('.CodeMirror-scroll', codeMirror)[0]);
    },

    resize : function () {
      this.inherited(arguments);
      this._refresh();
    },

    setState : function (state) {
      if (!state.fid)
        return;

      var selection
        = state.select
        ? state.select.split('|').map(function (x) { return parseInt(x); })
        : [1, 1, 1, 1];

      this.loadFile(state.fid);
      this.set('selection', selection);
      this.jumpToPos(selection[0], selection[1]);
    },

    /**
     * This function loads a file in the Text module.
     * @param {FileInfo | String} file This can be a FileInfo thrift object or a
     * file ID.
     */
    loadFile : function (file) {
      var urlFileInfo = urlHandler.getFileInfo();

      if (!(file instanceof FileInfo)) {
        // The same file shouldn't be loaded twice after each other.
        if (this._fileInfo && file === this._fileInfo.id)
          return;

        this._fileInfo = urlFileInfo && urlFileInfo.id === file
          ? urlFileInfo
          : model.project.getFileInfo(file);
      } else {
        this._fileInfo = file;
      }

      var fileContent = urlFileInfo && urlFileInfo.id === this._fileInfo.id
        ? urlHandler.getFileContent()
        : model.project.getFileContent(this._fileInfo.id);

      this.set('content', fileContent);
      this.set('header', this._fileInfo);
    },

    /**
     * This function jumps to the given location. The location will be about 100
     * pixels inside from the edge of the editor.
     */
    jumpToPos : function (line, column) {
      var selPosPixel
        = this._codeMirror.charCoords({ line : line, ch : column }, 'local');

      var editorSize = {
        width  : style.get(this.domNode, 'width'),
        height : style.get(this.domNode, 'height')
      };

      this._codeMirror.scrollIntoView({
        top    : selPosPixel.top - 100,
        bottom : selPosPixel.top + editorSize.height - 150,
        left   : selPosPixel.left < editorSize.width - 100
               ? 0
               : selPosPixel.left - 50,
        right  : selPosPixel.left < editorSize.width - 100
               ? 10
               : selPosPixel.left + editorSize.width - 100
      });
    },

    /**
     * This function calls the markText() function of CodeMirror, thus its
     * options are the same except that 'from' and 'to' objects has to have
     * 'line' and 'column' attributes.
     * @param {Object} from Mark text from this position.
     * @param {Object} to Mark text till this position.
     * @param {Object} options Pass these options to CodeMirror's markText
     * function.
     * @return {Number} This function returns an id which can be passed to
     * clearMark() function to remove this selection.
     */
    markText : function (from, to, options) {
      var fln = this._codeMirror.options.firstLineNumber;

      this._marks[this._markIdCounter] = this._codeMirror.markText(
        { line : from.line - fln, ch : from.column - 1 },
        { line : to.line   - fln, ch : to.column   - 1 },
        options);

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
     * This function clears all the text marks.
     */
    clearAllMarks : function () {
      for (var markId in this._marks)
        this.clearMark(markId);
      this._marks = {};
    },

    _eventHandler : function (event) {
      //--- Select the clicked word ---//

      var pos = this._codeMirror.coordsChar({
        top  : event.clientY,
        left : event.clientX
      });

      pos = [
        pos.line + this._codeMirror.options.firstLineNumber,
        pos.ch + 1];

      if (!positionInRange(pos, this.get('selection'))) {
        var token = this._getWordAt(pos);
        this.set('selection', [pos[0], token.start, pos[0], token.end]);
      }

      //--- Highlighting the same occurrence of the selected entity ---//

      this._markUsages(pos, this._fileInfo);

      //--- Right click ---//

      if (event.button === 2)
        buildContextMenu(pos, this._fileInfo, this._contextMenu);

      //--- Ctrl-click ---//

      if (event.button === 0 && event.ctrlKey) {
        var astNodeInfo = getAstNodeInfoByPosition(pos, this._fileInfo);
        var service = model.getLanguageService(this._fileInfo.type);
        astHelper.jumpToDef(astNodeInfo.id, service);
      }
    },

    /**
     * This function marks the usages of an AST node.
     */
    _markUsages : function (position, fileInfo) {
      var that = this;

      var astNodeInfo = astHelper.getAstNodeInfoByPosition(position, fileInfo);

      var refTypes = model.cppservice.getReferenceTypes(astNodeInfo.id);
      var usages = model.cppservice.getReferences(
        astNodeInfo.id,
        refTypes['Usage']);

      this.clearAllMarks();

      var fl = that._codeMirror.options.firstLineNumber;
      usages.forEach(function (astNodeInfo) {
        var range = astNodeInfo.range.range;

        if (range.endpos.line !== range.startpos.line) {
          range.endpos.line = range.startpos.line;

          var line = that._codeMirror.getLine(range.startpos.line - fl);

          if (line)
            range.endpos.column = line.length + 1;
        }

        that.markText(range.startpos, range.endpos, {
          className : 'cb-marked-select'
        });
      }, this);
    },

    /**
     * This function returns a token object which represents the clicked word.
     * @param {Array} position This parameter describes the clicked position.
     * The array has to have two elements: the line and column number
     * respectively.
     * @return {Object} The returning object has three attributes:
     *   - string: the word as a string under the clicked position.
     *   - start: the start position of the clicked word.
     *   - end: the end position of the clicked word.
     */
    _getWordAt : function (position) {
      var pattern = /[a-z0-9_]/i;

      var line = this._codeMirror.getLine(
        position[0] - this._codeMirror.options.firstLineNumber);

      var start = position[1] - 1;
      while (start > -1 && line[start] && line[start].match(pattern))
        --start;
      ++start;

      var end = position[1] - 1;
      while (end < line.length && line[end] && line[end].match(pattern))
        ++end;

      return {
        string : line.substring(start, end),
        start  : start + 1,
        end    : end + 1
      };
    },

    _refresh : function () {
      var that = this;
      setTimeout(function () {
        var fullHeight = parseInt(style.getComputedStyle(that.domNode).height);
        var headerHeight = util.getFullHeight(that._header.header);

        that._codeMirror.setSize('100%', (fullHeight - headerHeight) + 'px');
        that._codeMirror.refresh();
      }, 0);
    },

    _subscribeTopics : function () {
      var that = this;

      // Opens a file and sets the center module to this. Message attributes:
      // - fileId: The ID of the file to load.
      // - selection (optional): An array of four elements which determines the
      //   range of highlight in the text: start line, start column, end line
      //   and end column respectively.
      // - line (optional): The line number to jump to when the file is loaded.
      topic.subscribe('codecompass/openFile', function (message) {
        if (!message.selection)
          message.selection = [1, 1, 1, 1];

        urlHandler.setStateValue({
          fid    : message.fileId,
          select : message.selection.join('|'),
          center : that.id
        });

        that.loadFile(message.fileId);

        that.set('selection', message.selection);

        that.jumpToPos(
          message.line || 0,
          message.selection ? message.selection[1] : 0);

        topic.publish('codecompass/setCenterModule', that.id);
      });
    },

    /**
     * This function highlights the given range in the editor.
     * @param {Array} range An array containing four elements: start line, start
     * column, end line, end column respectively. This array determined the
     * range to select. The end column is exclusive, i.e. not part of the range.
     */
    _setSelectionAttr : function (range) {
      var that = this;
      this.selection = range;

      setTimeout(function () {
        var fl = that._codeMirror.options.firstLineNumber;
        that._codeMirror.doc.setSelection(
          { line : range[2] - fl, ch : range[3] - 1 },
          { line : range[0] - fl, ch : range[1] - 1 })
      }, 0);
    },

    _setContentAttr : function (content) {
      this._codeMirror.doc.setValue(content);
    },

    _setHeaderPathAttr : function (path) {
      path = path.split('/').slice(1);
      var pathElements = dom.create('span');
      var string = '';

      path.forEach(function (file) {
        string += '/' + file;
        var s = string; // This is needed because of closure affairs.

        dom.place(dom.toDom('/'), pathElements);
        dom.place(dom.create('span', {
          innerHTML : file,
          class     : 'pathelement',
          onclick   : function () { topic.publish('codecompass/openPath', s); }
        }), pathElements);
      });

      dom.place(pathElements, this._header.path, 'only');
    },

    _setHeaderAttr : function (fileInfo) {
      var statusClasses = ['fullyparsed', 'partiallyparsed', 'notparsed'];
      domClass.remove(this._header.parsestatus, statusClasses);

      switch (fileInfo.parseStatus) {
        case FileParseStatus.FullyParsed:
          domClass.add(this._header.parsestatus, statusClasses[0]);
          dom.place(dom.toDom('(Fully parsed)'), this._header.parsestatus, 'only');
          break;

        case FileParseStatus.PartiallyParsed:
          domClass.add(this._header.parsestatus, statusClasses[1]);
          dom.place(dom.toDom('(Partially parsed)'), this._header.parsestatus, 'only');
          break;

        default:
          domClass.add(this._header.parsestatus, statusClasses[2]);
          dom.place(dom.toDom('(Not parsed)'), this._header.parsestatus, 'only');
      }

      dom.place(dom.toDom(fileInfo.name), this._header.filename, 'only');
      this.set('headerPath', fileInfo.path);
    }
  });
});
