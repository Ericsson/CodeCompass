define([
  'dojo/topic',
  'dijit/MenuItem',
  'codecompass/astHelper',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, MenuItem, astHelper, model, viewHandler) {

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
});
