require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/Tooltip',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, Tooltip, model, viewHandler) {

  model.addService('competenceservice', 'CompetenceService', CompetenceServiceClient);

  var competenceHtml = `
<div id="dialog-competence" title="Add percentage">
<form>
<fieldset>
  <label for="ratio">Competence ratio</label>
  <input type="text" name="ratio" id="competence-ratio" value="50" class="text ui-widget-content ui-corner-all">
</fieldset>
</form>
</div>`;

  var competenceDiagramHandler = {
    id : 'competence-diagram-handler',

    getDiagram : function (diagramType, fileId, callback) {
      model.competenceservice.getDiagram(fileId, diagramType, callback);
    },

    getDiagramLegend : function (diagramType) {
      return model.competenceservice.getDiagramLegend(diagramType);
    },

    mouseOverInfo : function (diagramType, nodeId) {
      new Tooltip({
        connectId: nodeId,
        label: "value <b>74</b>",
        position: "above"
      });
    }
  };

  viewHandler.registerModule(competenceDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });

  $('body').append(competenceHtml);

  $("#competence-ratio").spinner({
     min:0, max:100
  });

  var selectedFileInfo = null;
  var competenceDialog = $("#dialog-competence").dialog({
      autoOpen: false,
      height: 200,
      width: 350,
      modal: true,
      buttons: {
          "OK": function () {
            // selectedFileInfo is accessible
              var comp = $("#competence-ratio").val();
              console.log(comp);
              model.competenceservice.setCompetenceRatio(selectedFileInfo.id,
                  comp,
                  function (resp) {
                      console.log(resp);
                      competenceDialog.dialog("close");
                  }
              );
          },
          "Cancel": function () {
              competenceDialog.dialog("close");
          }
      }
  });

  //--- Register Menu Points ---//

  var competenceMenu = {
    id     : 'cppCompetenceMenu',
    render : function (fileInfo) {
      var submenu = new Menu();

      submenu.addChild(new MenuItem({
        label : "Set comprehension rate",
        onClick : function () {
          selectedFileInfo = fileInfo;
          competenceDialog.dialog("open");
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Display diagram for current user",
        type : 0,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Team view",
        type : 1,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Team view for directory",
        type : 5,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Person-by-person company view",
        type : 2,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Accumulated company view",
        type : 3,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Risk view",
        type : 4,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Number of modifications by users",
        type : 6,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Developed by",
        type : 7,
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : this.type,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      return new PopupMenuItem({
        label : "Competence",
        popup : submenu
      });
    }
  };

  viewHandler.registerModule(competenceMenu, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});
