require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dojo/on',
  'dojo/mouse',
  'dojo/dom-attr',
  'dojo/query!css2',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, on, mouse, attr, query, model,
  viewHandler) {

  model.addService('cppservice', 'CppService', LanguageServiceClient);

  function highlightEdges(diagramType, node, svgDom) {
    query('.node', svgDom).forEach(function (node) {
      var edgeAttrCache = {};
      var edgeIds = '.edge[id^="' + node.id + '_"]';
      on(node, mouse.enter, function () {
        dojo.query(edgeIds).forEach(function (edge) {
          query('path', edge).forEach(function (path) {
            edgeAttrCache[attr.get(edge, 'id')] = {
              'stroke'       : attr.has(path, 'stroke')
                             ? attr.get(path, 'stroke')
                             : 'black',
              'stroke-width' : attr.has(path, 'stroke-width')
                             ? attr.get(path, 'stroke-width')
                             : 1
            };
            attr.set(path, {'stroke' : 'red', 'stroke-width' : 2});
          });
        });
      });
      on(node, mouse.leave, function () {
        dojo.query(edgeIds).forEach(function (edge) {
          query('path', edge).forEach(function (path) {
            attr.set(path, edgeAttrCache[attr.get(edge, 'id')]);
          });
        });
      });
    });
  }

  var astDiagram = {
    id : 'cpp-ast-diagram',

    getDiagram : function (diagramType, nodeId, callback) {
      model.cppservice.getDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.cppservice.getDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      var nodeInfo = model.cppservice.getAstNodeInfo(nodeId);
      var range = nodeInfo.range.range;

      return {
        fileId : nodeInfo.range.file,
        selection : [
          range.startpos.line,
          range.startpos.column,
          range.endpos.line,
          range.endpos.column
        ]
      };
    },

    callback : function (diagramType, node, svgDom) {
      highlightEdges(diagramType, node, svgDom);
    }
  };

  viewHandler.registerModule(astDiagram, {
    type : viewHandler.moduleType.Diagram
  });

  var fileDiagramHandler = {
    id : 'cpp-file-diagram-handler',

    getDiagram : function (diagramType, nodeId, callback) {
      model.cppservice.getFileDiagram(nodeId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.cppservice.getFileDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      return {
        fileId : nodeId,
        selection : [1,1,1,1]
      };
    }
  };

  viewHandler.registerModule(fileDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });

  var fileDiagrams = {
    id : 'cpp-file-diagrams',
    render : function (fileInfo) {
      var submenu = new Menu();

      var diagramTypes = model.cppservice.getFileDiagramTypes(fileInfo.id);
      for (diagramType in diagramTypes)
        submenu.addChild(new MenuItem({
          label   : diagramType,
          type    : diagramType,
          onClick : function () {
            var that = this;

            topic.publish('codecompass/openFile', { fileId : fileInfo.id });

            topic.publish('codecompass/openDiagram', {
              handler : 'cpp-file-diagram-handler',
              diagramType : diagramTypes[that.type],
              node : fileInfo.id
            });
          }
        }));

      if (Object.keys(diagramTypes).length !== 0)
        return new PopupMenuItem({
          label : 'C++ Diagrams',
          popup : submenu
        });
    }
  };

  viewHandler.registerModule(fileDiagrams, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});
