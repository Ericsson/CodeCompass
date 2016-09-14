define([],
function () {
  var labels = {
    /**
     * This action opens the given file.
     */
    'codecompass/openFile' : {
      fileId : 'Thrift FileId object, the ID of the file to open.',
      moduleId : 'ID of the registrated (moduleRegistration.js) module which\
        has to display the file.',
      newTab : 'A boolean value which indicates whether the file has to be\
        opened in a new tab.',
      info : 'A string message which is displayed in Browsing History. If not\
        given, the history tree won\'t contain this opened file.',
      newSession : 'A boolean flag. If false, then the tree node in Browsing\
        History will be placed under the current one, otherwise it is placed to\
        the first level.',
      line : 'A number of line to jump to when a new file is opened.',
      jumpFrom : 'FileRange thrift object from where we jump when "jump to\
        def". Browsing History uses this.',
      jumpTo : 'FileRange thrift object to where we jump when "jump to def".\
        Browsing History uses this.',
      selection : 'A range to select in the opened file in this format:\
        {from : {line : 1, column : 2}, to : {line : 3, column : 4}}'
    },
    
    /**
     * This action opens a diagram.
     */
    'codecompass/openDiagram' : {
      diagramNode : 'The diagram node id (string, number).',
      diagramType : 'DiagramId thrift object',
      fromNodeId  : 'Starting node in function call path diagram.'
    },
    
    /**
     * This action loads CodeBites view based on an AstNodeInfo.
     */
    'codecompass/codebites' : {
      astNodeInfo : 'AstNodeInfo thrift object.'
    },
    
    /**
     * This action loads the Doxygen view and displays the html files.
     */
    'codecompass/doxygen' : {
      path : 'Path to the doxygen doxumentation, relative to doxygen dir.'
    },
    
    /**
     * This action loads the MI diagram editor for the given diagram node.
     */
    'codecompass/openMiDiagramEditor' : {
      diagramNode : 'The diagram node id (string, number).',
      miScenario  : 'Serialized form of diagram editor.'
    },
    
    /**
     * This action changes the path under search fields.
     */
    'codecompass/fileClick' : {
      path : 'Full file path which is displayed under the search field.',
      display : 'If given then this string will be displayed in the input\
        field.'
    },
    
    /**
     * This action opens the info tree about the given file or AST node.
     */
    'codecompass/infotree' : {
      elementInfo : 'AstNodeInfo or FileInfo thrift object.',
      fileType : 'FileType thrift object'
    },
    
    /**
     * This action does a search and shows the results in Query Results.
     */
    'codecompass/search' : {
      text : 'Text to search.',
      fileFilter : 'A regexp as string to filter file names.',
      dirFilter : 'A regexp as string to filter paths.',
      searchType : 'SearchType thrift object.'
    },
    
    'codecompass/searchError' : {
      exception : 'A Thrift exception object returned from the server. The\
        error message is displayed based on this exception.'
    },
    
    /**
     * This action loads the references of a node and shows the results in
     * Query Results.
     */
    'codecompass/references' : {
      astNodeId : 'AstNodeId thrift object.',
      fileType : 'FileType thrift object',
      refType : 'ReferenceType thrift object'
    },
    
    /**
     * This action loads the main page of CodeCompass at the given tab.
     */
    'codecompass/infopage' : {
      page : 'Page name.'
    },
    
    /**
     * This action change the file manager
     */
    'codecompass/changefilemanager' : {
      fileManager: 'File Manager type'
    },
    
    /**
     * This action loads the statistics center module.
     */
    'codecompass/statistics' : {
      
    },
    
    /**
     * This action loads the catalog center module.
     */
    'codecompass/catalog' : {
      
    },

    /**
     * This action loads the Metrics center module.
     */
    'codecompass/metrics' : {
      fileId : 'The id of the file or directory of which we want to display\
        metrics for.',
      fileTypes : 'Array of model.FileType values. If not given then it behaves\
        as all values were listed. Only the files of these types will be\
        included in the metric.',
      metricsTypes : 'Array of model.MetricsType values. The array has to be of\
        length 2 which specify the size and color dimensions respectively.'
    }
  };
  
  return {
    getUndocumentedAttrs : function (label, message) {
      var attrs  = labels[label];
      var result = [];
      
      for (var attr in message)
        if (!attrs || !attrs[attr])
          result.push(attr);
      
      return result;
    }
  };
});
