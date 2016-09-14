define([
  'dojo/_base/declare',
  'dojo/_base/array',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dijit/layout/ContentPane',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Editor',
  'codecompass/registration/menuHelper',
  'codecompass/registration/model'],
function (declare, array, dom, style, topic, ContentPane, MenuItem,
  PopupMenuItem, ContextMenu, Editor, menuHelper, model) {
  
  /**
   * In CodeBites view every code box has the same languageService object. By
   * this solution it's supposed that every box contains files from the same
   * type.
   */
  var languageService = null;
  
  /**
   * This array contains arrays of boxes of CodeBites view. Every array
   * represents a row of boxes.
   */
  var elementMatrix = [];
  
  /**
   * This array contains the height of each row.
   */
  var sizeRows = [];
  
  /**
   * This array contains the width of each column.
   */
  var sizeCols = [];
  
  /**
   * When a token is clicked in a box then the token will be selected by one of
   * these colors as the edge too.
   */
  var colors = ["#42a62c", "#2c34a5", "#e5801b", "#a0398f", "#c91010"];
  
  /**
   * Selection colors are going round. This variable contains the index of the
   * current color.
   */
  var colorIdx = 0;
  
  /**
   * Default properties of boxes.
   */
  var defaults = {
    width  : 480,
    height : 300,
    left   : 50,
    top    : 100
  };
  
  var fromFunctionNodeInfo = null;
  
  function createRow(row) {
    if (!elementMatrix[row]) {
      elementMatrix[row] = [];
      sizeRows[row] = defaults.height;
    }
  }

  function createCol(col) {
    if (!sizeCols[col])
      sizeCols[col] = defaults.width;
  }
  
  function hasRow(row) {
    return sizeRows[row] !== undefined;
  }
  
  function hasCol(col) {
    return sizeCols[col] !== undefined;
  }
  
  // TODO: should be refactored, because almost same as in text module.
  function buildContextMenu(cbElement, clickedPos, searchText) {
    
    //--- Building client side menu items ---//
    
    var astNodeInfo
      = menuHelper.getAstNodeInfoByPosition(cbElement.fileInfo, clickedPos);
      
    cbElement.contextMenu.clear();
    
    if (astNodeInfo !== null) {
      cbElement.contextMenu.addChild(new MenuItem({
        label    : 'Jump to definition',
        accelKey : 'Ctrl - Click',
        onClick  : function () {
          // TODO: 'text' shouldn't be here
          menuHelper.jumpToDef(astNodeInfo, 'text', false, languageService);
        }
      }));
      
      cbElement.contextMenu.addChild(new MenuItem({
        label    : 'Info Tree',
        accelKey : 'Ctrl - RightClick',
        onClick  : function () {
          topic.publish('codecompass/infotree', {
            elementInfo : astNodeInfo,
            fileType    : cbElement.fileInfo.type
          });
        }
      }));
      
      if (astNodeInfo.astNodeType === 'Function') {
        var fromFunctionMenuItem = new model.MenuType();
        
        fromFunctionMenuItem.name = [
          'Diagrams',
          'Call Path Diagram',
          'From this function'
        ];
        
        if (fromFunctionNodeInfo)
          fromFunctionMenuItem.name[2]
            += ' (' + fromFunctionNodeInfo.astNodeValue + ')';
    
        cbElement.contextMenu.addChild({
          menuType : fromFunctionMenuItem,
          onClick  : function () {
            fromFunctionNodeInfo = astNodeInfo;
          }
        });
      }
    }
    
    //--- Search menu items ---//
    
    var token = searchText || cbElement.getWordAt(clickedPos).string;
    
    if (token.length > 50)
      token = token.substr(0, 50) + ' ...';
    
    var searchMenu = new ContextMenu();
    
    searchMenu.addChild(new MenuItem({
      label   : 'Text search',
      onClick : function () {
        menuHelper.publishSearch(token, model.SearchOptions.SearchInSource);
      }
    }));
    
    searchMenu.addChild(new MenuItem({
      label   : 'Definition search',
      onClick : function () {
        menuHelper.publishSearch(token, model.SearchOptions.SearchInDefs);
      }
    }));
    
    cbElement.contextMenu.addChild(new PopupMenuItem({
      label : 'Search (' + token + ')',
      popup : searchMenu
    }));
    
    //--- Insert menu items coming from the server ---//
    
    if (astNodeInfo !== null)
      array.forEach(languageService.getMenuTypes(astNodeInfo.astNodeId),
      function (menuType) {
        cbElement.contextMenu.addChild({
          menuType : menuType,
          onClick  : function () {
            menuHelper.contextMenuClicked(cbElement, astNodeInfo, menuType);
          }
        });
      });
  }
  
  /**
   * This function creates a new box in CodeBites view.
   * @param {Object} astNodeInfo AstNodeInfo thrift object which is displayed in
   * the box.
   * @param {Object} panel ContentPane dojo panel on which the box is placed.
   * @param {Object} parent CBElement object, the parent of this element.
   */
  function createCBElement(astNodeInfo, panel, parent) {
    var newElement = new CBElement({
      astNodeInfo     : astNodeInfo,
      firstLineNumber : astNodeInfo.range.range.startpos.line,
      closable        : true,
      resizable       : true,
      parent          : parent || null,
      panel           : panel,
      fileInfo        : model.project.getFileInfo(astNodeInfo.range.file),
      
      onResize : function (size) {
        array.forEach(elementMatrix, function (row) {
          array.forEach(row, function (element) {
            if (element.col === newElement.col)
              element.set('size', { width : size.width });
            if (element.col > newElement.col)
              element.set('position', {
                left : (parseInt(element.domNode.style.left) +
                        parseInt(newElement.domNode.style.width) -
                        sizeCols[newElement.col]) + 'px'
              });
            
            if (element.row === newElement.row)
              element.set('size', { height : size.height });
            if (element.row > newElement.row) {
              element.set('position', {
                top : (parseInt(element.domNode.style.top) +
                       parseInt(newElement.domNode.style.height) -
                       sizeRows[newElement.row]) + 'px'
              });
            }
          });
        });
        
        sizeRows[newElement.row] = parseInt(size.height);
        sizeCols[newElement.col] = parseInt(size.width);
        
        panel.jsPlumbInstance.repaintEverything();
      },
      
      onClick : function (pos) {

        //--- Get nodes by position ---//

        var mPos = new model.Position(),
            fPos = new model.FilePosition();

        mPos.line   = pos.line;
        mPos.column = pos.column;
        fPos.pos    = mPos;
        fPos.file   = newElement.astNodeInfo.range.file;

        var clickedAstNodeInfo = languageService.getAstNodeInfoByPosition(fPos),
            defAstNodeInfo = languageService.getReferences(
              clickedAstNodeInfo.astNodeId, model.RefTypes.GetDef)[0];

        //--- Check if it's necessary to open a new box ---//

        if (clickedAstNodeInfo.astNodeId.astNodeId ===
            defAstNodeInfo.astNodeId.astNodeId)
          return;

        if (hasRow(newElement.row + 1) &&
            array.some(elementMatrix[newElement.row + 1], function (element) {
              return element.astNodeInfo.astNodeId.astNodeId ===
                     defAstNodeInfo.astNodeId.astNodeId;
            }))
          return;

        //--- Color selection and open new box ---//

        var token = newElement.getWordAt(pos);
        var markId = this.markText(
          { line : pos.line, column : token.start },
          { line : pos.line, column : token.end   },
          { className : 'cb-marked-' + colorIdx   });

        var childElement
          = createCBElement(defAstNodeInfo, newElement.panel, newElement);
        
        newElement.marks[childElement.htmlId] = markId;

        colorIdx = (colorIdx + 1) % colors.length;
      },
      
      onRightClick : function (pos) {
        buildContextMenu(newElement, pos);
      },
      
      onClose : function () {
        
        //--- Remove child elements recursively ---//
        
        for (var element in newElement.children)
          newElement.children[element].onClose();
        
        //--- Replace elements in the same row ---//
        
        for (var i = elementMatrix[newElement.row].length - 1;
             i > newElement.col;
             --i) {
          var element = elementMatrix[newElement.row][i];
          
          --element.col;
          element.left = elementMatrix[newElement.row][i - 1].left;
          
          style.set(element.domNode, 'left', element.left + 'px');
        }
        
        //--- Remove from elementMatrix ---//
        
        elementMatrix[newElement.row].splice(newElement.col, 1);
        
        if (elementMatrix[newElement.row].length === 0)
          elementMatrix.splice(newElement.row, 1);
        
        //--- Remove connection, mark, and the node itself ---//
        
        if (parent) {
          delete parent.children[newElement.htmlId];
          parent.clearMark(parent.marks[newElement.htmlId]);
          delete parent.marks[newElement.htmlId];
        }
        
        if (newElement.connection)
          panel.jsPlumbInstance.detachAllConnections(newElement.domNode);
        
        dom.destroy(newElement.domNode);
        
        panel.jsPlumbInstance.repaintEverything();
        
        //--- When root element is closed ---//
        
        if (!newElement.parent) {
          topic.publish('codecompass/openFile', {
            newTab    : false,
            fileId    : newElement.fileInfo.file,
            moduleId  : 'text',
            line      : newElement.astNodeInfo.range.range.startpos.line,
            selection : {
              from : newElement.astNodeInfo.range.range.startpos,
              to   : newElement.astNodeInfo.range.range.endpos
            }
          });
        }
      }
    });
    
    //--- Set content ---//

    languageService = model.getLanguageService(newElement.fileInfo.type);

    newElement.set(
      'content', languageService.getSourceCode(astNodeInfo.astNodeId));
    newElement.setHeader(newElement.fileInfo);
    
    //--- Place node ---//

    dom.place(newElement.domNode, panel.domNode);
    
    if (parent)
      newElement.connection = panel.jsPlumbInstance.connect({
        source     : parent.domNode,
        target     : newElement.domNode,
        paintStyle : { strokeStyle : colors[colorIdx], lineWidth : 2 },
        overlays   : [['Label', {
          label    : astNodeInfo.astNodeValue,
          cssClass : 'cb-label cb-label-' + colorIdx
        }]]
      });
    
    return newElement;
  }
  
  var CBElement = declare(Editor, {
    /**
     * Constructor of a box element.
     * @param {Object} config CBElement class is inherited from Editor class, so
     * its constructor expects the same attribute. Furthermore 'config'
     * parameter has to contain these properties:
     * - astNodeId: (Thrift object) Node id of the loaded AST node. If
     * astNodeInfo is also given then this id is omitted.
     * - astNodeInfo: (Thrift object) Node info of the loaded AST node. If
     * astNodeId is also given then that's omitted.
     * - parent: (CBElement) Parent node of this element.
     */
    constructor : function (config) {

      //--- Set properties ---//

      this.astNodeInfo
        = config.astNodeInfo ||
          languageService.getAstNodeInfo(config.astNodeId);

      this.parent = config.parent;
      this.children = {};

      this.row = this.parent ? this.parent.row + 1 : 0; createRow(this.row);
      this.col = elementMatrix[this.row].length;        createCol(this.col);

      this.height = sizeRows[this.row];
      this.width  = sizeCols[this.col];

      this.top = 0;
      for (var i = 0; i < this.row; ++i)
        this.top += sizeRows[i] + defaults.top;

      this.left = 0;
      for (var i = 0; i < this.col; ++i)
        this.left += sizeCols[i] + defaults.left;

      this.panel = config.panel;
      this.htmlId = 'cp-row-' + this.row + '-astnodeid-'
                  + this.astNodeInfo.astNodeId.astNodeId;
      // TODO: We should use a normal id attribute.

      if (this.parent)
        this.parent.children[this.htmlId] = this;
      
      this.marks = {};

      elementMatrix[this.row][this.col] = this;
    },
    
    postCreate : function () {
      style.set(this._domElements.editor, 'border',   '1px solid gray');
      style.set(this._domElements.editor, 'position', 'absolute'      );
      style.set(this._domElements.editor, 'left',     this.left + 'px');
      style.set(this._domElements.editor, 'top',      this.top  + 'px');
      
      this.set('size', {
        width  : this.width  + 'px',
        height : this.height + 'px'
      });
      
      this.contextMenu = new ContextMenu();
      this.contextMenu.bindDomNode(this.domNode.lastChild);
    }
  });
  
  return declare(ContentPane, {
    constructor : function () {
      var that = this;

      topic.subscribe('codecompass/codebites', function (message) {
        createCBElement(message.astNodeInfo, that);
      });
    },
    
    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if (state.cent !== this.id || !state.diagNode || !state.fid)
        return;
      
      var fileId = new model.FileId();
      fileId.fid = state.fid;
      
      languageService
        = model.getLanguageService(model.project.getFileInfo(fileId).type);

      var astNodeId = new model.AstNodeId();
      astNodeId.astNodeId = state.diagNode;

      createCBElement(languageService.getAstNodeInfo(astNodeId), this);
    },

    onLoad : function (center) {
      style.set(center.domNode, 'overflow', 'auto');
      style.set(this.domNode, 'position', 'relative');
      style.set(this.domNode, 'overflow', 'auto');
      
      this.jsPlumbInstance = jsPlumb.getInstance({
        Container : this.domNode
      });

      this.jsPlumbInstance.importDefaults({
        Anchors  : ['Bottom', 'Top'],
        Endpoint : ['Dot', { radius : 1 }],
        ConnectionsDetechable : false,
        ConnectionOverlays    : [['Arrow', { location : 1 }]]
      });
    },

    onUnload : function (center) {
      elementMatrix = [];
      sizeCols      = [];
      sizeRows      = [];

      this.destroyDescendants();

      style.set(center.domNode, 'overflow', 'hidden');
    }
  });
});