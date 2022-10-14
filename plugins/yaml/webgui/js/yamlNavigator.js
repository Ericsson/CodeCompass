require([
    'dijit/Tooltip',
    'dijit/tree/ObjectStoreModel',
    'dojo/_base/declare',
    'dojo/store/Memory',
    'dojo/store/Observable',
    'dojo/topic',
    'codecompass/view/component/HtmlTree',
    'codecompass/model',
    'codecompass/viewHandler',
    'codecompass/util'],
  function (Tooltip, ObjectStoreModel, declare, Memory, Observable, topic,
            HtmlTree, model, viewHandler, util) {

  model.addService('yamlservice', 'YamlService', YamlServiceClient);

  var YamlNavigator = declare(HtmlTree, {
    _numOfMicroserviesToLoad : 20,

    constructor : function () {
      var that = this;

      this._data = [];

      this._store = new Observable(new Memory({
        data: this._data,
        getChildren: function (node) {
          return node.getChildren ? node.getChildren(node) : [];
        }
      }));

      var dataModel = new ObjectStoreModel({
        store: that._store,
        query: {id: 'root'},
        mayHaveChildren: function (node) {
          return node.hasChildren;
        }
      });

      this._data.push({
        id: 'root',
        name: 'List of microservices',
        cssClass: 'icon-list',
        hasChildren: true,
        getChildren: function () {
          //return that._store.query({parent: 'root'});
          return that.getMicroservices();
        }
      });

      this.set('model', dataModel);
      this.set('openOnClick', false);

    },

    getMicroservices : function () {
      var that = this;

      var ret = [];

      model.yamlservice.getMicroserviceList().forEach(function (service) {
        ret.push({
          id          : service.serviceId,
          name        : service.name,
          cssClass    : 'icon-head',
          hasChildren : false
        });
      });

      return ret;
    },

  });

  var navigator = new YamlNavigator({
    id    : 'yamlnavigator',
    title : 'Microservice Navigator'
  });

  viewHandler.registerModule(navigator, {
    type : viewHandler.moduleType.Accordion
  });
});