require([
  'dojo/topic',
  'dijit/layout/BorderContainer',
  'dijit/layout/ContentPane',
  'codecompass/viewHandler',
  'codecompass/model'],
function (topic, model, viewHandler) {
  model.addService('competenceservice', 'CompetenceService', CompetenceServiceClient);

  var fileDiagramHandler = {
    id : 'competence-file-diagram-handler',

    getDiagram : function (diagramType, fileId, callback) {
      model.competenceservice.getDiagram(fileId, diagramType, callback);
    }
  };

  viewHandler.registerModule(fileDiagramHandler, {
    type : viewHandler.moduleType.Diagram
  });


});