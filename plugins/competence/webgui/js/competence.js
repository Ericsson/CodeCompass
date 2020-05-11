require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, model, viewHandler) {

  model.addService('competenceservice', 'CompetenceService', CompetenceServiceClient);

  var competenceHtml = `
<div id="dialog-competence" title="Create new user">
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
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : 0,  // In case there are other diagrams.
            node : fileInfo.id
          });
        }
      }));

      submenu.addChild(new MenuItem({
        label : "Team view",
        onClick : function () {
          topic.publish('codecompass/openFile', { fileId : fileInfo.id });

          topic.publish('codecompass/openDiagram', {
            handler : 'competence-diagram-handler',
            diagramType : 1,  // In case there are other diagrams.
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
