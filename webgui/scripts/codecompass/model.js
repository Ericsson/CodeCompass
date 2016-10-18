define([
  'codecompass/urlHandler'],
function (urlHandler) {

  //--- Model object ---//

  var model = {
    _fileTypesToService : {},

    /**
     * Get service identified by name. Registrate if doesn't exist.
     */
    addService : function (name, url, Client) {
      if (!this[name]) {
        // These two services are independent from the workspaces, so these are
        // on an URL which don't contain the workspace name.
        if (url !== 'PluginService' && url !== 'WorkspaceService')
          url = workspace + '/' + url;

        var service
          = new Client(new Thrift.Protocol(new Thrift.Transport(url)));

        //--- Map filetypes to service for each language service ---//

        if (service.getFileTypes){
          service.getFileTypes().forEach(function (fileType) {
            model._fileTypesToService[fileType] = service;
          });
        }

        this[name] = service;
      }

      return this[name];
    },

    /**
     * This function returns the appropriate language service API belonging to
     * the given file type.
     * @param {String} fileType A file type which is gathered from one of
     * LanguageService getFileTypes() function.
     */
    getLanguageService : function (fileType) {
      return this._fileTypesToService["CPP"];
      return this._fileTypesToService[fileType];
    }
  };

  //--- Initial common services (workspace, plugin, project) ---//

  model.addService('plugin', 'PluginService', PluginServiceClient);
  model.addService('workspace', 'WorkspaceService', WorkspaceServiceClient);

  var workspace
    = urlHandler.getState('wsid') || model.workspace.getWorkspaces()[0].id;

  model.addService('project', 'ProjectService', ProjectServiceClient);

  return model;
});
