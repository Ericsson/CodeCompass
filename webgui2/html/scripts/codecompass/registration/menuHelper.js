define([
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/MenuSeparator',
  'dojo/_base/array',
  'dojo/dom-style',
  'dojo/mouse',
  'dojo/topic',
  'codecompass/view/component/ContextMenu',
  'codecompass/registration/lineNumberFormatter',
  'codecompass/registration/model',
  'codecompass/util'],
function (MenuItem, PopupMenuItem, MenuSeparator, array, style, mouse,
  topic, ContextMenu, lineNumberFormatter, model,
  util) {
  
  /**
   * This function returns a FilePosition thrift object based on the given file
   * and position.
   * @param {Object} clickedPos Position object containing 'line' and 'column'
   * properties.
   * @param {FileId} fileId Thrift object.
   */
  function getFilePositionByPosition(clickedPos, fileId) {
    var fpos = new model.FilePosition(),
        pos  = new model.Position();

    pos.line   = clickedPos.line;
    pos.column = clickedPos.column;
    fpos.file  = fileId;
    fpos.pos   = pos;

    return fpos;
  }
  
  /**
   * This function publishes an openFile message to jump to a given astNode.
   * @param {FileRange} toFileRange Target node file range to jump.
   * @param {FileRange} fromFileRange Source node file range from where we jump.
   * This is needed to the browsing history.
   * @param {Boolean} newTab Describes if the target should be opened in a new
   * tab.
   * @param {String} info Text displayed in browsing history.
   */
  function jump(toFileRange, fromFileRange, newTab, info) {
    topic.publish('codecompass/openFile', {
      fileId    : toFileRange.file,
      moduleId  : 'text',
      line      : toFileRange.range.startpos.line,
      info      : info,
      newTab    : newTab,
      jumpFrom  : fromFileRange,
      jumpTo    : toFileRange,
      selection : {
        from : toFileRange.range.startpos,
        to   : toFileRange.range.endpos
      }
    });
  }
  
  return {
    buildFileContextmenu : function (contextmenu, fileInfo) {
      
      contextmenu.clear();
      
      //--- Get language service by file type or exit ---//

      var languageService = model.getLanguageService(fileInfo.type);

      contextmenu.addChild(new MenuItem({
        label    : 'Open in new tab',
        accelKey : 'MiddleClick',
        onClick  : function () {
          topic.publish('codecompass/openFile', {
            fileId   : fileInfo.file,
            moduleId : 'text',
            newTab   : true
          });
        }
      }));
      
      if (fileInfo.isDirectory)
        contextmenu.addChild(new MenuItem({
          label   : 'Add to directory filter',
          onClick : function () {
            topic.publish('codecompass/fileClick', {
              path    : fileInfo.path,
              display : fileInfo.path + '.*'
            });
          }
        }));

      if (model.metricsservice)
        contextmenu.addChild(new MenuItem({
          label   : 'Metrics',
          onClick : function () {
            topic.publish('codecompass/metrics', {
              fileId : fileInfo.file
            });
          }
        }));

      if (!languageService)
        return;
      
      //--- Build menu items ---//

      var menuTypes
        = fileInfo.isDirectory
        ? languageService.getDirMenuTypes(fileInfo.file)
        : languageService.getFileMenuTypes(fileInfo.file);

      array.forEach(menuTypes, function (menuType) {
        contextmenu.addChild({
          menuType : menuType,
          onClick  : function () {
            switch (menuType.category) {
              case model.Category.diagram:
                topic.publish('codecompass/openDiagram', {
                  diagramNode : fileInfo.file.fid,
                  diagramType : menuType.menuItemId
                });
                break;

              case model.Category.infoTree:
                topic.publish('codecompass/infotree', {
                  elementInfo : fileInfo,
                  fileType    : fileInfo.type
                });
                break;

            }
          }
        });
      });
    },
    
    // TODO: We should fill menu items lazily when needed.
    buildFileDiagramContextmenu : function (contextmenu, fileInfo) {
      contextmenu.clear();
      
      var languageService = model.getLanguageService(fileInfo.type);
      
      if (!languageService)
        return;
      
      //--- File diagrams ---//
      
      languageService.getFileMenuTypes(fileInfo.file).forEach(
      function (menuType) {
        if (menuType.category === model.Category.diagram) {
          menuType.name.shift(); // Remove "Diagram" label layer.
          contextmenu.addChild({
            menuType : menuType,
            onClick  : function () {
              topic.publish('codecompass/openDiagram', {
                diagramNode : fileInfo.file.fid,
                diagramType : menuType.menuItemId
              });
            }
          });
        }
      });
      
      contextmenu.addChild(new MenuSeparator());
      
      //--- Directory diagrams ---//
      
      var popupMenuItems = {};
      
      var paths = model.project.getPathTillFile(fileInfo.file);
      paths.pop();
      
      paths.forEach(function (fileInfo) {
        // TODO: Can we suppose that all dirs have the same menu types?
        // Because this loop could be eliminated.
        languageService.getDirMenuTypes(fileInfo.file, function(menuTypes){
          array.forEach(menuTypes, function(menuType){
            menuType.name.shift(); // Remove "Diagram" label layer.
            var name = menuType.name[0];

            var popupMenuItem;
            if (name in popupMenuItems)
              popupMenuItem = popupMenuItems[name];
            else {
              popupMenuItem = new PopupMenuItem({
                label : name,
                popup : new ContextMenu()
              });

              popupMenuItems[name] = popupMenuItem;
              contextmenu.addChild(popupMenuItem);
            }

            menuType.name = [fileInfo.path];
            popupMenuItem.get('popup').addChild({
              menuType : menuType,
              onClick  : function () {
                topic.publish('codecompass/openDiagram', {
                  diagramNode : fileInfo.file.fid,
                  diagramType : menuType.menuItemId
                });
              }
            });
          });
        });
      });
    },
    
    buildMetricsContextmenu : function (contextmenu, fileInfo) {
      contextmenu.clear();
      
      var languageService = model.getLanguageService(fileInfo.type);
      
      if (!languageService)
        return;
      
      var popupMenuItems = {};
      
      var paths = model.project.getPathTillFile(fileInfo.file);
      paths.pop();
      
      paths.forEach(function (fileInfo) {
        contextmenu.addChild(new MenuItem({
          label   : fileInfo.path,
          onClick : function () {
            topic.publish('codecompass/metrics', {
              fileId : fileInfo.file
            });
          }
        }));
      });
    },
    
    /**
     * This function jumps to the definition of the node or place given in
     * jumpFrom parameter.
     * @param {AstNodeId | AstNodeInfo | Object} jumpFrom This object can be an
     * AstNodeId, AstNodeInfo which is the node of which the definition we want
     * to jump to. It can also be an object which contains a fileInfo attribute
     * of type FileInfo and a text attribute of type String in which case we
     * provide a text search and show the first few results in a table so the
     * user can chose one. If an AST node is given and the definition is
     * ambiguous, then the user has to chose the definition node to which he
     * wants to jump.
     * @param {Boolean} newTab If true, then the result is opened in a new tab.
     * @param {Object} languageService The language service which is used to
     * find the definition of the node. This parameter is not used when jumpFrom
     * parameter is not an AstNodeId or AstNodeInfo.
     */
    jumpToDef : function (jumpFrom, newTab, languageService) {
      var astNodeInfo = null;

      if (jumpFrom instanceof model.AstNodeId)
        astNodeInfo = languageService.getAstNodeInfo(jumpFrom);
      else if (jumpFrom instanceof model.AstNodeInfo)
        astNodeInfo = jumpFrom;
      
      if (!astNodeInfo) {
        
        // If no AST node for clicked position, e.g. not parsed file
        
        var params = new model.SearchParams();
        var query  = new SearchQuery();
        var range  = new SearchRange();
        
        query.query = '"' + jumpFrom.text.replace(/:/g, '\\:') + '"';
        
        range.start   = 0;
        range.maxSize = 10; 
       
        params.options = model.SearchOptions.SearchInSource;
        params.query   = query;
        params.range   = range;
        
        var result = model.searchservice.search(params);
        
        util.buildAmbiguousRefPage(result.results, function (toFileRange) {
          jump(toFileRange, jumpFrom.fileRange, newTab, info);
        });
        
      } else if (astNodeInfo.astNodeType === 'File') {

        // When click on #include ...
        // TODO: Should be more general than C/C++ #include

        var fileId = new model.FileId();
        fileId.fid = astNodeInfo.astNodeValue;

        var from = Math.max(astNodeInfo.astNodeSrcText.indexOf('"'),
                            astNodeInfo.astNodeSrcText.indexOf('<'));
        var to   = Math.max(astNodeInfo.astNodeSrcText.lastIndexOf('"'),
                            astNodeInfo.astNodeSrcText.lastIndexOf('>'));

        topic.publish('codecompass/openFile', {
          fileId   : fileId,
          moduleId : 'text',
          newTab   : newTab,
          info     : 'Open File: '
                   + astNodeInfo.astNodeSrcText.substring(from + 1, to)
        });

      } else {

        // When click on AST node

        var astNodeInfos = languageService.getReferences(
          astNodeInfo.astNodeId,
          model.RefTypes.GetDef);

        var info = 'Jump to definition: '
                 + '<b>' + astNodeInfo.astNodeValue + '</b>';

        if (astNodeInfos.length === 0)
          console.warning("Can't find definition to this node");
        else if (astNodeInfos.length === 1)
          jump(astNodeInfos[0].range, astNodeInfo.range, newTab, info);
        else
          util.buildAmbiguousRefPage(astNodeInfos, function (toFileRange) {
            jump(toFileRange, astNodeInfo.range, newTab, info);
          });
      }
    },
    
    /**
     * This function queries data from server to display infobox.
     * @param {Text} textModule Text instance.
     * @param {AstNodeId | AstNodeInfo} astNode Thrift object.
     */
    displayInfobox : function (textModule, astNode) {
      function show(nodeInfo) {
        var clickedFileInfo = model.project.getFileInfo(nodeInfo.range.file);
        var infoBox = textModule._languageService.getInfoBox(nodeInfo.astNodeId);

        textModule._infoBoxWidget.set('fileInfo', clickedFileInfo);
        textModule._infoBoxWidget.set('data', infoBox);
        textModule._infoBoxWidget.set('style', 'display: block');
        textModule._infoBoxWidget.resize();
      }
      
      var astNodeInfo = astNode instanceof model.AstNodeInfo
                      ? astNode
                      : textModule._languageService.getAstNodeInfo(astNode);
      
      var astNodeInfos
        = astNodeInfo.astNodeType === 'Macro'
        ? [astNodeInfo]
        : textModule._languageService.getReferences(
            astNodeInfo.astNodeId,
            model.RefTypes.GetDef);
      
      if (astNodeInfos.length === 0)
        console.warning("Can't find definition to this node");
      else if (astNodeInfos.length === 1)
        show(astNodeInfos[0]);
      else
        util.buildAmbiguousRefPage(astNodeInfos, function (nodeInfo) {
          show(nodeInfo);
        });
    },
    
    /**
     * This function runs when a menu item coming from server side is clicked.
     * @pre This function supposes that clicked position is valid i.e.
     * astNodeInfo belongs to a real AST node.
     * @param {Text} textModule An instance of this Text module.
     * @param {MenuType} menuType Thrift object. This parameter identifies the
     * menu category which says which api function should be called or what
     * should happen in case of client side added menu items.
     * @param {clickedContextAstNodeId} clicked element astNodeId
     */
    contextMenuClicked : function (textModule, menuType, clickedContextAstNodeId) {

      switch (menuType.category) {
        case model.Category.jumpToDef:
          this.jumpToDef(
            clickedContextAstNodeId, false, textModule._languageService);
          break;
          
        case model.Category.infoBox:
          this.displayInfobox(textModule, menuType.astNodeId);
          break;
        
        case model.Category.infoTree:
          topic.publish('codecompass/infotree', {
            fileType    : textModule._fileInfo.type,
            elementInfo : textModule._languageService.getAstNodeInfo(
              menuType.astNodeId)
          });
          break;
        
        case model.Category.references:
        case model.Category.pagingResult:
          topic.publish('codecompass/references', {
            astNodeId  : menuType.astNodeId,
            fileType   : textModule._fileInfo.type,
            refType    : menuType.menuItemId
          });
          break;

        case model.Category.diagram:
          topic.publish('codecompass/openDiagram', {
            diagramNode : menuType.astNodeId.astNodeId,
            diagramType : menuType.menuItemId,
            fromNodeId  : textModule._fromFunctionNodeInfo
                        ? textModule._fromFunctionNodeInfo.astNodeId
                        : null
          });
          break;

        case model.Category.codebites:
          topic.publish('codecompass/codebites', {
            astNodeInfo : textModule._languageService.getAstNodeInfo(
              menuType.astNodeId)
          });
          break;
          
        case model.Category.slicing:
          var astNodeInfo
            = textModule._languageService.getAstNodeInfo(menuType.astNodeId);
    
          var filePos = new model.FilePosition;
          
          filePos.file = textModule._fileInfo.file;
          filePos.pos  = astNodeInfo.range.range.startpos;
          
          var ranges;
          
          switch (menuType.menuItemId) {
            case model.SlicingTypes.Before:
              ranges = textModule._languageService.getBackwardSlicePos(filePos);
              break;
            case model.SlicingTypes.After:
              ranges = textModule._languageService.getForwardSlicePos(filePos);
              break;
          }
          
          textModule._editor.clearAllMarks();
          
          ranges.forEach(function (range) {
            textModule._editor.markText(range.startpos, range.endpos, {
              className : 'cb-marked-slicing'
            });
          });
          break;
      }
    },
    
    /**
     * This function returns an AstNodeInfo object by a given position or null
     * in case of bad position.
     * @param {FileInfo} fileInfo Thrift object.
     * @param {Object} clickedPos Position object containing 'line' and 'column'
     * properties.
     */
    getAstNodeInfoByPosition : function (fileInfo, clickedPos) {
      try {
        var languageService = model.getLanguageService(fileInfo.type);
        return languageService.getAstNodeInfoByPosition(
          getFilePositionByPosition(clickedPos, fileInfo.file));
      } catch (error) {
        return null;
      }
    },
    
    publishSearch : function (text, searchType) {
      topic.publish('codecompass/search', {
        text       : text.replace(/:/g, '\\:'),
        fileFilter : '',
        dirFilter  : '',
        searchType : searchType
      });
    },
    
    toggleAnnotateView : function (
      editor, versionrepoid, versioncommitid, versionpath, maybeModified) {
      
      return function () {
        editor.setLineNumberFormatter(
          editor._isSetFormatter
          ? undefined
          : lineNumberFormatter.getBlameFormatter(
              versionrepoid, versioncommitid, versionpath, maybeModified));
        
        editor._isSetFormatter = !editor._isSetFormatter;
      };
    },

    enterHistoryView : function (
      editor, versionrepoid, versionbranch, versionpath) {
      
      return function (event) {
        topic.publish('codecompass/versionOpenHistory', {
          newTab          : mouse.isMiddle(event),
          versionrepoid   : versionrepoid,
          versionbranch   : versionbranch,
          versionpath     : versionpath
        });
      };
    }
  };
});
