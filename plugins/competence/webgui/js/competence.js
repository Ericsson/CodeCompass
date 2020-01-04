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

  model.addService('competenceservice', 'CompetenceService', CompetenceServiceClient);

  /*viewHandler.registerModule(new Competence({ id : 'competence' }), {
    type : viewHandler.moduleType.Center
  });*/

  var competenceHtml = `
<div id="dialog-competence" title="Create new user">
<form>
<fieldset>
  <label for="ratio">Competence ratio</label>
  <input type="text" name="ratio" id="competence-ratio" value="50" class="text ui-widget-content ui-corner-all">
</fieldset>
</form>
</div>`;


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
      return new MenuItem({
        label    : 'Competence',
        onClick  : function () {
          /*topic.publish('codecompass/competence', {
            fileInfo : fileInfo
          })*/
          console.log(fileInfo);
          selectedFileInfo = fileInfo;
          competenceDialog.dialog("open");
        }
      });
    }
  };

  viewHandler.registerModule(competenceMenu, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });
});
