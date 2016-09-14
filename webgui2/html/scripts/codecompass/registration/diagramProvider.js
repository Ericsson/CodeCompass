define([
  'codecompass/urlHandler',
  'codecompass/registration/model'],
function (urlHandler, model) {
  
  function getAstDiagram(diagramType) {
    var languageService;
    
    return {
      getDiagram : function (nodeId, fileInfo, callback) {
        var astNodeId = new model.AstNodeId();
        astNodeId.astNodeId = nodeId;
        languageService = model.getLanguageService(fileInfo.type);
        return languageService.getDiagram(astNodeId, diagramType, callback);
      },

      getTooltip : function (nodeId, callback) {
        return languageService.getInfoBox(nodeId, function (infoBox) {
          callback(infoBox.documentation);
        });
      },

      mouseOverInfo : function (nodeId) {
        var astNodeId = new model.AstNodeId();
        astNodeId.astNodeId = nodeId;

        var fileRange = languageService.getAstNodeInfo(astNodeId).range;

        return {
          fileId    : fileRange.file,
          selection : {
            from : {
              line   : fileRange.range.startpos.line,
              column : fileRange.range.startpos.column },
            to : {
              line   : fileRange.range.endpos.line,
              column : fileRange.range.endpos.column }
          }
        };
      }
    };
  }
  
  function getDirectoryDiagram(diagramType) {
    return {
      getDiagram : function (nodeId, fileInfo, callback) {
        var fileId = new model.FileId();
        fileId.fid = nodeId;
        // TODO: cppservice shouldn't be used explicitly
        return model.cppservice.getFileDiagram(fileId, diagramType, callback);
      },

      mouseOverInfo : function (nodeId) {
        var astNodeId = new model.AstNodeId();
        astNodeId.astNodeId = nodeId;
        var fileRange = model.cppservice.getAstNodeInfo(astNodeId).range;

        return {
          fileId    : fileRange.file,
          selection : {
            from : {
              line   : fileRange.range.startpos.line,
              column : fileRange.range.startpos.column },
            to : {
              line   : fileRange.range.endpos.line,
              column : fileRange.range.endpos.column }
          }
        };
      }
    };
  }
  
  function getFileDiagram(diagramType) {
    return {
      getDiagram : function (nodeId, fileInfo, callback) {
        var fileId = new model.FileId();
        fileId.fid = nodeId;
        // TODO: cppservice shouldn't be used explicitly
        return model.cppservice.getFileDiagram(fileId, diagramType, callback);
      },
      
      mouseOverInfo : function (nodeId) {
        var fileId = new model.FileId();
        fileId.fid = nodeId;
        
        return {
          fileId    : fileId,
          selection : {
            from : { line : 1, column : 1 },
            to   : { line : 1, column : 1 }
          }
        };
      }
    };
  }
  
  var callPathDiagram = {
    getTooltip : function (nodeId, callback) {
      return model.cppservice.getInfoBox(nodeId, function (infoBox) {
        callback(infoBox.documentation);
      });
    },

    getDiagram : function (nodeId, fileInfo, callback) {
      var astNodeId = new model.AstNodeId();
      astNodeId.astNodeId = nodeId;
      
      var fromNode = new model.AstNodeId();
      fromNode.astNodeId = urlHandler.getState().diagFrom;
      
      return model
            .getLanguageService(fileInfo.type)
            .getFunctionCallPathDiagram(fromNode, astNodeId, callback);
    }
  };
  
  var diagramTypes = {};
  
  diagramTypes[model.DiagramId.FUNCTION_CALL]
    = getAstDiagram(model.DiagramId.FUNCTION_CALL);
  diagramTypes[model.DiagramId.CLASS]
    = getAstDiagram(model.DiagramId.CLASS);
  diagramTypes[model.DiagramId.POINTER_ANALYSIS]
    = getAstDiagram(model.DiagramId.POINTER_ANALYSIS);
  diagramTypes[model.DiagramId.FULL_CLASS]
    = getAstDiagram(model.DiagramId.FULL_CLASS);
  diagramTypes[model.DiagramId.DIR_FULL_CLASS]
    = getDirectoryDiagram(model.DiagramId.DIR_FULL_CLASS);
  diagramTypes[model.DiagramId.CLASS_COLLABORATION]
    = getAstDiagram(model.DiagramId.CLASS_COLLABORATION);
  diagramTypes[model.DiagramId.FUNCTION_PATH]
    = callPathDiagram;
  diagramTypes[model.DiagramId.INCLUDE_DEPENDENCY]
    = getFileDiagram(model.DiagramId.INCLUDE_DEPENDENCY);
  diagramTypes[model.DiagramId.INTERFACE]
    = getFileDiagram(model.DiagramId.INTERFACE);
  diagramTypes[model.DiagramId.COMPONENTS_USED]
    = getFileDiagram(model.DiagramId.COMPONENTS_USED);
  diagramTypes[model.DiagramId.COMPONENT_USERS]
    = getFileDiagram(model.DiagramId.COMPONENT_USERS);
  diagramTypes[model.DiagramId.SUBSYSTEM_IMPLEMENT]
    = getFileDiagram(model.DiagramId.SUBSYSTEM_IMPLEMENT);
  diagramTypes[model.DiagramId.SUBSYSTEM_DEPENDENCY]
    = getFileDiagram(model.DiagramId.SUBSYSTEM_DEPENDENCY);
  diagramTypes[model.DiagramId.EXTERNAL_IMPLEMENTS]
    = getFileDiagram(model.DiagramId.EXTERNAL_IMPLEMENTS);
  diagramTypes[model.DiagramId.EXTERNAL_DEPENDENCIES]
    = getFileDiagram(model.DiagramId.EXTERNAL_DEPENDENCIES);
  diagramTypes[model.DiagramId.EXTERNAL_USERS]
    = getFileDiagram(model.DiagramId.EXTERNAL_USERS);
  
  return { diagramTypes : diagramTypes };
});