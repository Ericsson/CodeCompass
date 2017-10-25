require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  var team = {
    id : 'git-text-team',
    render : function (nodeInfo, fileInfo) {
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

  viewHandler.registerModule(team, {
    type     : viewHandler.moduleType.TextContextMenu,
    priority : 70
  });
});
