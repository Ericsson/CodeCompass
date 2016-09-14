define([
  'dojo/_base/lang',
  'dijit/_WidgetBase'
],
function (lang, _WidgetBase) {
  /**
   * This object contains the registered modules. The keys are the id of the given module and the
   * value is an other object with 'module' and 'type' keys. 'module' contains the module itself and
   * 'type' is the type of the module (Accordion or Center).
   */
  var modules = {};
  
  var viewHandler = {
    /**
     * We can make a distinction between the accordion and center modules by this enumeration type.
     */
    moduleType : {
      Accordion : 0,
      Center    : 1
    },
    
    /**
     * This function returns a module by id.
     * @param {String | Number} id Id of queried module.
     */
    getModule : function (id) {
      return modules[id] ? modules[id].module : undefined;
    },
    
    /**
     * This function returns an array of modules with given type. If type is not given then all
     * modules are returned.
     * @param {moduleType} type Module type, either Accordion or Center.
     * @returns {Array}
     */
    getModules : function (type) {
      var result = [];
      
      for (var module in modules)
        if (type === undefined || modules[module].type === type) {
          result.push(modules[module].module);
        }
      
      return result;
    },
    
    /**
     * This function registers a module so that it will be displayed on the page.
     * @param {Object} module Module object.
     * @param {moduleType} moduleType Module type (Accordion or Center).
     * @throws The function throws an exception if the id of the registered module is not a simple
     * type (i.e. Object type, such as Object, Array or Function), its id is not unique (has been
     * already registered with this id) or the value of 'content' attribute is not an instance of
     * dijit._WidgetBase.
     */
    registerModule : function (module, moduleType) {
      
      //--- Check module ---//
      
      if (!module)
        return;
      
      if (lang.isObject(module.id))
        throw "Module id type can't be Object";
      
      if (modules[module.id])
        throw 'Module id has to be unique: ' + module.id;
      
//      // Exception handling is necessary because basic types don't have 'isInstanceOf' method.
//      try {
//        var isBadContentType = !module.content.isInstanceOf(_WidgetBase);
//      } catch (error) {
//        isBadContentType = true;
//      }
//      
//      if (isBadContentType)
//        throw "Module has to have 'content' attribute which is instance of dijit/_WidgetBase";
      
      //--- Register module ---//
      
      modules[module.id] = {
        module : module,
        type   : moduleType
      };
    }
  };
  
  return viewHandler;
});