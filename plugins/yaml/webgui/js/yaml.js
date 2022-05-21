require([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojo/dom-style',
  'dijit/MenuItem',
  'dijit/form/Button',
  'dijit/form/CheckBox',
  'dijit/form/Select',
  'dijit/layout/ContentPane',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/model'],
function (declare, dom, topic, style, MenuItem, Button, CheckBox, Select,
  ContentPane, viewHandler, urlHandler, model) {
  
  model.addService('yamlservice', 'YamlService', YamlServiceClient);

  var fileDiagramHandler = {
    id : 'yaml-file-diagram-handler',

    getDiagram : function (diagramType, fileId, callback) {
      model.yamlservice.getYamlFileDiagram(fileId, callback);
    },

    mouseOverInfo : function (diagramType, fileId) {
      return {
        fileId : fileId,
        selection : [1,1,1,1]
      };
    }
  };

  viewHandler.registerModule(fileDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });

  var yamlMenu = {
    id     : 'yamlMenu',
    render : function (fileInfo) {
      return new MenuItem({
        label    : 'Yaml',
        onClick  : function () {
          topic.publish('codecompass/openDiagram', {
            handler : 'yaml-file-diagram-handler',
            diagramType : 1,
            node : fileInfo.id
          })
        }
      });
    }
  };

  viewHandler.registerModule(yamlMenu, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });

  var fileInfoHandler = {
    id : 'yaml-file-info-handler',

    getDiagram : function (diagramType, fileId, callback) {
      model.yamlservice.getYamlFileInfo(fileId, callback);
    },

    mouseOverInfo : function (diagramType, fileId) {
      return {
        fileId : fileId,
        selection : [1,1,1,1]
      };
    }
  };

  viewHandler.registerModule(fileInfoHandler, {
    type : viewHandler.moduleType.Diagram
  });

  // var infobox = {
  //   id : 'Yaml-infobox',
  //   render : function (nodeInfo, fileInfo) {
  //     return new MenuItem({
  //       label : 'Documentation',
  //       onClick : function () {
  //         topic.publish('codecompass/documentation', {
  //           handler : 'yaml-file-diagram-handler',
  //           fileType    : fileInfo.type,
  //           elementInfo : nodeInfo
  //         });

  //         if (window.gtag) {
  //             window.gtag ('event', 'documentation', {
  //             'event_category' : urlHandler.getState('wsid'),
  //             'event_label' : urlHandler.getFileInfo().name
  //                           + ': '
  //                           + nodeInfo.astNodeValue
  //           });
  //         }
  //       }
  //     });
  //   }
  // };

  var infobox = {
    id : 'yamlMenu1',
    render : function (fileInfo) {
      return new MenuItem({
        label : 'YamlInfo',
        onClick : function () {
          topic.publish('codecompass/openDiagram', {
            handler : 'yaml-file-info-handler',
            diagramType : 1,
            node : fileInfo.id
          })

          // if (window.gtag) {
          //     window.gtag ('event', 'documentation', {
          //     'event_category' : urlHandler.getState('wsid')
          //   });
          // }
        }
      });
    }
  };

  viewHandler.registerModule(infobox, {
    type :  viewHandler.moduleType.FileManagerContextMenu
  });

});