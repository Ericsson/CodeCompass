namespace cpp cc.service.plugin

service PluginService
{
  /**
   * Returns a list of active plugins which are loaded from shared object
   */
  list<string> getPlugins()

  /**
   * Returns a list of javascript plugins from install web directory
   * (eg. generated thrift files, modules)
   */
  list<string> getWebPlugins()
  
  /**
   * Returns a list of css plugins from install web directory
   */
  list<string> getWebStylePlugins()
}
