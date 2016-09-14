define([
  'dojo/_base/declare',
  'dojo/dom-style',
  'dojo/topic',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/layout/ContentPane',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Editor',
  'codecompass/registration/menuHelper',
  'codecompass/registration/model'],
function (declare, style, topic, MenuItem, PopupMenuItem, ContentPane,
  ContextMenu, Editor, menuHelper, model) {

  function isNewClickPosition(textModule, clickedPos) {

    //--- Check if it's the previously clicked position ---//

    // This branch is for performance reasons. See the comments of prevClickedPos variable.
    if (textModule._prevClickedPos.line   === clickedPos.line &&
        textModule._prevClickedPos.column === clickedPos.column)
      return false;

    textModule._prevClickedPos = clickedPos;
    return true;
  }

  /**
   * This function returns true if the given position is inside the given range.
   * @param {type} position A position object with "line" and "column" attribute.
   * @param {type} range A range attribute which contains "from" and "to" attributes.
   */
  function positionInRange(position, range) {
    if (position.line > range.from.line && position.line < range.to.line)
      return true;
    
    return range.from.line === range.to.line
         ? position.line   === range.from.line   &&
           position.column  >= range.from.column && position.column <= range.to.column
         : position.line === range.from.line && position.column >= range.from.column ||
           position.line === range.to.line   && position.column <= range.to.column;
  }
  
  function buildContextMenu(textModule, clickedPos, selectedText) {

    textModule._contextMenu.clear();

    //--- Search menu items ---//

    var token = textModule._editor.getWordAt(clickedPos).string;

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
    
    //--- Team menu items ---//
    
    var teamMenu = new ContextMenu();
    
    teamMenu.addChild(new MenuItem({
      label   : 'Blame',
      onClick : menuHelper.toggleAnnotateView(
        textModule._editor,
        textModule.versionrepoid,
        textModule.versioncommitid,
        textModule.versionpath,
        false)
    }));
    
    /*
     * this is currently incorrect as we need a branch commit
     * TODO fix this somehow
     * 
    teamMenu.addChild(new MenuItem({
      label   : 'History',
      onClick : menuHelper.enterHistoryView(
        textModule._editor,
        textModule._inRepositoryDetails.repositoryId,
        textModule._inRepositoryDetails.commitId,
        textModule._inRepositoryDetails.repositoryPath)
    }));*/

    textModule._contextMenu.addChild(new PopupMenuItem({
      label : 'Team',
      popup : teamMenu
    }));
    
    
  }

  
  
  return declare(ContentPane, {
    constructor : function () {
      var that = this;
      
      //--- Header ---//
      this._topcontrols    = new ContentPane({
        region   : 'top',
      });
      //this._topcontrols
      //TODO

      //--- Editor ---//

      this._editor         = new Editor({
        region   : 'center',
      });
      this._contextMenu    = new ContextMenu();
      this._prevClickedPos = {};

      this._editor.set('content', "(module not yet loaded)");
      
      this._editor.on('RightClick', function (clickedPos) {
        if (isNewClickPosition(that, clickedPos)) {
          if (!positionInRange(clickedPos, that._editor.get('selection'))) {
            var token = that._editor.getWordAt(clickedPos);

            that._editor.set('selection', {
              from : { line : clickedPos.line, column : token.start },
              to   : { line : clickedPos.line, column : token.end   }
            });
          } else {
            var selectedText = that._editor.getSelectedText();
          }

          buildContextMenu(that, clickedPos, selectedText);
        }
      });
      require(['dojo/query', 'dojo/domReady!'], function (query) {
        that._contextMenu.bindDomNode(
          query('.CodeMirror-scroll', that._editor.domNode.lastChild)[0]);
      });
      
      //--- Messaging ---//

      topic.subscribe('codecompass/versionOpenText', function (message) {
        if (message.newTab)
          return;

        that._initializeVersionTextComponent(
          message.versionrepoid,
          message.versioncommitid,
          message.versionpath
        );
      });
    },

    postCreate : function () {
      this.addChild(this._topcontrols);
      this.addChild(this._editor);
    },
    
    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if (state.cent !== this.id)
        return;
      
      console.log("VersionText::setState");

      var versionrepoid   = state.versionrepoid   || "";
      var versioncommitid = state.versioncommitid || "";
      var versionpath     = state.versionpath     || "";

      this._initializeVersionTextComponent(
        versionrepoid,
        versioncommitid,
        versionpath
      );
    },
    
    _initializeVersionTextComponent : function (
        versionrepoid,
        versioncommitid,
        versionpath
    ) {
      
      this.versionrepoid = versionrepoid;
      this.versioncommitid = versioncommitid;
      this.versionpath = versionpath;

      try {
        versionblobid = model.versionservice.getBlobOidByPath(
          versionrepoid,
          versioncommitid,
          versionpath
        );
        
        console.log(versionblobid);
      
        var fileInfo = {
          path: versioncommitid + "/" + versionpath,
          parseStatus: model.FileParseStatus.VCView
        };
        this._editor.setHeader(fileInfo);
        
        //remove previous annotations
        this._editor.setLineNumberFormatter();

        var blob = model.versionservice.getBlob(versionrepoid, versionblobid);
        this._editor.set('content', blob.data);
        
      } catch (err) {
        //show some error message to user on error.
        //TODO more concrete error message
        //TODO path node should not be an error
        
        console.log(err);
        this._editor.set(
          'content',
          "(Could not load text data." +
          " Invalid path or the path represents a directory.)"
        );
        
      }
      
    },
    


  });
});
