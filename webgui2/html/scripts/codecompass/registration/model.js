define([
  'dojo/topic',
  'codecompass/urlHandler'],
function (topic, urlHandler) {
  var model = {};
  
  function addService(serviceUrl, modelName, Client) {
    if (serviceUrl === 'PluginService' ||
        serviceUrl === 'WorkspaceService' ||
        plugins.indexOf(serviceUrl) !== -1)
      model[modelName] = new Client(new Thrift.Protocol(new Thrift.Transport(serviceUrl)));
  }
  
  function addType(typeName, Type) {
    model[typeName] = Type;
  }
  
  addService('PluginService',    'pluginservice',    PluginServiceClient   );
  addService('WorkspaceService', 'workspaceservice', WorkspaceServiceClient);
  
  var plugins   = model.pluginservice.getPlugins();
  var workspace = urlHandler.getState().wsid || model.workspaceservice.getWorkspaces()[0].id;

  addService(workspace + '/ProjectService',     'project',            ProjectServiceClient    );
  addService(workspace + '/CppService',         'cppservice',         LanguageServiceClient   );
  addService(workspace + '/JavaService',        'javaservice',        LanguageServiceClient   );
  addService(workspace + '/SearchService',      'searchservice',      SearchServiceClient     );
  addService(workspace + '/VersionService',     'versionservice',     VersionServiceClient    );
  addService(workspace + '/PythonService',      'pythonservice',      LanguageServiceClient   );
  addService(workspace + '/MetricsService',     'metricsservice',     MetricsServiceClient    );
  addService(workspace + '/CodeCheckerService', 'codecheckerservice', codeCheckerDBAccess.codeCheckerDBAccessClient);
    
  addType('AstNodeId',              AstNodeId              );
  addType('AstNodeInfo',            AstNodeInfo            );
  addType('FileId',                 FileId                 );
  addType('FileType',               FileType               );
  addType('FilePosition',           FilePosition           );
  addType('FileParseStatus',        FileParseStatus        );
  addType('FileRange',              FileRange              );
  addType('DiagramId',              DiagramId              );
  addType('ProjectId',              ProjectId              );
  addType('RefTypes',               RefTypes               );
  addType('Category',               Category               );
  addType('Position',               Position               );
  addType('Range',                  Range                  );
  addType('MenuType',               MenuType               );
  addType('SearchParams',           SearchParams           );
  addType('SearchSuggestionParams', SearchSuggestionParams );
  addType('SearchSuggestions',      SearchSuggestions      );
  addType('SearchQuery',            SearchQuery            );
  addType('SearchRange',            SearchRange            );
  addType('SearchFilter',           SearchFilter           );
  addType('SearchOptions',          SearchOptions          );
  addType('MessageType',            MessageType            );
  addType('VersionBlob',            VersionBlob            );
  addType('VersionTreeEntry',       VersionTreeEntry       );
  addType('VersionCommit',          VersionCommit          );
  addType('VersionDiffOptions',     VersionDiffOptions     );
  addType('SlicingTypes',           SlicingTypes           );
  addType('ChunkInfo',              ChunkInfo              );
  addType('Cluster',                Cluster                );
  addType('LocInfo',                LocInfo                );
  addType('Severity',               Severity               );
  addType('MetricsType',            MetricsType            );
  
  /**
   * This function returns the appropriate language service api belonging to the given file type.
   * @param {Number} fileType FileType thrift enum.
   */
  model.getLanguageService = function (fileType) {
    switch (fileType) {
      case model.FileType.CSource:
      case model.FileType.CxxSource:
      case model.FileType.Directory:   // TODO: This should not be here!!!
      case model.FileType.GenericFile: // TODO: This should not be here!!! (object and executable)
        return model.cppservice;
        break;

      case model.FileType.JavaSource:
        return model.javaservice;
        break;

      case model.FileType.PythonScript:
        return model.pythonservice;
        break;

      default:
        return null;
    }
  };
  
  return model;
});
