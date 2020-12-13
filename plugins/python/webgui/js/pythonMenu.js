require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/astHelper',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, astHelper, model, viewHandler){
  model.addService('pythonservice', 'PythonService', LanguageServiceClient);

  var getdefinition = {
    id : 'python-text-getdefinition',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Jump to definition',
        accelKey : 'ctrl - click',
        onClick : function () {
          if (!nodeInfo || !fileInfo) {
            return;
          }

          // var languageService = model.getLanguageService(fileInfo.type);
          astHelper.jumpToDef(nodeInfo.id, model.pythonservice);
        }
      });
    }
  };

  viewHandler.registerModule(getdefinition, {
    type : viewHandler.moduleType.TextContextMenu,
    service : model.pythonservice
  });

  var infoTree = {
    id : 'python-text-infotree',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Info Tree',
        onClick : function () {
          if (!nodeInfo || !fileInfo) {
            return;
          }

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
    service : model.pythonservice
  });

  var infobox = {
    id : 'python-text-infobox',
    render : function (nodeInfo, fileInfo) {
      return new MenuItem({
        label : 'Documentation',
        onClick : function () {
          topic.publish('codecompass/documentation', {
            fileType    : fileInfo.type,
            elementInfo : nodeInfo
          });
        }
      });
    }
  };

  viewHandler.registerModule(infobox, {
    type : viewHandler.moduleType.TextContextMenu,
    service : model.pythonservice
  });
});