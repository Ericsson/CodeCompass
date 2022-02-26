require([
  'dijit/tree/ObjectStoreModel',
  'dijit/layout/BorderContainer',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dojo/mouse',
  'codecompass/model',
  'codecompass/urlHandler',
  'codecompass/util',
  'codecompass/view/component/HtmlTree',
  'codecompass/view/component/Pager',
  'codecompass/view/component/TooltipTreeMixin',
  'codecompass/viewHandler'],
function (ObjectStoreModel, BorderContainer, declare, Memory, Observable, topic,
  mouse, model, urlHandler, util, HtmlTree, Pager, TooltipTreeMixin, viewHandler) {

  var moreSize = 5;
  var moreText = 'More ...';

  var IconTree = declare([HtmlTree, TooltipTreeMixin], {
    getIconClass : function (item, opened) {
      if (item.fileName && item.name !== moreText)
        var iconClass = util.getIconClass(item.fileName);

      if (item.fileRange)
        return 'icon icon-node';
      if (iconClass)
        return 'icon ' + iconClass;

      return this.inherited(arguments);
    }
  });

  var SearchResult = declare(BorderContainer, {
    constructor: function () {
      var that = this;

      //--- Initialisation ---//

      topic.subscribe('codecompass/search', function (message) {
        if (that._pagerChanged)
          that._pagerChanged = false;
        else
          that._pager.set('pageNumber', 1);

        that._currentQueryData = {
          text: message.text,
          fileFilter: message.fileFilter,
          dirFilter: message.dirFilter,
          searchType: message.searchType
        };

        that._searchTypes = model.searchservice.getSearchTypes();

        that._loadResults();

        topic.publish('codecompass/selectAccordion', that.id);
      });
    },

    postCreate: function () {
      var that = this;

      this._pager = new Pager({
        region: 'top',
        onChange: function (pageNumber, pageSize) {
          if (!that._currentQueryData)
            return;

          if (that._currentQueryData) {
            that._pagerChanged = true;
            topic.publish(
              'codecompass/search',
              that._currentQueryData);
          }
        }
      });

      this.addChild(this._pager);
    },

    /**
     * This function loads the results in the given QueryResult instance.
     */
    _loadResults: function () {
      var that = this;

      //--- Remove previous tree ---//

      if (this._tree)
        this.removeChild(this._tree);

      //--- Store and model ---//

      this._data = [];

      this._store = new Observable(new Memory({
        data: that._data,
        getChildren: function (node) {
          return that._store.query({parent: node.id});
        }
      }));

      this._dataModel = new ObjectStoreModel({
        store: that._store,
        query: {id: 'root'},
        mayHaveChildren: function (node) {
          return !node.fileRange;
        }
      });

      //--- Call loader ---//

      this._loadSearch(this._currentQueryData);

      //--- Tree ---//

      this._data.push({id: 'root'});

      this._tree = new IconTree({
        region: 'center',
        model: that._dataModel,
        showRoot: false,
        openOnClick: true,
        onClick: function (item, node, event) {
          if (item.name === moreText) {
            that._store.remove(item.id);

            that._moreMap[item.parent].splice(0, moreSize).forEach(
              function (lineMatch) {
                that._store.add({
                  name: lineMatch.text,
                  parent: item.parent,
                  fileRange: lineMatch.range,
                  fileName: item.fileName
                });
              });

            if (that._moreMap[item.parent].length !== 0)
              that._store.add(item);

          } else if (item.fileRange) { // Not More button
            var range = item.fileRange.range;
            topic.publish('codecompass/openFile', {
              fileId: item.fileRange.file,
              line: item.fileRange.range.startpos.line,
              moduleId: 'text',
              newTab: mouse.isMiddle(event),
              newSession: true,
              selection: [
                range.startpos.line,
                range.startpos.column,
                range.endpos.line,
                range.endpos.column]
            });
          }
        }
      });

      this.addChild(this._tree);
      this._tree.expandAll();
    },

    /**
     * This function prepares the tree so that a leaf node can be inserted. The
     * query result tree always has three levels: the directory path, the file
     * name and the queried result itself in the file. This function creates the
     * first two level according to its parameter.
     * @param {Object} fileInfo A FileInfo object from which we can read the
     * file name and path.
     * @param {Boolean} fileLevel If this parameter is true then the second
     * level (file level) will be the last one, i.e. there is no more level
     * under it.
     * @return {Object} The function returns the node of the second level.
     */
    _createDirsByFInfo: function (fileInfo, fileLevel) {
      var directory = util.getDirectory(fileInfo.path);
      if (!this._store.get(directory))
        this._store.add({
          id: directory,
          name: directory,
          parent: 'root'
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
        id: fileInfo.path,
        name: name,
        parent: directory,
        fileName: fileInfo.name
      };

      if (fileLevel) {
        var fileRange = new FileRange();
        var position = new Position();
        var range = new Range();

        position.line = 1;
        position.column = 1;

        range.startpos = position;
        range.endpos = position;

        fileRange.file = fileInfo.id;
        fileRange.range = range;

        node.fileRange = fileRange;
      }

      if (this._store.query({id: fileInfo.path, parent: directory}).total === 0)
        this._store.add(node);

      return node;
    },



    /**
     * This function queries loads search results in search results.
     * @param {Object} data This object contains the necessary information for
     * loading search. This object has to have these parameters:
     * - text: Search on this text.
     * - fileFilter: File name filter. (optional)
     * - dirFilter: Directory (path) filter. (optional)
     * - searchType: A value from SearchOptions enum type.
     */
    _loadSearch: function (data) {
      var that = this;

      //--- Query search results ---//

      var pageNumber = this._pager.get('pageNumber');
      var pageSize = this._pager.get('pageSize');

      var params = new SearchParams();
      var range = new SearchRange();
      var filter = new SearchFilter();

      range.start = (pageNumber - 1) * pageSize;
      range.maxSize = pageSize;

      filter.fileFilter = data.fileFilter || '';
      filter.dirFilter = data.dirFilter || '';

      params.options = data.searchType;
      params.query = data.text;
      params.range = range;
      params.filter = filter;

      if (window.gtag) {
        var type = this._searchTypes.find(t => t.id === data.searchType);
        window.gtag('event', 'search: ' + type.name, {
          'event_category': urlHandler.getState('wsid'),
          'event_label': params.query
        });
      }

      //--- Build new tree ---//

      this._moreMap = {};

      try {
        var searchResult
          = data.searchType === SearchOptions.SearchForFileName
          ? model.searchservice.searchFile(params)
          : model.searchservice.search(params);
      } catch (ex) {
        topic.publish('codecompass/searchError', {exception: ex});
      }

      this._pager.set(
        'total', searchResult ? searchResult.totalFiles : 0);

      if (!searchResult || searchResult.totalFiles === 0) {
        this._store.add({
          parent: 'root',
          name: 'No result ...'
        });
        return;
      }

      if (data.searchType === SearchOptions.SearchForFileName)
        searchResult.results.forEach(function (fileInfo) {
          that._createDirsByFInfo(fileInfo, true);
        });
      else
        searchResult.results.forEach(function (searchResultEntry) {
          var fileNode = that._createDirsByFInfo(searchResultEntry.finfo);

          searchResultEntry.matchingLines.splice(0, moreSize).forEach(
            function (lineMatch) {
              that._store.add({
                name: util.escapeTags(lineMatch.text),
                parent: searchResultEntry.finfo.path,
                fileRange: lineMatch.range,
                fileName: searchResultEntry.finfo.name
              });
            });

          if (searchResultEntry.matchingLines.length !== 0)
            that._store.add({
              name: moreText,
              parent: searchResultEntry.finfo.path,
              fileName: searchResultEntry.finfo.name
            });

          that._moreMap[fileNode.id] = searchResultEntry.matchingLines;
        });
    }
  });

  var searchResult = new SearchResult({
    id      : 'searchresult',
    title   : 'Search Results',
    gutters : false
  });

  viewHandler.registerModule(searchResult, {
    type : viewHandler.moduleType.Accordion,
    priority : 20
  });
});
