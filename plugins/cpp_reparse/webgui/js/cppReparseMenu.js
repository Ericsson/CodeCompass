require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {

  model.addService('cppreparseservice', 'CppReparseService',
                   CppReparseServiceClient);

  if (!model.cppreparseservice.isEnabled())
    // Don't create the menus if the server can't reparse.
    return;

  //--- Reparse menu for source codes ---//
  var nodeMenu = {
    id : 'cppreparse-node',
    render : function (nodeInfo, fileInfo) {
      if (fileInfo.type !== "CPP")
        return;

      var submenu = new Menu();

      submenu.addChild(new MenuItem({
        label : 'Show syntax InfoTree',
        onClick : function () {
          topic.publish('codecompass/infotree', {
            fileType : 'cpp-reparse',
            elementInfo : nodeInfo
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : 'Show AST HTML',
        onClick : function () {
          topic.publish('codecompass/cppreparsenode', {
            fileInfo : fileInfo,
            nodeInfo : nodeInfo
          });
        }
      }));

      return new PopupMenuItem({
        label : 'C++',
        popup : submenu
      });
    }
  };

  viewHandler.registerModule(nodeMenu, {
    type     : viewHandler.moduleType.TextContextMenu,
    priority : 10
  });

  //--- Reparse menu for source files in the project manager ---//
  var fileMenu = {
    id : 'cppreparse-file',
    render : function (fileInfo) {
      if (fileInfo.type !== "CPP")
        return;

      var submenu = new Menu();

      submenu.addChild(new MenuItem({
        label : 'Show syntax InfoTree',
        onClick : function () {
          topic.publish('codecompass/infotree', {
              fileType : 'cpp-reparse',
              elementInfo : fileInfo
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : 'Show AST HTML',
        onClick : function () {
          topic.publish('codecompass/cppreparsefile', {
            fileInfo : fileInfo
          });
        }
      }));

      return new PopupMenuItem({
        label : 'C++',
        popup : submenu
      });
    }
  };

  viewHandler.registerModule(fileMenu, {
    type     : viewHandler.moduleType.FileManagerContextMenu,
    priority : 10
  });
});
