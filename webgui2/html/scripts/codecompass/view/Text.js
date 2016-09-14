define([
  'dojo/_base/declare',
  'dojo/_base/array',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/query',
  'dojo/topic',
  'dijit/form/DropDownButton',
  'dijit/form/NumberSpinner',
  'dijit/layout/ContentPane',
  'dijit/Dialog',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/TitlePane',
  'dijit/Tooltip',
  'codecompass/view/InfoTree',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Editor',
  'codecompass/view/component/InfoBox',
  'codecompass/registration/keypressHandler',
  'codecompass/registration/menuHelper',
  'codecompass/registration/model',
  'codecompass/urlHandler',
  'codecompass/util'],
function (declare, array, dom, style, query, topic, DropDownButton,
  NumberSpinner, ContentPane, Dialog, MenuItem, PopupMenuItem, TitlePane,
  Tooltip, InfoTree, ContextMenu, Editor, InfoBox, keypressHandler, menuHelper,
  model, urlHandler, util) {
  
  /**
   * This function builds the context menu when clicked in CodeMirror. The menu
   * items are inserted every time when click in text view so that node specific
   * items can be shown. In this function we insert menu items from client and
   * server side.
   * @param {Text} textModule An instance of this Text module.
   * @param {Object} clickedPos This objects contains the clicked position. It
   * has 'line' and 'column' attributes.
   * @param {String} searchText Optional parameter. If given then the search
   * menu item contains this text. Otherwise search text will be the token under
   * the clicked position.
   */
  function buildContextMenu(textModule, clickedPos, searchText) {

    textModule._contextMenu.clear();

    var astNodeInfo
      = menuHelper.getAstNodeInfoByPosition(textModule._fileInfo, clickedPos);

    //--- Insert menu items coming from the server ---//
    
    if (astNodeInfo !== null)
      array.forEach(textModule._languageService.getMenuTypes(
      astNodeInfo.astNodeId), function (menuType) {
        textModule._contextMenu.addChild({
          menuType : menuType,
          onClick  : function () {
            menuHelper.contextMenuClicked(textModule, menuType, astNodeInfo.astNodeId);
          }
        });
      });

    //--- Building client side menu items ---//

    textModule._contextMenu.addChild(new MenuItem({
      label   : 'Get permalink to selection',
      onClick : function (event) {
        var text = dom.create('div', {
          innerHTML : 'You can copy the page URL from this checkbox which\
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
        
        (new Dialog({
          title   : 'Copy URL',
          content : content
        }).show());
        
        textInput.select();
      }
    }));

    if (astNodeInfo !== null) {
//      if (astNodeInfo.astNodeType === 'Function') {
//        var fromFunctionMenuItem = new model.MenuType();
//
//        fromFunctionMenuItem.name = [
//          'Diagrams',
//          'Call Path Diagram',
//          'From this function'
//        ];
//
//        if (textModule._fromFunctionNodeInfo)
//          fromFunctionMenuItem.name[2]
//            += ' (' + textModule._fromFunctionNodeInfo.astNodeValue + ')';
//
//        textModule._contextMenu.addChild({
//          menuType : fromFunctionMenuItem,
//          onClick  : function (event) {
//            textModule._fromFunctionNodeInfo = astNodeInfo;
//          }
//        });
//      }
    }
    
    //--- Search menu items ---//

    var token = searchText || textModule._editor.getWordAt(clickedPos).string;

    if (token.length > 50)
      token = token.substr(0, 50) + ' ...';

    var searchMenu = new ContextMenu();

    searchMenu.addChild(new MenuItem({
      label   : 'Text search',
      onClick : function (event) {
        menuHelper.publishSearch(token, model.SearchOptions.SearchInSource);
      }
    }));

    searchMenu.addChild(new MenuItem({
      label   : 'Definition search',
      onClick : function (event) {
        menuHelper.publishSearch(token, model.SearchOptions.SearchInDefs);
      }
    }));

    textModule._contextMenu.addChild(new PopupMenuItem({
      label : 'Search (' + token + ')',
      popup : searchMenu
    }));

    //--- Team menu ---//
    
    if (textModule._inRepositoryDetails) {
      var teamMenu = new ContextMenu();
      
      //--- Blame annotation ---//
      
      teamMenu.addChild(new MenuItem({
        label   : 'Blame',
        onClick : menuHelper.toggleAnnotateView(
          textModule._editor,
          textModule._inRepositoryDetails.repositoryId,
          textModule._inRepositoryDetails.commitId,
          textModule._inRepositoryDetails.repositoryPath,
          true)
      }));
      
      //--- History ---//
      
      //not yet enabled
      if (textModule._inRepositoryDetails.activeReference)
      {
        //history is currently working on the current branch
        
        teamMenu.addChild(new MenuItem({
          label   : 'History',
          onClick : menuHelper.enterHistoryView(
            textModule._editor,
            textModule._inRepositoryDetails.repositoryId,
            textModule._inRepositoryDetails.activeReference,
            textModule._inRepositoryDetails.repositoryPath)
        }));
      }

      textModule._contextMenu.addChild(new PopupMenuItem({
        label : 'Team',
        popup : teamMenu
      }));
    }
  }
  
  /**
   * This function sets the click events of the given editor.
   * @param {Text} text Text module object.
   */
  function setEditorEvents(text) {
    text._editor.on('Click', function (clickedPos) {
      Tooltip.hide(text._docTooltipNode);

      if (text._preventMarks) {
        text._preventMarks = false;
        return;
      }

      text.markUsages(clickedPos);
    });
      
    text._editor.on('RightClick', function (clickedPos) {
      Tooltip.hide(text._docTooltipNode);

      if (!positionInRange(clickedPos, text._editor.get('selection'))) {
        var token = text._editor.getWordAt(clickedPos);

        text._editor.set('selection', {
          from : { line : clickedPos.line, column : token.start },
          to   : { line : clickedPos.line, column : token.end   }
        });
      } else {
        var selectedText = text._editor.getSelectedText();
      }

      buildContextMenu(text, clickedPos, selectedText);
    });

    text._editor.on('CtrlClick', function (clickedPos) {
      Tooltip.hide(text._docTooltipNode);

      var jumpTo
        = menuHelper.getAstNodeInfoByPosition(text._fileInfo, clickedPos);

      if (!jumpTo) {
        var fileRange = new model.FileRange();
        fileRange.range = new model.Range();

        var token = text._editor.getWordAt(clickedPos);

        var startpos  = new model.Position();
        var endpos    = new model.Position();

        startpos.line   = clickedPos.line;
        startpos.column = token.start;
        endpos.line     = clickedPos.line;
        endpos.column   = token.end;

        fileRange.file = text._fileInfo.file;
        fileRange.range.startpos = startpos;
        fileRange.range.endpos   = endpos;

        jumpTo = { text : token.string, fileRange : fileRange };
      }

      menuHelper.jumpToDef(jumpTo, false, text._languageService);
    });

    text._editor.on('CtrlRightClick', function (clickedPos) {
      Tooltip.hide(text._docTooltipNode);

      menuHelper.displayInfobox(
        text,
        menuHelper.getAstNodeInfoByPosition(text._fileInfo, clickedPos));
    });

    text._editor.on('MiddleClick', function (clickedPos) {
      Tooltip.hide(text._docTooltipNode);

      menuHelper.jumpToDef(
        menuHelper.getAstNodeInfoByPosition(text._fileInfo, clickedPos),
        true,
        text._languageService);
    });

    text._editor.on('SelectionChange', function (range) {
      text._preventMarks = true;
      this.clearAllMarks();

      clearTimeout(this._selectionTimeoutId);
      this._selectionTimeoutId = setTimeout(function () {
        urlHandler.setStateValue('select', util.rangeToString(range));
      }, 3000);
    });
  }
  
  /**
   * This function returns true if the given position is inside the given range.
   * @param {type} position A position object with "line" and "column"
   * attribute.
   * @param {type} range A range attribute which contains "from" and "to"
   * attributes.
   */
  function positionInRange(position, range) {
    if (position.line > range.from.line && position.line < range.to.line)
      return true;
    
    return range.from.line === range.to.line
      ? position.line === range.from.line   &&
        position.column >= range.from.column && position.column <= range.to.column
      : position.line === range.from.line && position.column >= range.from.column ||
        position.line === range.to.line   && position.column <= range.to.column;
  }
  
  function createContextButtons(that) {
    
    //--- Diagrams ---//
    
    var diagButton = new DropDownButton({
      label    : 'File diagrams',
      dropDown : new ContextMenu()
    });
    
    style.set(diagButton.domNode, 'width', '100%');
    style.set(diagButton.domNode.firstChild, 'display', 'block');
    style.set(diagButton.domNode, 'margin-left', '0px');
    
    //--- File outline ---//
    
    var fileOutline = new TitlePane({
      title   : 'File Outline',
      class   : 'fileOutline',
      open    : false,
      content : new InfoTree({
        id    : 'fileoutline',
        style : { overflow : 'auto', maxHeight : '300px' }
      })
    });
    
    //--- Metrics button ---//
    
    if (model.metricsservice) {
      var metricsButton = new DropDownButton({
        label    : 'Metrics',
        dropDown : new ContextMenu()
      });

      style.set(metricsButton.domNode, 'width', '100%');
      style.set(metricsButton.domNode.firstChild, 'display', 'block');
      style.set(metricsButton.domNode, 'margin-left', '0px');
    }
    
    return [fileOutline, diagButton, metricsButton];
  }
  
  function loadFileDiagItems(text, fileId) {
    var diagButton = text.contextButton[1];

    if (!diagButton)
      return;

    if (fileId === diagButton.prevFileId)
      return;

    diagButton.prevFileId = fileId;

    if (!fileId)
      return;
      
    if (!text._fileInfo)
      return;
    
    var menu = text.contextButton[1].get('dropDown');
    menuHelper.buildFileDiagramContextmenu(menu, text._fileInfo);
  }
  
  function loadMetricsItems(text, fileId) {
    var metricsButton = text.contextButton[2];

    if (!metricsButton)
      return;
    
    if (fileId.fid === metricsButton.prevFileId)
      return;
    
    metricsButton.prevFileId = fileId;
    
    if (!fileId)
      return;
    
    if (!text._fileInfo)
      return;
    
    var menu = text.contextButton[2].get('dropDown');
    menuHelper.buildMetricsContextmenu(menu, text._fileInfo);
  }
  
  /**
   * This function populates file outline module if the file id to be set
   * differs from the previously loaded one.
   * @param {Text} text Text module instance.
   * @param {FileId} fileId FileId thrift object. If not set then file id is
   * read from the url.
   */
  function loadFileOutline(text, fileId) {
    var fileOutline = text.contextButton[0];

    if (fileId.fid === fileOutline.prevFileId)
      return;

    fileOutline.prevFileId = fileId.fid;

    if (!fileId.fid)
      return;

    try {
      var fileInfo = model.project.getFileInfo(fileId);
      fileOutline.content.loadInfoTree(fileInfo, fileInfo.type);
    } catch (ex) {
      console.warn('Invalid file id', fileId);
    }
  }
  
  return declare(ContentPane, {
    constructor : function () {
      var that = this;

      this._editor      = new Editor();
      this._contextMenu = new ContextMenu();
      this._marks       = [];
      
      this.contextButton = createContextButtons(that);

      this._infoBoxWidget = new InfoBox({
        style : 'display: none'
      }, dom.create('div', { class : 'infobox' }));

      var lineJumpDialog = new Dialog({
        title   : 'Jump to line',
        content : new NumberSpinner({
          onKeyDown : function (event) {
            if (event.keyCode === 13) {
              lineJumpDialog.hide();
              that.jumpToPos(lineJumpDialog.content.get('value'));
            }
          }
        })
      });

      topic.subscribe('codecompass/openFile', function (message) {
        if (message.newTab)
          return;

        that._editor.clearAllMarks();

        that.loadFile(message.fileId);

        if (message.selection)
          that._editor.set('selection', message.selection);

        that.jumpToPos(
          message.line || 0,
          message.selection ? message.selection.from.column : 0);
          
        // TODO: This should also run if the content is loaded at page load
        lineJumpDialog.content.set('constraints', {
          min : 1, max : that._editor.getLineCount()
        });
      });
      
      topic.subscribe('/dojo/hashchange', function (url) {
        that.loadContextButtonContents();
      });
      
      // ctrl-f
      keypressHandler(
        function (event) { return event.ctrlKey && event.keyCode === 70; },
        function ()      { that.getSearchInput().select(); });
        
      // ctrl-g
      keypressHandler(
        function (event) { return event.ctrlKey && event.keyCode === 71; },
        function ()      { lineJumpDialog.show(); });
    },

    postCreate : function () {
      this.set('content', this._editor.domNode);
      dom.place(this._infoBoxWidget.domNode, this.domNode);
    },
    
    startup : function () {
      this.inherited(arguments);
      
      this._contextMenu.bindDomNode(
        query('.CodeMirror-scroll', this._editor.domNode.lastChild)[0]);

      setEditorEvents(this);
      this.loadContextButtonContents();
    },

    /**
     * This function returns an object which represents the selection in the
     * text view.
     */
    _getSelectionAttr : function () {
      return this._editor.getSelection();
    },

    _setSelectionAttr : function (selection) {
      this._editor.set('selection', selection);
    },

    /**
     * The state of the module can be set with an object which is queried by
     * getState() function.
     * @param {Object} state State descriptor object.
     */
    setState : function (state) {
      if (!state.fid)
        return;

      var fileId = new model.FileId();
      fileId.fid = state.fid;

      var selection = state.select ? state.select.split('|') : [1, 1, 1, 1];
      selection
        = array.map(selection, function (item) { return parseInt(item); });

      // TODO: It in on purpose that we don't publish topic here for opening a
      // file. Maybe these should be organized in a function since we do the
      // same in openFile subscript handler.
      this.loadFile(fileId);
      this._editor.set('selection', {
        from : { line : selection[0], column : selection[1] },
        to   : { line : selection[2], column : selection[3] }
      });
      this.jumpToPos(selection[0] || 0, selection[1]);
    },

    /**
     * This function loads a file in the text view.
     * @param {Object} fileId FileId thrift object.
     */
    loadFile : function (fileId) {
      var that = this; 
        
      // The same file shouldn't be loaded twice after each other.
      if (this._fileInfo && fileId.fid === this._fileInfo.file.fid)
        return;

      // Turn off blame flag
      this._editor._isSetFormatter = false;

      try {
        var fileStat = model.project.getFileStat(fileId);
      } catch (ex) {
        console.warn('Invalid file id', fileId);
        return;
      }

      this._fileInfo = fileStat.info;
      this._languageService = model.getLanguageService(this._fileInfo.type);

      this._editor.set('content', fileStat.content);
      this._editor.setHeader(this._fileInfo);

      var mode;
      switch (this._fileInfo.type) {
        case model.FileType.CSource:      mode = 'text/x-csrc';     break;
        case model.FileType.CxxSource:    mode = 'text/x-c++src';   break;
        case model.FileType.JavaSource:   mode = 'text/x-java';     break;
        case model.FileType.JavaScript:   mode = 'text/javascript'; break;
        case model.FileType.ErlangSource: mode = 'text/x-erlang';   break;
        case model.FileType.BashScript:   mode = 'text/x-sh';       break;
        case model.FileType.PerlScript:   mode = 'text/x-perl';     break;
        case model.FileType.PythonScript: mode = 'text/x-python';   break;
        case model.FileType.RubyScript:   mode = 'text/x-ruby';     break;
        case model.FileType.SqlScript:    mode = 'text/x-sql';      break;
        default:                          mode = '';                break;
      }

      this._editor.set('mode', mode);
      
      this._languageService.getSyntaxHighlight(fileId, function (syntax) {
        array.forEach(syntax, function (s) {
          this._editor.markText(s.range.startpos, s.range.endpos, {
            className: s.className
          }, true);
        }, that);
      });
      
      require(['dojo/query', 'dojo/dom-geometry', 'dojo/domReady!'],
      function (query, geometry) {        
        var tooltipSelectors = ['.cm-variable', '.cm-Macro'];
        tooltipSelectors.forEach(function(selector){ 
          new Tooltip({
            connectId  : that._editor.domNode,
            position   : ['below', 'above'],
            selector   : selector,
            showDelay  : 1000,
            getContent : function (node) {
              that._docTooltipNode = node;

              var geoPos = geometry.position(node);

              // This happens if there is Tooltip should be shown after the node
              // is clicked, not just hovered. In this case CodeMirror select the
              // clicked <span> element as the same AST nodes are also selected.
              // Set selection in CodeMirror is done by replacing the node, which
              // means that by the time the Tooltip is shown, the node will have
              // changed and Tooltip's position can't be set properly.
              if (geoPos.x === 0 && geoPos.y === 0)
                return;

              var position = that._editor.lineColByPosition(geoPos);

              var filePos = new model.FilePosition();
              filePos.pos = new model.Position();
              filePos.file       = that._fileInfo.file;
              filePos.pos.line   = position.line;
              filePos.pos.column = position.column;

              var infobox = that._languageService.getInfoBoxByPosition(filePos);
              return infobox.documentation;
            }
          });
        });
      });
      
      this._editor.setLineNumberFormatter();

      // Check if file is in a version control repository
      if (model.versionservice) {
        var rbpp = model.versionservice.getRepositoryByProjectPath(
          this._fileInfo.path);
      
        this._inRepositoryDetails = rbpp.isInRepository ? rbpp : false;
      }
    },

    addBubbles : function (bubbles) {
      this._editor.addBubbles(bubbles);
    },
    
    clearBubbles : function () {
      this._editor.clearBubbles();
    },

    addLines : function (points) {
      this._editor.addLines(points);
    },
    
    clearLines : function () {
      this._editor.clearLines();
    },

    /**
     * This function returns the load file info object of the loaded file. If no
     * file was loaded then it returns null.
     */
    getFileInfo : function () {
      return this._fileInfo;
    },
    
    /**
     * This function returns the search input field if any.
     */
    getSearchInput : function () {
      return this._editor.getSearchInput();
    },

    /**
     * This function jumps to the given position.
     * @param {Number} line Jump to this line.
     * @param {Number} column Jump to this column. If not defined then its value
     * is 0.
     */
    jumpToPos : function (line, column) {
      this._editor.jumpTo(line, column || 0);
    },
    
    /**
     * This function sets if search bar should always be visible.
     * @param {Boolean} display If true the search bar is always displayed.
     */
    _setDisplaySearchAttr : function (display) {
      this._editor.set('displaySearch', display);
    },
    
    /**
     * This function returns true if search bar is always visible, false
     * otherwise.
     */
    _getDisplaySearchAttr : function () {
      return this._editor.get('displaySearch');
    },
    
    /**
     * This function marks the usages of an AST node at the given position.
     * @param {Object} position A position object which has 'line' and 'column'
     * attributes. The references of an AST node at this position will be
     * marked.
     */
    markUsages : function (position) {
      try {
        var astNodeInfos = this._languageService.getReferencesInFile(
          menuHelper.getAstNodeInfoByPosition(this._fileInfo, position).astNodeId,
          model.RefTypes.GetUsage,
          this._fileInfo.file);
      } catch (ex) {
        // If no AST node at this position
        this._editor.clearAllMarks();
        return;
      }

      this._editor.clearAllMarks();

      astNodeInfos.forEach(function (astNodeInfo) {
        var range = astNodeInfo.range.range;

        if (range.endpos.line !== range.startpos.line) {
          range.endpos.line
            = range.startpos.line;
    
          var line = this._editor.getLine(range.startpos.line);
          if(line)
            range.endpos.column = line.length + 1;
        }

        this._editor.markText(range.startpos, range.endpos, {
          className : 'cb-marked-select'
        });
      }, this);
    },
    
    loadContextButtonContents : function () {
      var fileId = new model.FileId();
      fileId.fid = urlHandler.getState().fid;
      
      loadFileDiagItems(this, fileId);
      loadFileOutline(this, fileId);
      loadMetricsItems(this, fileId);
    }
  });
});
