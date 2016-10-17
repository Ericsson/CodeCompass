define([
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'dijit/Tooltip',
  'dijit/layout/BorderContainer',
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dojo/on',
  'dojo/mouse',
  'dojo/query',
  'codecompass/model',
  'codecompass/util',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Pager', 
  'codecompass/viewHandler'],
function (Tree, ObjectStoreModel, Tooltip, BorderContainer, array, declare,
  Memory, Observable, topic, on, mouse, query, model, util,
  ContextMenu, Pager, viewHandler) {

  var HtmlTreeNode = declare(Tree._TreeNode, {
    _setLabelAttr : { node : "labelNode", type : "innerHTML" }
  });

  var IconTree = declare(Tree, {
    getIconClass : function (item, opened) {
      if (item.fileName && item.name !== 'More ...')
        var iconClass = util.getIconClass(item.fileName);

      if(item.fileRange){
        return 'icon icon-node';
      }
      if(iconClass)
        return 'icon ' + iconClass;

      return this.inherited(arguments);
    },

    _onNodeMouseEnter : function (node, evt) {
      Tooltip.show(node.item.name, node.labelNode, ['above']);
    },

    _onNodeMouseLeave : function (node, evt) {
      Tooltip.hide(node.labelNode);
    },

    _createTreeNode : function (args) {
      return new HtmlTreeNode(args);
    }
  });

  /**
   * This function prepares the tree so that a leaf node can be inserted. The
   * query result tree always has three levels: the directory path, the file
   * name and the queried result itself in the file. This function creates the
   * first two level according to its parameter.
   * @param {QueryResult} queryResult Used QueryResult module.
   * @param {Object} fileInfo A FileInfo object from which we can read the file
   * name and path.
   * @param {Boolean} fileLevel If this parameter is true then the second level
   * (file level) will
   * be the last one, i.e. there is no more level under it.
   * @return {Object} The function returns the node of the second level.
   */
  function createDirsByFInfo(queryResult, fileInfo, fileLevel) {
    var path = fileInfo.path;
    var directory = util.getDirectory(path);
    if (!queryResult._store.get(directory))
      queryResult._store.add({
        id     : directory,
        name   : directory,
        parent : 'root'
      });

    var parseStatus = model.project.getFileInfo(fileInfo.id).parseStatus;
    var name = fileInfo.name;

    if (parseStatus === FileParseStatus.FullyParsed)
      name = '<span style="color: green">' + name + '</span>';
    else if (parseStatus === FileParseStatus.PartiallyParsed)
      name = '<span style="color: orange">' + name + '</span>';

    if (!fileLevel)
      name = '<b>' + name + '</b>';

    var node = {
      id       : path,
      name     : name,
      parent   : directory,
      fileName : fileInfo.name
    };

    if (fileLevel) {
      var fileRange = new FileRange();
      var position  = new Position();
      var range     = new Range();

      position.line   = 1;
      position.column = 1;

      range.startpos = position;
      range.endpos   = position;

      fileRange.file  = fileInfo.id;
      fileRange.range = range;

      node.fileRange = fileRange;
    }

    if (queryResult._store.query({ id : path, parent : directory }).total === 0)
      queryResult._store.add(node);

    return node;
  }

  /**
   * This function loads search results in query results.
   * @param {QueryResult} queryResult Used QueryResult module.
   * @param {Object} data This object contains the necessary information for
   * loading search. This object has to have these parameters:
   * - text: Search on this text.
   * - fileFilter: File name filter. (optional)
   * - dirFilter: Directory (path) filter. (optional)
   * - searchType: A value from SearchOptions enum type.
   */
  function loadSearch(queryResult, data) {

    //--- Query search results ---//

    var pageNumber = queryResult._pager.get('pageNumber');
    var pageSize   = queryResult._pager.get('pageSize');

    var params = new SearchParams();
    var query  = new SearchQuery();
    var range  = new SearchRange();
    var filter = new SearchFilter();

    query.query = data.text;

    range.start   = (pageNumber - 1) * pageSize;
    range.maxSize = pageSize;

    filter.fileFilter = data.fileFilter || '';
    filter.dirFilter  = data.dirFilter  || '';

    params.options = data.searchType;
    params.query   = query;
    params.range   = range;
    params.filter  = filter;

    //--- Build new tree ---//

    queryResult._moreMap = {};

    var exception;

    try {
      var searchResult
        = data.searchType === SearchOptions.SearchForFileName
        ? model.searchservice.searchFile(params)
        : model.searchservice.search(params);
    } catch (ex) {
      exception = ex;
    }

    topic.publish('codecompass/searchError', { exception : exception });

    queryResult._pager.set(
      'total', searchResult ? searchResult.totalFiles : 0);

    if (!searchResult || searchResult.totalFiles === 0) {
      queryResult._store.add({
        parent : 'root',
        name   : 'No result...'
      });
      return;
    }

    // TODO: Do we need this if?
    if (data.searchType === SearchOptions.SearchForFileName)
      array.forEach(searchResult.results, function (basicFileInfo) {
        createDirsByFInfo(queryResult, basicFileInfo, true);
      });
    else
      array.forEach(searchResult.results, function (searchResultEntry) {
        var fileNode = createDirsByFInfo(queryResult, searchResultEntry.finfo);

        array.forEach(searchResultEntry.matchingLines.splice(0, 5),
        function (lineMatch) {
          queryResult._store.add({
            name      : util.escapeTags(lineMatch.text),
            parent    : searchResultEntry.finfo.path,
            fileRange : lineMatch.range,
            fileName  : searchResultEntry.finfo.name
          });
        });

        if (searchResultEntry.matchingLines.length !== 0)
          queryResult._store.add({
            name     : 'More ...',
            parent   : searchResultEntry.finfo.path,
            fileName : searchResultEntry.finfo.name
          });
        
        queryResult._moreMap[fileNode.id] = searchResultEntry.matchingLines;
      });
  }

  /**
   * This function loads the results in the given QueryResult instance.
   * @param {QueryResult} queryResult QueryResult instance.
   * @param {Function} loader Loader function which is used to query data to
   * fill the tree with. This function is given the QueryResult instance and the
   * _currentQueryData object.
   */
  function loadResults(queryResult, loader) {

    //--- Remove previous tree ---//

    if (queryResult._tree)
      queryResult.removeChild(queryResult._tree);

    //--- Store and model ---//

    queryResult._data = [];

    queryResult._store = new Observable(new Memory({
      data        : queryResult._data,
      getChildren : function (node) {
        return queryResult._store.query({ parent : node.id });
      }
    }));

    queryResult._dataModel = new ObjectStoreModel({
      store           : queryResult._store,
      query           : { id : 'root' },
      mayHaveChildren : function (node) { return !node.fileRange; }
    });

    //--- Call loader ---//

    loader(queryResult, queryResult._currentQueryData);

    //--- Tree ---//

    queryResult._data.push({ id : 'root' });

    queryResult._tree = new IconTree({
      region      : 'center',
      model       : queryResult._dataModel,
      showRoot    : false,
      openOnClick : true,
      onClick     : function (item, node, event) {
        if (item.name === 'More ...') {
          queryResult._store.remove(item.id);
          
          array.forEach(queryResult._moreMap[item.parent].splice(0, 5),
          function (lineMatch) {
            queryResult._store.add({
              name      : lineMatch.text,
              parent    : item.parent,
              fileRange : lineMatch.range,
              fileName  : item.fileName
            });
          });
          
          if (queryResult._moreMap[item.parent].length !== 0)
            queryResult._store.add(item);
          
        } else if(item.fileRange){ // Not More button
          var range = item.fileRange.range;
          topic.publish('codecompass/openFile', {
            fileId     : item.fileRange.file,
            line       : item.fileRange.range.startpos.line,
            moduleId   : 'text',
            newTab     : mouse.isMiddle(event),
            newSession : true,
            selection  : [
              range.startpos.line,
              range.startpos.column,
              range.endpos.line,
              range.endpos.column]
          });
        }
      }
    });

    queryResult.addChild(queryResult._tree);
    queryResult._tree.expandAll();
  }

  var QueryResult = declare(BorderContainer, {
    constructor : function () {
      var that = this;

      //--- Initialisation ---//

      topic.subscribe('codecompass/search', function (message) {
        if (that._pagerChanged)
          that._pagerChanged = false;
        else
          that._pager.set('pageNumber', 1);

        that._searchOrReferences = 'search';
        that._currentQueryData = {
          text       : message.text,
          fileFilter : message.fileFilter,
          dirFilter  : message.dirFilter,
          searchType : message.searchType
        };

        loadResults(that, loadSearch);
      });

      //--- Pager ---//

      this._pager = new Pager({
        region   : 'top',
        onChange : function (pageNumber, pageSize) {
          if (!that._currentQueryData)
            return;

          if (that._currentQueryData) {
            that._pagerChanged = true;
            topic.publish(
              'codecompass/' + that._searchOrReferences,
              that._currentQueryData);
          }
        }
      });
    },

    postCreate : function () {
      this.addChild(this._pager);
    }
  });

  topic.subscribe('codecompass/search', function (message) {
    topic.publish('codecompass/selectAccordion', 'queryresult');
  });

  var queryResult = new QueryResult({
    id      : 'queryresult',
    title   : 'Query Results',
    gutters : false
  });
  viewHandler.registerModule(queryResult, {
    type : viewHandler.moduleType.Accordion,
    priority : 20
  });

  return QueryResult;
});