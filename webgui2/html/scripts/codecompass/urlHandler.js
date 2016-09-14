define([
  'dojo/_base/array',
  'dojo/_base/lang',
  'dojo/hash',
  'dojo/topic'],
function (array, lang, hash, topic) {
  var state = {};
  
  function statesToString() {
    /**
     * This function escapes some special characters in a string which are used as separator
     * character in the url.
     * @param {String} value Value to escape
     */
    function escape(value) {
      return value.match(/["|;\\]/)
        ? '"' +
          value.replace(/\\/g, '\\\\') // Because \ is used for escaping
               .replace( /"/g, '\\"' ) // Because " is used for surrounding escaped string
               .replace( /|/g, '\\|' ) // Because | is used for module separation
               .replace( /;/g, '\\;' ) // Because ; is used for property separation
               .replace( /:/g, '\\:' ) // Because : is used for object property separation
               .replace( /=/g, '\\=' ) // Because = is used for property id separation
               .replace( /{/g, '\\{' ) // Because { is used for object description
               .replace( /}/g, '\\}' ) // Because } is used for object description
          + '"'
        : value;
    }
    
    var result = [];
    
    for (var id in state) {
      var props = ['id:' + id];
      
      for (var prop in state[id])
        props.push(prop + ':' + escape(state[id][prop]));
      
      result.push(props.join(';'));
    }
    
    return result.join('|');
  }
  
  var urlHandler = {
    // TODO: escape values

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
      
      setTimeout(function () { that.hashSetProgress = false; }, 0);
    },

    /**
     * This function returns the value of the current state at the given key.
     */
    getStateValue : function (key) { return state[key]; },
    
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
     * This function returns the whole current state from the browser url as a
     * JavaScript object.
     * TODO: What's the difference between this and returning the global state
     * object?
     * TODO: This function sould be deleted and merged with getStateValue
     */
    getState : function () {
      var result = {};
      
      var h = hash();
      
      // This hack is needed because IE changes &cent to ¢ character.
      if (h.indexOf('¢') !== -1)
        hash(h = h.replace('¢', '&cent'));
      
      array.forEach(h.split('&'), function (element) {
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
    }
  };
  
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
  });
  
  return urlHandler;
});