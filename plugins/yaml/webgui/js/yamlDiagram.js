require([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojo/dom-style',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/form/Button',
  'dijit/form/CheckBox',
  'dijit/form/Select',
  'dijit/layout/ContentPane',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/model'],
function (declare, dom, topic, style, Menu, MenuItem, PopupMenuItem, Button, CheckBox, Select,
  ContentPane, viewHandler, urlHandler, model) {
  
  model.addService('yamlservice', 'YamlService', LanguageServiceClient);

  var fileDiagramHandler = {
    id : 'yaml-file-diagram-handler',

    getDiagram : function (diagramType, fileId, callback) {
      model.yamlservice.getFileDiagram(fileId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.yamlservice.getFileDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, fileId) {
      return {
        fileId : fileId,
        selection : [1,1,1,1]
      };
    }
  };

  viewHandler.registerModule(fileDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });

  var fileDiagrams = {
    id : 'yaml-file-diagrams',
    render : function (fileInfo) {
      var submenu = new Menu();

      var diagramTypes = model.yamlservice.getFileDiagramTypes(fileInfo.id);
      for (let diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label   : diagramType,
          type    : diagramType,
          onClick : function () {
            var that = this;

            topic.publish('codecompass/openFile', { fileId : fileInfo.id });

            topic.publish('codecompass/openDiagram', {
              handler : 'yaml-file-diagram-handler',
              diagramType : diagramTypes[that.type],
              node : fileInfo.id
            });
          }
        }));

      if (Object.keys(diagramTypes).length !== 0)
        return new PopupMenuItem({
          label : 'YAML Diagrams',
          popup : submenu
        });
    }
  };

  viewHandler.registerModule(fileDiagrams, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});