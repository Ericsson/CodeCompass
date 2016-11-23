define([
  'dojo/_base/lang',
  'dojo/Deferred',
  'codecompass/model'],
function (lang, Deferred, model) {
  /**
   * This object contains the registered modules. The keys are the ID of the
   * given module and the value is an other object with 'module' and 'option'
   * keys. 'module' contains the module itself and 'option' is meta-information
   * of module (eg. type, service).
   */
  var modules = {};

  /**
   * This object contains objects of type Deferred. The keys are the ID of the
   * given module and the value is a deferred object which completes when the
   * module is registrated.
   */
  var asyncModules = {};

  return {

    moduleType : {
      Other                  : -1, /** Unknown type **/
      Header                 :  0, /** Header (eg. Search) **/
      Accordion              :  1, /** Left side accordion (eg. FileManager) **/
      Center                 :  2, /** Center module (eg. Text module) **/
      ContextButton          :  3, /** Context button menu item (eg. File outline) **/
      TextContextMenu        :  4, /** Text module context menu. **/
      Diagram                :  5, /** Diagram (eg. CppDiagram)**/
      InfoTree               :  6, /** Info tree item (eg. CppInforTree)**/
      FileManagerContextMenu :  7, /** File manager context menu (eg. Diagram directory)**/
      InfoPage               :  8, /** Info page (eg. Credit, User Guide) **/
    },

    /**
     * This function returns a module by ID.
     * @param {String} id ID of queried module.
     */
    getModule : function (id) {
      return modules[id] ? modules[id].module : undefined;
    },

    /**
     * This function returns an object of type Deferred. This object completes
     * when the module with the given ID is registrated.
     */
    getModuleAsync : function (id) {
      if (!asyncModules[id])
        asyncModules[id] = new Deferred();

      return asyncModules[id];
    },

    /**
     * This function returns an array of modules with given type. If type is not
     * given then all modules are returned.
     * @param {moduleType} type Module type, listed in "moduleType" object.
     * @returns {Array}
     */
    getModules : function (filter) {
      var result = [];      
      var sortableModules = [];

      filter = filter || {};

      //--- Filter modules ---//

      for (var module in modules)
        if (// Module type matches.
            (filter.type === undefined ||
             modules[module].options.type === filter.type) &&
            // Module is for the given file type.
            (filter.fileType === undefined ||
             modules[module].options.service === undefined ||
             modules[module].options.service ===
               model.getLanguageService(filter.fileType)))
          sortableModules.push(modules[module]);

      //--- Sort modules by priority ---//

      sortableModules.sort(function (a, b) {
        // If no priority given, then sort by ID.
        if (a.options.priority === undefined &&
            b.options.priority === undefined)
          return a.module.id < b.module.id ?  1 :
                 a.module.id > b.module.id ? -1 : 0;

        // Sort by priority if possible.
        if (a.options.priority !== undefined &&
            b.options.priority !== undefined)
          return a.options.priority - b.options.priority;

        // Those with priority provided are stronger.
        return a.options.priority === undefined ? 1 : -1;
      });

      for (var module in sortableModules)
        result.push(sortableModules[module].module);

      return result;
    },

    /**
     * This function registers a module so that it will be displayed on the
     * page.
     * @param {Object} module Module object.
     * @param {option} meta-information of module
     *  - type: tyoe of the module. It can't be undefined.
     *  - service: the current module belongs to this service 
     *    (eg. cpp menus belongs to cppservice). This is an optional parameter.
     *  - priority: Order of the module. Optional parameter. 
     * @throws The function throws an exception if the id of the registered
     * module is not a simple type (i.e. Object type, such as Object, Array or
     * Function), its id is not unique (has been already registered with this
     * id).
     */
    registerModule : function (module, options) {
      if (!options || options.type === undefined)
        throw "Options type can't be undefined: " + module;

      if (!module)
        return;

      if (lang.isObject(module.id))
        throw "Module id type can't be Object";

      if (modules[module.id])
        throw 'Module id has to be unique: ' + module.id;

      modules[module.id] = {
        module : module,
        options : options
      };

      this.getModuleAsync(module.id).resolve(modules[module.id].module);
    }
  };
});
