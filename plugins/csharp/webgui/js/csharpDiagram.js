require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {
  model.addService('csharpservice', 'CsharpService', LanguageServiceClient);

  var astDiagram = {
    id : 'csharp-ast-diagram',

    getDiagram : function (diagramType, nodeId, callback) {
      model.csharpservice.getDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.csharpservice.getDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      var nodeInfo = model.csharpservice.getAstNodeInfo(nodeId);
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

  var fileDiagramHandler = {
    id : 'csharp-file-diagram-handler',

    getDiagram : function (diagramType, nodeId, callback) {
      model.csharpservice.getFileDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.csharpservice.getFileDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      return {
        fileId : nodeId,
        selection : [1,1,1,1]
      };
    }
  };

  viewHandler.registerModule(fileDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });

  var fileDiagrams = {
    id : 'csharp-file-diagrams',
    render : function (fileInfo) {
      var submenu = new Menu();

      var diagramTypes = model.csharpservice.getFileDiagramTypes(fileInfo.id);
      for (diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label   : diagramType,
          type    : diagramType,
          onClick : function () {
            var that = this;

            topic.publish('codecompass/openFile', { fileId : fileInfo.id });

            topic.publish('codecompass/openDiagram', {
              handler : 'csharp-file-diagram-handler',
              diagramType : diagramTypes[that.type],
              node : fileInfo.id
            });
          }
        }));

      if (Object.keys(diagramTypes).length !== 0)
        return new PopupMenuItem({
          label : 'C++ Diagrams',
          popup : submenu
        });
    }
  };

  viewHandler.registerModule(fileDiagrams, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});
