require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  var nodeMenu = {
    id : 'git-text-team-node',
    render : function (nodeInfo, fileInfo) {
      var submenu = new Menu();

      submenu.addChild(new MenuItem({
        label : 'Blame',
        onClick : function () {
          topic.publish('codecompass/gitblame', {
            fileInfo : fileInfo,
            nodeInfo : nodeInfo
          });
        }
      }));

      return new PopupMenuItem({
        label : 'Team',
        popup : submenu
      });
    }
  };

  viewHandler.registerModule(nodeMenu, {
    type     : viewHandler.moduleType.TextContextMenu,
    priority : 70
  });

  var fileMenu = {
    id : 'git-text-team-file',
    render : function (fileInfo) {
      if (fileInfo.type === "Dir")
        return;

      var submenu = new Menu();

      submenu.addChild(new MenuItem({
        label : 'Blame',
        onClick : function () {
          topic.publish('codecompass/gitblame', {
            fileInfo : fileInfo
          });
        }
      }));

      return new PopupMenuItem({
        label : 'Team',
        popup : submenu
      });
    }
  };

  viewHandler.registerModule(fileMenu, {
    type     : viewHandler.moduleType.FileManagerContextMenu,
    priority : 70
  });
});
