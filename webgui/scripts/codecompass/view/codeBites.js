require([
  'dojo/_base/declare',
  'dojo/_base/array',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dojo/on',
  'dijit/layout/ContentPane',
  'dojox/layout/ResizeHandle',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Text',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/urlHandler'],
function (declare, array, dom, style, topic, on, ContentPane, ResizeHandle,
  ContextMenu, Text, model, viewHandler, urlHandler) {

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
  var colors = ['#42a62c', '#2c34a5', '#e5801b', '#a0398f', '#c91010'];

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

  /**
   * This function creates a new box in CodeBites view.
   * @param {Object} astNodeInfo AstNodeInfo thrift object which is displayed in
   * the box.
   * @param {Object} panel ContentPane dojo panel on which the box is placed.
   * @param {Object} parent CodeBitesElement object, the parent of this element.
   */
  function createCodeBitesElement(astNodeInfo, panel, parent) {

    //--- Set content ---//

    var fileInfo = model.project.getFileInfo(astNodeInfo.range.file);

    languageService = model.getLanguageService(fileInfo.type);
    if (astNodeInfo.astNodeType !== 'Definition')
      astNodeInfo = languageService.getReferences(astNodeInfo.id, 0)[0];

    var srcText = languageService.getSourceText(astNodeInfo.id);

    var newElement = new CodeBitesElement({
      astNodeInfo     : astNodeInfo,
      firstLineNumber : astNodeInfo.range.range.startpos.line,
      closable        : true,
      resizable       : true,
      parent          : parent || null,
      panel           : panel,
      fileInfo        : fileInfo,
      selection       : panel.selection
    });

    newElement.set('content', srcText);
    newElement.set('header', fileInfo);

    //--- Place node ---//

    dom.place(newElement.domNode, panel.domNode);

    if (parent)
      newElement.connection = panel._jsPlumbInstance.connect({
        source     : parent.domNode,
        target     : newElement.domNode,
        paintStyle : { stroke : colors[colorIdx], strokeWidth : 2 },
        Anchors  : ['Perimeter', { shape : 'Ellipse' }],
        Endpoint : ['Dot', { radius : 1 }],
        overlays   : [['Label', {
          label    : astNodeInfo.astNodeValue,
          cssClass : 'cb-label cb-label-' + colorIdx
        }]]
      });

    if (window.gtag) {
      window.gtag ('event', 'code_bites', {
        'event_category' : urlHandler.getState('wsid'),
        'event_label' : urlHandler.getFileInfo().name
                      + ': '
                      + astNodeInfo.astNodeValue.toString()
      });
    }

    return newElement;
  }

  /**
   * CodeBites box element.
   */
  var CodeBitesElement = declare(Text, {
    /**
     * Constructor of a box element.
     * @param {Object} config CodeBitesElement class is inherited from Editor
     * class, so its constructor expects the same attributes. Furthermore
     * 'config' parameter has to contain these properties:
     * - astNodeId: (Thrift object) Node ID of the loaded AST node. If
     * astNodeInfo is also given then this ID is omitted.
     * - astNodeInfo: (Thrift object) Node info of the loaded AST node. If
     * astNodeId is also given then that's omitted.
     * - parent: (CodeBitesElement) Parent node of this element.
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
                  + this.astNodeInfo.id;

      if (this.parent)
        this.parent.children[this.htmlId] = this;

      elementMatrix[this.row][this.col] = this;
    },

    postCreate : function () {
      this.inherited(arguments);

      var that = this;

      style.set(this.domNode, 'border',   '1px solid grey');
      style.set(this.domNode, 'position', 'absolute'      );
      style.set(this.domNode, 'left',     this.left + 'px');
      style.set(this.domNode, 'top',      this.top  + 'px');

      style.set(this.domNode, 'width',  this.width  + 'px');
      style.set(this.domNode, 'height', this.height + 'px');

      //--- Resize handler ---//

      new ResizeHandle({
        targetContainer : that.domNode,
        animateMethod   : 'combine'
      }).placeAt(this);

      //--- Close button ---//

      var closeImg = dom.create('span', { class : 'icon-x' });

      dom.place(closeImg, this._header.header);
      on(closeImg, 'click', function () {
        that._onClose();
      });

      this.contextMenu = new ContextMenu();
      this.contextMenu.bindDomNode(this.domNode.lastChild);

      this._refresh();
    },

    resize : function() {
      this._refresh();
    },

    /**
     * Subscribe on topics
     * !Warning!: We need to override it to avoid openFile topic
     */
    _subscribeTopics : function () {
      var that = this;

      topic.subscribe('/dojo/resize/stop', function (inst) {
        if (inst.targetDomNode.id !== that.id)
          return;

        that._onResize({
          width  : that.domNode.style.width,
          height : that.domNode.style.height
        });

        that._refresh();
      });
    },

    /**
     * Handle mouse events
     */
    _eventHandler : function (event) {

      //--- Select the clicked word ---//

      var pos = this._codeMirror.coordsChar({
        top  : event.clientY,
        left : event.clientX
      });

      //--- Right click ---//

      if (event.button === 0) {
        this._clickOnAstNode({
          'line'   : pos.line + this._codeMirror.options.firstLineNumber,
          'column' : pos.ch + 1
        });
      }
    },

    /**
     * Close a CodeBites window
     */
    _onClose : function () {

      //--- Remove child elements recursively ---//

      for (var element in this.children)
        this.children[element]._onClose();

      //--- Replace elements in the same row ---//

      for (var i = elementMatrix[this.row].length - 1; i > this.col; --i) {
        var element = elementMatrix[this.row][i];

        --element.col;
        element.left = elementMatrix[this.row][i - 1].left;

        style.set(element.domNode, 'left', element.left + 'px');
      }

      //--- Remove from elementMatrix ---//

      elementMatrix[this.row].splice(this.col, 1);

      if (elementMatrix[this.row].length === 0)
        elementMatrix.splice(this.row, 1);

      //--- Remove connection, mark, and the node itself ---//

      if (this.parent) {
        delete this.parent.children[this.htmlId];
        this.parent.clearMark(this.parent._marks[this.htmlId]);
        delete this.parent._marks[this.htmlId];
      }

      if (this.connection)
        this.panel._jsPlumbInstance.detachAllConnections(this.domNode);

      dom.destroy(this.domNode);

      this.panel._jsPlumbInstance.repaintEverything();
      
      //--- When root element is closed ---//

      if (!this.parent) {
        var range = this.astNodeInfo.range.range;
        topic.publish('codecompass/openFile', {
          fileId    : this.astNodeInfo.range.file,
          line      : range.startpos.line,
          selection : [
            range.startpos.line,
            range.startpos.column,
            range.endpos.line,
            range.endpos.column]
        });
      }
    },

    /**
     * Click on an AST node
     */
    _clickOnAstNode : function (pos) {

      //--- Get nodes by position ---//

      var mPos = new Position(),
          fPos = new FilePosition();

      mPos.line   = pos.line;
      mPos.column = pos.column;
      fPos.pos    = mPos;
      fPos.file   = this.astNodeInfo.range.file;

      var astNodeInfo    = languageService.getAstNodeInfoByPosition(fPos);
      var service = model.getLanguageService(fileInfo.type);
      if(service){
        var refTypes = service.getReferenceTypes(astNodeInfo.id);
      } else {
        return;
      }
      var defAstNodeInfo = languageService.getReferences(
          astNodeInfo.id, refTypes["Definition"])[0];

      //--- Check if it's necessary to open a new box ---//

      if (astNodeInfo.id === defAstNodeInfo.id)
        return;

      if (hasRow(this.row + 1) &&
        array.some(elementMatrix[this.row + 1], function (element) {
          return element.astNodeInfo.id === defAstNodeInfo.id;
        }))
        return;

      //--- Color selection and open new box ---//

      var token = this._getWordAt([pos.line, pos.column]);
      var markId = this.markText(
        { line : pos.line, column : token.start },
        { line : pos.line, column : token.end   },
        { className : 'cb-marked-' + colorIdx   });

      var childElement
        = createCodeBitesElement(defAstNodeInfo, this.panel, this);

      this._marks[childElement.htmlId] = markId;

      colorIdx = (colorIdx + 1) % colors.length;
    },

    /**
     * Resize a CodeBites item
     */
    _onResize : function (size) {
      var that = this;

      array.forEach(elementMatrix, function (row) {
        array.forEach(row, function (element) {
          if (element.col === that.col)
            style.set(element.domNode, 'width', size.width);
          if (element.col > that.col)
            style.set(element.domNode, 'left',
                (parseInt(element.domNode.style.left) +
                parseInt(that.domNode.style.width) -
                sizeCols[that.col]) + 'px');

          if (element.row === that.row)
            style.set(element.domNode, 'height', size.height);
          if (element.row > that.row) {
            style.set(element.domNode, 'top',
              (parseInt(element.domNode.style.top) +
               parseInt(that.domNode.style.height) -
               sizeRows[that.row]) + 'px');
          }
        });
      });

      sizeRows[this.row] = parseInt(size.height);
      sizeCols[this.col] = parseInt(size.width);

      this.panel._jsPlumbInstance.repaintEverything();
    },

  });

  /**
   * CodeBites module
   */
  var CodeBites = declare(ContentPane, {

    /**
     * Constructor for initialization
     */
    constructor : function () {
      this._subscribeTopics();
    },

    /**
     * Init variables
     */
    onLoad : function (center) {
      style.set(this.domNode, 'position', 'relative');
      style.set(this.domNode, 'overflow', 'auto');
      
      this._jsPlumbInstance = jsPlumb.getInstance({
        Container : this.domNode,
      });

      this._jsPlumbInstance.importDefaults({
        Anchors  : ['Bottom', 'Top'],
        Endpoint : ['Dot', { radius : 1 }],
        ConnectionsDetechable : false,
        ConnectionOverlays    : [['Arrow', { location : 1 }]]
      });

    },

    /**
     * Reset settings
     */
    onUnload : function (center) {
      elementMatrix = [];
      sizeCols      = [];
      sizeRows      = [];
      
      this.destroyDescendants();
    },

    /**
     * Set CodeBites's state from url
     */
    setState : function (state) {
      if ( state.center !== this.id ||
          !state.node ||
          !state.fid)
        return;

      var selection
        = state.select
        ? state.select.split('|').map(function (x) { return parseInt(x); })
        : [1, 1, 1, 1];

      topic.publish('codecompass/codebites', {
        'node' : urlHandler.getAstNodeInfo()
      });
    },

    /**
     * Subscribe on topic events
     */
    _subscribeTopics : function () {
      var that = this;

      topic.subscribe('codecompass/codebites', function (message) {

        if (!message.selection)
          message.selection = [1, 1, 1, 1];

        that.set('selection', message.selection);

        topic.publish('codecompass/setCenterModule', that.id);
        createCodeBitesElement(message.node, that);

        urlHandler.setStateValue({
          fid    : message.node.range.file,
          center : that.id,
          select : message.selection.join('|'),
          node   : message.node.id
        });
      });
    }
  });

  //--- Create and register CodeBites ---//

  var codebites = new CodeBites({
    id : 'codebites'
  });

  viewHandler.registerModule(codebites, {
    type : viewHandler.moduleType.Center
  });
});
