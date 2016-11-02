define([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/astHelper',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, astHelper, model, viewHandler) {

  model.addService('cppservice', 'CppService', LanguageServiceClient);

  var getdefintion = {
    id : 'cpp-text-getdefintion',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Jump to definition',
        accelKey : 'ctrl - click',
        onClick : function () {
          var languageService = model.getLanguageService(fileInfo.type);
          astHelper.jumpToDef(nodeInfo.id, model.cppservice);
        }
      });
    }
  };

  viewHandler.registerModule(getdefintion, {
    type : viewHandler.moduleType.TextContextMenu,
    service : model.cppservice
  });

  var infoTree = {
    id : 'cpp-text-infotree',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Info Tree',
        onClick : function () {
          topic.publish('codecompass/infotree', {
            fileType : fileInfo.type,
            elementInfo : nodeInfo
          });
        }
      });
    }
  };

  viewHandler.registerModule(infoTree, {
    type : viewHandler.moduleType.TextContextMenu,
    service : model.cppservice
  });

  var diagrams = {
    id : 'cpp-text-diagrams',
    render : function (nodeInfo, fileInfo) {
      var submenu = new Menu();

      var diagramTypes = model.cppservice.getDiagramTypes(nodeInfo.id);
      for (diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label : diagramType,
          onClick : function () {
            topic.publish('codecompass/openDiagram', {
              handler : 'cpp-ast-diagram',
              diagramType : diagramTypes[diagramType],
              node : nodeInfo.id
            });
          }
        }));

      if (Object.keys(diagramTypes).length !== 0)
        return new PopupMenuItem({
          label : 'Diagrams',
          popup : submenu
        });
    }
  };

  viewHandler.registerModule(diagrams, {
    type : viewHandler.moduleType.TextContextMenu,
    service : model.cppservice
  });
});
