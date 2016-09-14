define([
  'dojo/topic',
  'dojo/_base/declare',
  'dojo/store/Memory',
  'dijit/Tree',
  'dojo/store/Observable',
  'dijit/tree/ObjectStoreModel',
  'codecompass/registration/model',
  'codecompass/urlHandler'],
function (topic, declare, Memory, Tree, Observable, ObjectStoreModel, model, urlHandler) {
  return declare(Tree, {
    constructor : function () {      
      this._init();     
      
      this.set('model', this._dataModel);
      this.set('showRoot', false);    
    },
    
    _init : function () {
      this._data = [];
      
      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          return this.query({ parent : node.id });
        }
      }));
      
      this._store.put({
        id : 'root'
      });
      this._store.put({
        id     : 'parsingstatistics',
        name   : 'Parsing Statistics',
        parent : 'root'
      });
      this._store.put({
        id     : 'catalog',
        name   : 'C/C++ Catalog',
        parent : 'root'
      });
      this._store.put({
        id     : 'metrics',
        name   : 'Metrics',
        parent : 'root',
        loaded   : true
      });
      
      this._dataModel = new ObjectStoreModel({
        store : this._store,
        query : { id : 'root' },
        mayHaveChildren : function (node) { 
          return node.id === 'root' || node.id === 'metrics'; 
        }
      });
    },
    
    onClick : function (data, node, event) {
      switch (data.id) {
        case 'parsingstatistics':
          topic.publish('codecompass/statistics');
          break;
          
        case 'catalog':
          topic.publish('codecompass/catalog');
          break;
          
        case 'metrics':
          this.getMetricsSubInfo();
          break;
      }
      
      if(data.parent == 'metrics'){
        topic.publish('codecompass/metrics', {
          fileId : data.file
        });
      }
    },
    
    getMetricsSubInfo : function(){
      var that = this;
      
      var fileId = new model.FileId();
      fileId.fid = urlHandler.getState().fid;
      var fileInfo = model.project.getFileInfo(fileId);
            
      var paths = model.project.getPathTillFile(fileInfo.file);
      paths.pop();
      
      this._store.query().forEach(function (node) {
        if(node.parent == 'metrics')
          that._store.remove(node.id);
      });
      
      paths.forEach(function (fileInfo) {
        that._store.put({
          parent   : 'metrics',
          loaded : false,
          name   : fileInfo.path,
          file : fileInfo.file
        });
      });
    },
    
    getIconClass : function (item, opened) {      
      
      if(item.id == "parsingstatistics")
        return 'parsing_statistics';
      
      if(item.id == "catalog")
        return 'cpp_catalog';
      
      if(item.id == "metrics")
        return 'metrics';
      
      if(item.parent == 'metrics')
        return 'folder';
      
      return this.inherited(arguments);
    },
  });
});