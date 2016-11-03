require([
  'codecompass/model',
  'codecompass/viewHandler'],
function (model, viewHandler) {
  model.addService('cppservice', 'CppService', LanguageServiceClient);

  var astDiagram = {
    id : 'cpp-ast-diagram',

    getDiagram : function (diagramType, nodeId, callback) {
      model.cppservice.getDiagram(nodeId, diagramType, callback);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      var nodeInfo = model.cppservice.getAstNodeInfo(nodeId);
      var range = nodeInfo.range.range;

      return {
        fileId : nodeInfo.range.file,
        selection : [
          range.startpos.line,
          range.startpos.column,
          range.endpos.line,
          range.endpos.column
        ]
      };
    }
  };

  viewHandler.registerModule(astDiagram, {
    type : viewHandler.moduleType.Diagram
  });
});
