define([
  'dojo/_base/lang',
  'dojo/hash',
  'dojo/topic',
  'codecompass/model'],
function (lang, hash, topic, model) {

  /**
   * URL state object
   */
  var state = {};

  /**
   * Thrift FileInfo object which associated with the URL `fid` state.
   */
  var fileInfo = null;

  /**
   * File content which associated with the URL `fid` state.
   */
  var fileContent = null;

  /**
   * Language service API associated with the URL `fid` state.
   */
  var service = null;
  
  /**
   * AST node information associated with the URL `node` state.
   */
  var astNodeInfo = null;

  var urlHandler = {
    /**
     * This function pushes the currently stored state to the browser history.
     * During the update process, the "hashSetProgress" variable of the
     * urlHandler object is set to true. This signs that the url update in the
     * browser was done by urlHandler, not by the "browser back" or "browser
     * forward" buttons.
     */
    updateUrl : function () {
      var that = this;

      var hashValue = [];

      for (var s in state)
        hashValue.push(s + '=' + state[s]);
      this.hashSetProgress = true;

      hash(hashValue.join('&'));

      this._cacheDbInformation();

      setTimeout(function () { that.hashSetProgress = false; }, 0);
    },

    /**
     * This function sets the url state and pushes the state to the browser
     * history. The function can be called with 1 or 2 parameters. In case of
     * one parameter it has to be an object (key-value pairs), in case of two
     * parameters a single key and value. In both cases the given value(s) are
     * modified, the rest of the current state is untouched. For example setting
     * the key1-value1 and key2-value2 pairs means that the url will contain
     * #key1=value1&key2=value2 hash.
     */
    setStateValue : function (key, value) {
      if (lang.isObject(key))
        for (var k in key)
          state[k] = key[k];
      else
        state[key] = value;

      this.updateUrl();
    },

    /**
     * This function deletes a value from the current state at the given key,
     * and pushes the state to the browser history.
     */
    unsetStateValue : function (key) { delete state[key]; this.updateUrl(); },

    /**
     * This function overwrites the whole current state, and pushes the new
     * state to the browser history.
     */
    setState : function (stateValues) {
      state = lang.clone(stateValues);
      this.updateUrl();
    },

    /**
     * This function returns the whole current value associated with the given
     * key in the URL. If key is not given then the whole state from the browser
     * url as a JavaScript object is returned.
     */
    getState : function (key) {
      if (key)
        return state[key];

      var result = {};

      var h = hash();

      h.split('&').forEach(function (element) {
        var pos = element.indexOf('=');
        if (pos !== -1)
          result[element.substr(0, pos)] = element.substr(pos + 1);
      });

      return result;
    },

    /**
     * This function returns the current URL as string.
     */
    getURL : function () {
      return document.URL;
    },

    /**
     * This function returns the thrift FileInfo which associated with the URL
     * `fid` state.
     */
    getFileInfo : function () {
      return fileInfo;
    },

    /**
     * This function returns the file content which associated with the URL 
     * `fid` state.
     */
    getFileContent : function () {
      return fileContent;
    },

    /**
     * This function returns a Thrift language service object which associated
     * with the URL `fid` state.
     */
    getLanguageService : function () {
      return service;
    },

    /**
     * This function returns the AST node information which associated with the
     * URL `node` state.
     */
    getAstNodeInfo : function () {
      return astNodeInfo;
    },

    /**
     * This function caches database information which associated with the URL
     * states such as file information, file content, service etc.
     */
    _cacheDbInformation : function () {
      var fileId = this.getState('fid');
      if (fileId && (!fileInfo || fileInfo.id !== fileId)) {
        fileInfo = model.project.getFileInfo(fileId);
        service  = model.getLanguageService(fileInfo.type);
        fileContent = model.project.getFileContent(fileInfo.id);
      }

      var nodeId = this.getState('node');
      if (nodeId && service && (!astNodeInfo || astNodeInfo.id !== nodeId)) {
        astNodeInfo = service.getAstNodeInfo(nodeId);
      }
    }
  };

  //--- Init state by url ---//

  state = urlHandler.getState();

  /**
   * When "browser back" or "browser forward" button is pressed, then the global
   * state object is set to the changed url hash, so that the state variable and
   * the current url be synchronized. 
   */
  topic.subscribe('/dojo/hashchange', function (url) {
    if (urlHandler.hashSetProgress) return;

    state = {};

    url.split('&').forEach(function (part) {
      var elem = part.split('=');
      state[elem[0]] = elem[1];
    });

    urlHandler._cacheDbInformation();
  });

  return urlHandler;
});
