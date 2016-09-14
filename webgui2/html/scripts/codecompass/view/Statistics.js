define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/_base/array',
  'dijit/layout/ContentPane',
  'dojox/grid/DataGrid',
  'dojo/data/ItemFileWriteStore',
  'codecompass/registration/model'],
function (declare, dom, array, ContentPane, DataGrid, ItemFileWriteStore,
    model) {
  
  return declare(ContentPane, {
    
    constructor : function() {
      var that = this;
      
      this._tableMap = {};
      this._storeMap = {};
    },
    
    postCreate : function () {
      var that = this;
      
      array.forEach(model.project.getStatistics(), function (statInfo) {
        that.addStatistic(statInfo);
      });
    },
    
    addStatistic : function(statInfo) {
      if (this._tableMap[statInfo.group] === undefined) {
        this.addTable(statInfo.group);
      }
      
      this.addItem(statInfo);
    },
    
    addTable : function(heading) {
      var layout = [
                    {name: 'Statistic', field: 'id', width: 15},
                    {name: 'Value', field: 'value', width: 10, styles: 'text-align: right;'}
                ];
      
      var store = new ItemFileWriteStore({
        data: {
            identifier: "id",
            items: []
        }
      });
      
      store.comparatorMap = {};
      store.comparatorMap["value"] = function (a, b) {
        var numA = parseInt(a);
        var numB = parseInt(b);
        
        if (numA < numB)
          return -1;
          
        if (numA > numB)
          return 1;
          
        return 0;
      };
      
      this._storeMap[heading] = store;
      
      var datagrid = new DataGrid({
        id: 'grid_' + heading,
        store: this._storeMap[heading],
        structure: layout,
        autoWidth: true,
        autoHeight: true
      });
      
      this._tableMap[heading] = datagrid;
      
      dojo.place("<h1>" + heading + "</h1>", this.containerNode);
      this.addChild(datagrid);
    },
    
    addItem : function(statistic) {
      this._storeMap[statistic.group].newItem(
            {id: statistic.key, value: statistic.value}
          );
    }
    
  }
  
  );
});
