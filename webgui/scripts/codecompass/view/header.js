require([
  'dojo/cookie',
  'dojo/topic',
  'dojo/_base/declare',
  'dijit/DropDownMenu',
  'dijit/form/DropDownButton',
  'dijit/form/Select',
  'dijit/MenuItem',
  'dijit/layout/ContentPane',
  'codecompass/model',
  'codecompass/urlHandler',
  'codecompass/viewHandler'],
function (cookie, topic, declare, DropDownMenu, DropDownButton, Select,
    MenuItem, ContentPane, model, urlHandler, viewHandler) {

  //--- Header Menu ---//

  var HeaderMenu = declare(ContentPane, {
    constructor : function () {
      //--- Drop down menu ---//

      var menu = new DropDownMenu();

      menu.addChild(new MenuItem({
        label     : 'About',
        iconClass : 'menuicon icon icon-info',
        onClick   : function () {
          topic.publish('codecompass/infopage', 'startpage');
        }
      }));

      menu.addChild(new MenuItem({
        label     : 'Report a bug here',
        iconClass : 'menuicon icon icon-bug',
        onClick   : function () {
          window.open(
            'https://github.com/Ericsson/CodeCompass/issues');
        }
      }));

      menu.addChild(new MenuItem({
        label     : 'User guide',
        iconClass : 'menuicon icon icon-star',
        onClick   : function () {
          topic.publish('codecompass/infopage', 'userguide');
        }
      }));

      menu.addChild(new MenuItem({
        label     : 'Credits',
        iconClass : 'menuicon icon icon-user',
        onClick   : function () {
          topic.publish('codecompass/infopage', 'credits');
        }
      }));

      menu.addChild(new MenuItem({
        label     : 'New Web GUI',
        onClick   : function () {
          window.open(
            `${window.location.pathname}/new`);
        }
      }));

      try {
        if (model.authentication.isRequiringAuthentication()) {
          menu.addChild(new MenuItem({
            label: 'Log out ' + model.authentication.getLoggedInUser(),
            iconClass: 'menuicon icon icon-project',
            onClick: function () {
              try {
                var logoutResult = false;
                try {
                  logoutResult = model.authentication.logout();
                } catch (ex) {
                  console.error(ex);
                }

                if (logoutResult) {
                  cookie(logoutResult, '<invalid>', {path: '/', expires: -1});

                  // Redirect the user to the homepage after successful logout.
                  window.location.reload(true);
                } else {
                  console.warn("Server rejected logout.");
                }
              } catch (exc) {
                console.error("Logout failed.", exc);
              }
            }
          }));
        }
      } catch (ex) {
        console.warn(ex);
      }

      this._menuButton = new DropDownButton({
        id       : 'menu-btn',
        dropDown : menu,
        label    : 'Menu',
      });

      //--- Workspaces ---//

      var workspaces = [];

      model.workspace.getWorkspaces().forEach(function (workspaceInfo) {
        workspaces.push({
          label : workspaceInfo.description,
          value : workspaceInfo.id
        });
      });

      this._workspaceButton = new Select({
        id       : 'workspace-btn',
        options  : workspaces,
        value    : urlHandler.getState('wsid') || null,
        onChange : function (value) {
          urlHandler.setState({ 'wsid' : value });
          location.reload(true);
        }
      });

      document.title = this._workspaceButton.get('value') + ' - CodeCompass';
    },

    postCreate : function () {
      this.addChild(this._workspaceButton);
      this.addChild(this._menuButton);
    }
  });

  var headerMenu = new HeaderMenu({
    id : 'cc-headermenu' 
  });

  viewHandler.registerModule(headerMenu, {
    type : viewHandler.moduleType.Header
  });

});
