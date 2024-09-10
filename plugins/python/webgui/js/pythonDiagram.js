require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {

  model.addService('pythonservice', 'PythonService', LanguageServiceClient);

  var astDiagram = {
    id : 'python-ast-diagram',

    getDiagram : function (diagramType, nodeId, callback) {
      model.pythonservice.getDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.pythonservice.getDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      var nodeInfo = model.pythonservice.getAstNodeInfo(nodeId);
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
    id : 'python-file-diagram-handler',

    getDiagram : function (diagramType, nodeId, callback) {
      model.pythonservice.getFileDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.pythonservice.getFileDiagramLegend(diagramType);
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
    id : 'python-file-diagrams',
    render : function (fileInfo) {
      var submenu = new Menu();

      var diagramTypes = model.pythonservice.getFileDiagramTypes(fileInfo.id);
      for (diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label   : diagramType,
          type    : diagramType,
          onClick : function () {
            var that = this;

            topic.publish('codecompass/openFile', { fileId : fileInfo.id });

            topic.publish('codecompass/openDiagram', {
              handler : 'python-file-diagram-handler',
              diagramType : diagramTypes[that.type],
              node : fileInfo.id
            });
          }
        }));

      if (Object.keys(diagramTypes).length !== 0)
        return new PopupMenuItem({
          label : 'Python Diagrams',
          popup : submenu
        });
    }
  };

  viewHandler.registerModule(fileDiagrams, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});
