require([
    'dojo/on',
    'dijit/Tooltip',
    'dijit/tree/ObjectStoreModel',
    'dojo/_base/declare',
    'dojo/store/Memory',
    'dojo/store/Observable',
    'dijit/Tree',
    'dojo/topic',
    'codecompass/view/component/HtmlTree',
    'codecompass/model',
    'codecompass/viewHandler',
    'codecompass/util',
    'codecompass/view/component/ContextMenu'],
  function (on, Tooltip, ObjectStoreModel, declare, Memory, Observable, Tree, topic,
            HtmlTree, model, viewHandler, util, ContextMenu) {

  model.addService('yamlservice', 'YamlService', YamlServiceClient);

  var YamlNavigator = declare(Tree, {
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

      this._dataModel = new ObjectStoreModel({
        store: that._store,
        query: { id: 'root' },
        mayHaveChildren: function (node) {
          return node.hasChildren;
        }
      });

      this._contextMenu = new ContextMenu();

      this._data.push({
        id: 'root',
        name: 'List of microservices',
        cssClass: 'icon-list',
        hasChildren: true,
        getChildren: function () {
          return that._store.query({parent: 'root'});
        }
      });

      model.yamlservice.getMicroserviceTypes().forEach(function (type) {

        that._store.put({
          id          : type,
          name        : that.serviceTypeToString(type),
          cssClass    : 'icon-repository',
          hasChildren : true,
          loaded      : true,
          parent      : 'root',
          getChildren : function (type) {
            return that.getMicroservices(type);
          }
        });
      });

      this.set('model', this._dataModel);
      this.set('openOnClick', false);
    },

    getMicroservices : function (serviceType) {
      var that = this;

      var ret = [];

      model.yamlservice.getMicroserviceList(serviceType.id).forEach(function (service) {
        ret.push({
          id: service.serviceId,
          name: service.name,
          cssClass: 'icon-head',
          hasChildren: false
        });
      });

      return ret;
    },

    startup : function () {
      this.inherited(arguments);
      var that = this;

      var contextMenu = new ContextMenu({
        targetNodeIds : [this.id],
        selector      : '.dijitTreeNode'
      });

      on(this, '.dijitTreeNode:contextmenu', function (event) {
        that.buildContextMenu(contextMenu);
      });
    },

    buildContextMenu : function (contextMenu) {
      contextMenu.clear();

      viewHandler.getModules({
        type : viewHandler.moduleType.MicroserviceContextMenu,
        fileType : fileInfo.type
      }).forEach(function (menuItem) {
        var item = menuItem.render(fileInfo);
        if (item)
          contextMenu.addChild(item);
      });
    },

    serviceTypeToString : function (serviceType) {
      switch (serviceType) {
        case ServiceType.Internal: return 'Internal';
        case ServiceType.External: return 'External';
      }
    }

  });

  var navigator = new YamlNavigator({
    id    : 'yamlnavigator',
    title : 'Microservice Navigator'
  });

  viewHandler.registerModule(navigator, {
    type : viewHandler.moduleType.Accordion
  });
});