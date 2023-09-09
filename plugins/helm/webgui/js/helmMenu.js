require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/astHelper',
  'codecompass/model',
  'codecompass/urlHandler',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, astHelper, model, urlHandler, viewHandler) {

  model.addService('yamlservice', 'YamlService', YamlServiceClient);

  var infoTree = {
    id : 'yaml-text-infotree',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Info Tree',
        onClick : function () {
          if (!nodeInfo || !fileInfo)
            return;

          topic.publish('codecompass/infotree', {
            fileType : fileInfo.type,
            elementInfo : nodeInfo
          });

          if (window.gtag) {
            window.gtag ('event', 'info_tree', {
              'event_category' : urlHandler.getState('wsid'),
              'event_label' : urlHandler.getFileInfo().name
                + ': '
                + nodeInfo.astNodeValue
            });
          }
        }
      });
    }
  };

  viewHandler.registerModule(infoTree, {
      type : viewHandler.moduleType.TextContextMenu,
      service : model.yamlservice
  });

  var diagrams = {
    id : 'yaml-text-diagrams',
    render : function (nodeInfo, fileInfo) {
      if (!nodeInfo || !fileInfo)
        return;

      var submenu = new Menu();

      var diagramTypes = model.yamlservice.getDiagramTypes(nodeInfo.id);
      for (diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label   : diagramType,
          type    : diagramType,
          onClick : function () {
            var that = this;

            topic.publish('codecompass/openDiagram', {
              handler : 'yaml-ast-diagram',
              diagramType : diagramTypes[that.type],
              node : nodeInfo.id
            });
          }
        }));

      submenu.addChild(new MenuItem({
        label : "CodeBites",
        onClick : function () {
          topic.publish('codecompass/codebites', {
            node : nodeInfo
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
    service : model.yamlservice
  });
});
