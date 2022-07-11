require([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util) {

  model.addService('yamlservice', 'YamlService', LanguageServiceClient);

  function createRootNode(elementInfo) {
    var rootLabel
      = '<span class="root label">'
      + (elementInfo instanceof AstNodeInfo
        ? elementInfo.symbolType
        : 'File')
      + '</span>';

    var rootValue
      = '<span class="root value">'
      + (elementInfo instanceof AstNodeInfo
        ? elementInfo.astNodeValue
        : elementInfo.name)
      + '</span>';

    var label
      = '<span class="root label">'
      + rootLabel + ': ' + rootValue
      + '</span>';

    return {
      id          : 'root',
      name        : label,
      cssClass    : 'icon-info',
      hasChildren : true,
      getChildren : function () {
        return that._store.query({ parent : 'root' });
      }
    };
  }

  var yamlInfoTree = {
    id : 'yaml-infotree',
    render: function (elementInfo) {
      var ret = [];
      console.log(elementInfo);

      ret.push(createRootNode(elementInfo));

      if (elementInfo instanceof AstNodeInfo) {
        var props = model.yamlservice.getProperties(elementInfo.id);

        for (var propName in props) {
          var propId = propName.replace(/ /g, '-');
          var label
            = '<span class="label">' + propName + '</span>: '
            + '<span class="value">' + props[propName] + '</span>';

          ret.push({
            name        : label,
            parent      : 'root',
            nodeInfo    : elementInfo,
            cssClass    : 'icon-' + propId,
            hasChildren : false
          });
        }
      }

      return ret;
    }
  };

  viewHandler.registerModule(yamlInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.yamlservice
  });
});
