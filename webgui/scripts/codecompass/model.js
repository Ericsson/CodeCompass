define([
  'codecompass/urlHandler',
  'exports'],
function (urlHandler, exports) {

  /**
   * This object is mapping a file type with a service.
   */
  exports._fileTypesToService = {};

  /**
   * Get service identified by name. Register it if it doesn't exist.
   */
  exports.addService = function (name, url, Client) {
    if (!this[name]) {
      // These two services are independent from the workspaces, so these are
      // on an URL which don't contain the workspace name.
      if (url !== 'AuthenticationService' &&
          url !== 'PluginService' &&
          url !== 'WorkspaceService')
        url = workspace + '/' + url;

      var service
        = new Client(new Thrift.Protocol(new Thrift.Transport(url)));

      //--- Map filetypes to service for each language service ---//

      if (service instanceof LanguageServiceClient){
        service.getFileTypes().forEach(function (fileType) {
          exports._fileTypesToService[fileType] = service;
        });
      }

      this[name] = service;
    }

    return this[name];
  }

  /**
   * This function returns the appropriate language service API belonging to
   * the given file type.
   * @param {String} fileType A file type which is gathered from one of
   * LanguageService getFileTypes() function.
   */
  exports.getLanguageService = function (fileType) {
    return exports._fileTypesToService[fileType];
  }

  //--- Initial common services (workspace, plugin, project) ---//

  exports.addService('authentication', 'AuthenticationService',
    AuthenticationServiceClient);

  exports.addService('plugin', 'PluginService', PluginServiceClient);
  exports.addService('workspace', 'WorkspaceService', WorkspaceServiceClient);

  var workspace = urlHandler.getState('wsid') ||
    exports.workspace.getWorkspaces()[0].id;

  exports.addService('project', 'ProjectService', ProjectServiceClient);

  return exports;
});
