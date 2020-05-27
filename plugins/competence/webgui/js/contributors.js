require([
  'dojo/topic',
  'dojox/layout/TableContainer',
  'codecompass/model',
  'codecompass/viewHandler'],
function (topic, TableContainer, model, viewHandler) {

  model.addService('competenceservice', 'CompetenceService', CompetenceServiceClient);

  var emailPairs = model.competenceservice.getUserEmailPairs();
  emailPairs.forEach()
  var emailTable = `
    <div data-dojo-type="dojox.layout.TableContainer" data-dojo-props="cols:2, customClass:'justLabels'" id="tc1">
    <div data-dojo-type="dijit.form.TextBox" title="First Name:" value="Eamonn"></div>
    <div data-dojo-type="dijit.form.TextBox" title="Last Name:" value="De Valera"></div>
    </div> `

  /*try {
    if (model.authentication.isRequiringAuthentication()) {

    }
  } catch (ex) {
    console.warn(ex);
  }*/

});