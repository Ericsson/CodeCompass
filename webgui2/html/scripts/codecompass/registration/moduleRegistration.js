define([
  'dojo/topic',
  'codecompass/viewHandler',
  'codecompass/view/FileManager',
  'codecompass/view/QueryResult',
  'codecompass/view/InfoTree',
  'codecompass/view/Text',
  'codecompass/view/Diagram',
  'codecompass/view/CodeBites',
  'codecompass/view/InfoPage',
  'codecompass/view/BrowsingHistory',
  'codecompass/view/CodeCheckerNavigator',
  'codecompass/view/VersionNavigator',
  'codecompass/view/VersionText',
  'codecompass/view/VersionCommit',
  'codecompass/view/VersionHistory',
  'codecompass/view/VersionHistoryDiff',
  'codecompass/view/Metrics',
  'codecompass/view/Doxygen',
  'codecompass/view/ProjectInformation',
  'codecompass/view/Statistics',
  'codecompass/view/Catalog',
  'codecompass/registration/model'],
function (topic, viewHandler, FileManager, QueryResult, InfoTree,Text, 
  Diagram, CodeBites, InfoPage, BrowsingHistory, CodeCheckerNavigator,
  VersionNavigator, VersionText, VersionCommit, VersionHistory,
  VersionHistoryDiff, Metrics, Doxygen, ProjectInformation, Statistics,
  Catalog, model) {
  
  function registration () {
    
    //--- Create modules ---//

    var filemanager = new FileManager({
      id    : 'filemanager',
      title : 'File Manager'
    });

    var queryResult = new QueryResult({
      id      : 'queryresult',
      title   : 'Query Results',
      gutters : false
    });
    
    var infoTree = new InfoTree({
      id    : 'infotree',
      title : 'Info Tree'
    });
    
    var browsingHistory = new BrowsingHistory({
      id       : 'browsinghistory',
      title    : 'Browsing History',
      showRoot : false
    });
    
    if (model.codecheckerservice)
      var codecheckernavigator = new CodeCheckerNavigator({
        id    : 'codecheckernavigator',
        title : 'CodeChecker'
      });
    
    var text = new Text({
      id            : 'text',
      displaySearch : true
    });
    
    var diagram = new Diagram({
      id         : 'diagram',
      textModule : text,
      gutters    : false
    });
    
    var codebites = new CodeBites({
      id : 'codebites'
    });
    
    var infoPage = new InfoPage({
      id : 'infopage'
    });
    
    if (model.versionservice)
      var versionnavigator = new VersionNavigator({
        id      : 'versionnavigator',
        title   : 'Revision Control Navigator',
        gutters : false
      });
    
    var versiontext = new VersionText({
      id : 'versiontext'
    });

    var versioncommit = new VersionCommit({
      id : 'versioncommit'
    });

    var versionhistory = new VersionHistory({
      id : 'versionhistory'
    });

    var versionhistorydiff = new VersionHistoryDiff({
      id : 'versionhistorydiff'
    });
    
    var doxygen = new Doxygen({
      id : 'doxygen'
    });
    
    var projectinformation = new ProjectInformation({
      id    : 'projectinformation',
      title : 'Project Information'
    });
    
    var statistics = new Statistics({
      id    : 'statistics',
      title : 'Statistics',
      style : 'padding: 10px'
    });
    
    var catalog = new Catalog({
      id    : 'catalog',
      title : 'Catalog',
      style : 'padding: 10px'
    });

    if (model.metricsservice)
      var metrics = new Metrics({
        id : 'metrics'
      });

    //--- View module registration ---//

    viewHandler.registerModule(text,                 viewHandler.moduleType.Center);
    viewHandler.registerModule(diagram,              viewHandler.moduleType.Center);
    viewHandler.registerModule(codebites,            viewHandler.moduleType.Center);
    viewHandler.registerModule(infoPage,             viewHandler.moduleType.Center);
    viewHandler.registerModule(versiontext,          viewHandler.moduleType.Center);
    viewHandler.registerModule(versioncommit,        viewHandler.moduleType.Center);
    viewHandler.registerModule(versionhistory,       viewHandler.moduleType.Center);
    viewHandler.registerModule(versionhistorydiff,   viewHandler.moduleType.Center);
    viewHandler.registerModule(doxygen,              viewHandler.moduleType.Center);
    viewHandler.registerModule(statistics,           viewHandler.moduleType.Center);
    viewHandler.registerModule(catalog,              viewHandler.moduleType.Center);
    viewHandler.registerModule(metrics,              viewHandler.moduleType.Center);
    viewHandler.registerModule(filemanager,          viewHandler.moduleType.Accordion);
    viewHandler.registerModule(queryResult,          viewHandler.moduleType.Accordion);
    viewHandler.registerModule(infoTree,             viewHandler.moduleType.Accordion);
    viewHandler.registerModule(browsingHistory,      viewHandler.moduleType.Accordion);
    viewHandler.registerModule(codecheckernavigator, viewHandler.moduleType.Accordion);
    viewHandler.registerModule(versionnavigator,     viewHandler.moduleType.Accordion);
    viewHandler.registerModule(projectinformation,   viewHandler.moduleType.Accordion);
  }
  
  return { registration : registration };
});
